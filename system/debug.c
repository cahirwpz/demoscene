#include <config.h>
#include <common.h>
#include <custom.h>
#include <debug.h>
#include <system/cia.h>
#include <system/interrupt.h>
#include <system/boot.h>

#if !defined(UAE) && DEBUG >= 1
#include <stdarg.h>
#include <stdio.h>

#define CRASHLOG_SIZE 4096 /* must be power of two */

typedef struct CrashLog {
  short head, tail;
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

void CrashPutChar(void *ptr, char data) {
  struct CrashLog *cl = ptr;

  cl->buffer[cl->tail++] = data;
  cl->tail &= CRASHLOG_SIZE - 1;
  if (cl->tail == cl->head)
    cl->head++;
  cl->head &= CRASHLOG_SIZE - 1;
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
static __code char PageStr[] = "[Page ?]";

static void DumpStatus(u_char *screen) {
  static __code short page = 0;
  page++;
  PageStr[6] = page + '0';
  PutStr(MoveTo(screen, 0, 0), InfoStr);
  PutStr(MoveTo(screen, 72, 0), PageStr);
}

static void DumpCrash(u_char *screen, CrashLogT *cl) {
  u_char *pos;
  short y;

  y = 1;
  DumpStatus(screen);
  pos = MoveTo(screen, 0, y);

  while (cl->head != cl->tail) {
    char c = cl->buffer[cl->head];

    cl->head++;
    cl->head &= CRASHLOG_SIZE - 1;

    if (c == '\n') {
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
      continue;
    }

    PutChar(pos++, c);
  }
}

static void ResetHardware(CustomPtrT custom_) {
  custom_->intena_ = INTF_ALL;
  custom_->dmacon = DMAF_ALL;

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

    bset(ciaprb, CIAB_DSKSEL0);
    bset(ciaprb, CIAB_DSKMOTOR);
    bclr(ciaprb, CIAB_DSKSEL0);
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

  DumpCrash(screen, &CrashLog);
  HALT();
  for (;;)
    continue;
}

void Log(const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf(CrashPutChar, (void *)&CrashLog, format, args);
  va_end(args);
}

__noreturn void Panic(const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf(CrashPutChar, (void *)&CrashLog, format, args);
  va_end(args);

  Crash();
}

const char hex[] = "0123456789abcdef";

static inline void PrintHex(char i) {
  CrashPutChar(&CrashLog, hex[i & 15]);
}

static inline void PrintChar(char c) {
  CrashPutChar(&CrashLog, c);
}

void HexDump(const void *_ptr, u_int len) {
  const u_char *ptr = _ptr;
  u_int addr = 0;

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
}
#else
void CrashInit(BootDataT *bd) {
  (void)bd;
}

__noreturn void Crash(void) {
  HALT();
  for(;;);
}
#endif
