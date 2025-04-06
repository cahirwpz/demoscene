#include <config.h>
#include <common.h>
#include <custom.h>
#include <debug.h>
#include <system/boot.h>
#include <system/cia.h>
#include <system/interrupt.h>
#include <system/mutex.h>
#include <system/task.h>

#if !defined(UAE) && DEBUG >= 1
#include <stdarg.h>
#include <stdio.h>

#define CRASHLOG_SIZE 4096 /* must be power of two */

typedef struct CrashLog {
  short tail, head;
  char buffer[CRASHLOG_SIZE];
} CrashLogT;

/* All diagnostic messages are stored in circular buffer. */
static CrashLogT CrashLog;
static __code u_char *ChipMem;
static __code u_char *Font;

void CrashInit(BootDataT *bd) {
  Font = bd->bd_topaz - (intptr_t)' ';
  ChipMem = (u_char *)0x400;
}

static void CrashPutChar(void *ptr, char data) {
  struct CrashLog *cl = ptr;
  short tail = cl->tail;

  cl->buffer[tail++] = data;
  tail &= CRASHLOG_SIZE - 1;
  cl->tail = tail;
  if (tail == cl->head) {
    cl->head++;
    cl->head &= CRASHLOG_SIZE - 1;
  }
}

/*
 * This code will be jumped into by Assert/Panic/TrapHandler functions,
 * so the user can see diagnostic messages collected in CrashLog buffer.
 */

#define MOVE(reg, val) { offsetof(struct Custom, reg), (val) }
#define WAIT(y) { (((y) + 0x2c) << 8) | 1, 0xff00 }
#define END() { 0xffff, 0xfffe }

static __code u_short CopList[][2] = {
  MOVE(bplpt, 0),
  MOVE(bplpt+2, 0),
  MOVE(bpl1mod, 0),
  MOVE(bpl2mod, 0),
  MOVE(diwstrt, 0x2c81),
  MOVE(diwstop, 0x2cc1),
  MOVE(ddfstrt, 0x003c),
  MOVE(ddfstop, 0x00d4),
  MOVE(bplcon0, 0x9200),
  MOVE(bplcon1, 0x0000),
  MOVE(bplcon2, 0x0000),
  MOVE(fmode, 0),
  WAIT(0),
  MOVE(color[0], 0xfff),
  MOVE(color[1], 0),
  WAIT(8),
  MOVE(color[0], 0),
  MOVE(color[1], 0xfff),
  END()
};

#define WIDTH 640
#define HEIGHT 256
#define FONTW 1536
#define FONTH 8
#define BPLSIZE (WIDTH * HEIGHT / 8)

static void ClearScreen(u_char *screen, u_int val) {
  u_int *dst = (u_int *)screen;
  short n = BPLSIZE / sizeof(u_int);
  while (--n >= 0) {
    *dst++ = val;
  }
}

static inline u_char *MoveTo(u_char *screen, short x, short y) {
  return screen + x + y * WIDTH;
}

static void PutChar(u_char *pos, short c) {
  u_char *font = Font + c;
  short n = FONTH;

  while (--n >= 0) {
    *pos = *font;
    font += FONTW / 8;
    pos += WIDTH / 8;
  }
}

static void PutStr(u_char *pos, const char *str) {
  char c;

  while ((c = *str++)) {
    PutChar(pos++, c);
  }
}

static const char InfoStr[] =
  "It crashed! Send screenshots to cahirwpz at Discord. Press LMB!";
static __code char PageStr[] = "[Page ?/?]";
static __code short npages = 1;

static void DumpStatus(u_char *screen) {
  static __code short page = 0;
  page++;
  PageStr[6] = page + '0';
  PageStr[8] = npages + '0';
  PutStr(MoveTo(screen, 0, 0), InfoStr);
  PutStr(MoveTo(screen, 70, 0), PageStr);
}

static short CountPages(CrashLogT *cl) {
  short i = cl->head;
  short pages = 1;
  short y = 1;
  short x = 0;

  while (i != cl->tail) {
    char c = cl->buffer[i++];

    i &= CRASHLOG_SIZE - 1;

    if ((c == '\n') || (x == 80)) {
      x = 0;
      y++;
      if (y >= HEIGHT / FONTH) {
        y = 1;
        pages++;
      }
      if (c == '\n')
        continue;
    }

    x++;
  }

  return pages;
}

static void DumpCrash(u_char *screen, CrashLogT *cl) {
  short i = cl->head;
  u_char *pos;
  short x = 0, y = 1;

  DumpStatus(screen);
  pos = MoveTo(screen, 0, y);

  while (i != cl->tail) {
    char c = cl->buffer[i++];

    i &= CRASHLOG_SIZE - 1;

    if ((c == '\n') || (x == 80)) {
      x = 0;
      pos = MoveTo(screen, 0, ++y);
      if (y >= HEIGHT / FONTH) {
        while (!LeftMouseButton())
          continue;
        while (LeftMouseButton())
          continue;
        y = 1;
        ClearScreen(screen, 0);
        DumpStatus(screen);
        pos = MoveTo(screen, 0, y);
      }
      if (c == '\n')
        continue;
    }

    PutChar(pos++, c);
    x++;
  }
}

static void DumpCustom(CustomPtrT custom_) {
  u_int vpos = custom_->vposr_ & 0x1ffff;
  u_short intena = custom_->intenar;
  u_short intreq = custom_->intreqr;
  u_short dmacon = custom_->dmaconr;
  u_short adkcon = custom_->adkconr;

  custom_->intena_ = INTF_ALL;
  custom_->dmacon = DMAF_ALL;

  Log("[Custom] INTENA $%04x INTREQ $%04x DMACON $%04x ADKCON $%04x VPOS $%05x\n",
      intena, intreq, dmacon, adkcon, vpos);
}

static void DumpCIA(CIAPtrT cia_, const char *id) {
  u_char icr = SampleICR(cia_, CIAICRF_ALL);
  u_int tod = 0;
  u_short ta = 0;
  u_short tb = 0;

  Log("[%s] ICR $%02x CRA $%02x CRB $%02x PRA $%02x PRB $%02x DDRA $%02x DDRB $%02x\n",
      id, icr, cia_->ciacra, cia_->ciacrb, cia_->ciapra, cia_->ciaprb, cia_->ciaddra, cia_->ciaddrb);

  ta |= cia_->ciatahi;
  ta <<= 8;
  ta |= cia_->ciatalo;

  tb |= cia_->ciatbhi;
  tb <<= 8;
  tb |= cia_->ciatblo;

  tod |= cia_->ciatodhi;
  tod <<= 8;
  tod |= cia_->ciatodmid;
  tod <<= 8;
  tod |= cia_->ciatodlow;

  Log("[%s] TA $%04x TB $%04x TOD $%06x\n",
      id, ta, tb, tod);
}

static void ResetHardware(CustomPtrT custom_) {
  CrashPutChar(&CrashLog, '\n');

  DumpCustom(custom_);
  DumpCIA(ciaa, "CIA-A");
  DumpCIA(ciab, "CIA-B");

#ifdef MULTITASK
  TaskDebug();
#endif

  /* Reset sprites. */
  {
    volatile struct SpriteDef *spr = custom_->spr;
    short n = 8;
    while (--n >= 0) {
      *(u_int *)&spr->dataa = 0;
      spr++;
    }
  }

  /* Stop floppy motor. */
  {
    u_char *ciaprb = (u_char *)&ciab->ciaprb;
    u_char dsksel = CIAB_DSKSEL0 + BootDev;

    bset(ciaprb, dsksel);
    bset(ciaprb, CIAB_DSKMOTOR);
    bclr(ciaprb, dsksel);
  }
}

__noreturn void Crash(void) {
  u_char *screen = ChipMem;
  u_short *cp = (u_short *)(ChipMem + BPLSIZE);

  ResetHardware(custom);
  ClearScreen(screen,0);

  /* Set up bitplane pointer. */
  CopList[0][1] = (uintptr_t)ChipMem >> 16;
  CopList[1][1] = (uintptr_t)ChipMem;

  /* Copy copper list to CHIP memory. */
  {
    u_int *src = (u_int *)CopList;
    u_int *dst = (u_int *)cp;
    short n = sizeof(CopList) / sizeof(u_int);
    while (--n >= 0) {
      *dst++ = *src++;
    }
  }

  /* Activate copper list. */
  custom->cop1lc = (u_int)cp;
  EnableDMA(DMAF_MASTER | DMAF_COPPER | DMAF_RASTER);

  npages = CountPages(&CrashLog);
  DumpCrash(screen, &CrashLog);
  HALT();
  for (;;)
    continue;
}

static MUTEX(DebugMtx);

void Log(const char *format, ...) {
  va_list args;

  MutexLock(&DebugMtx);

  va_start(args, format);
  kvprintf(CrashPutChar, (void *)&CrashLog, format, args);
  va_end(args);

  MutexUnlock(&DebugMtx);
}

static const char hex[] = "0123456789abcdef";

static inline void PrintHex(char i) {
  CrashPutChar(&CrashLog, hex[i & 15]);
}

static inline void PrintChar(char c) {
  CrashPutChar(&CrashLog, c);
}

void HexDump(const void *_ptr, u_int len) {
  const u_char *ptr = _ptr;
  u_int addr = 0;

  MutexLock(&DebugMtx);

  for (addr = 0; addr < len; addr++) {
    /* print address */
    if ((addr & 15) == 0) {
      u_int data = addr;
      short i;

      for (i = 0; i < 8; i++) {
        data = roll(data, 4);
        PrintHex(data);
      }

      PrintChar(':');
      PrintChar(' ');
    }

    {
      u_char data = *ptr++;
      PrintHex(data >> 4);
      PrintHex(data);
    }
 
    if ((addr & 3) == 3) {
      PrintChar((addr & 15) == 15 ? '\n' : ' ');
    }
  }

  if (addr & 15) {
    PrintChar('\n');
  }

  MutexUnlock(&DebugMtx);
}
#else
void CrashInit(BootDataT *bd) {
  (void)bd;
}

__noreturn void Crash(void) {
  /* TODO fix emulator to flush stdout before HALT */
  UaeLog("\n");
  HALT();
  for(;;);
}
#endif
