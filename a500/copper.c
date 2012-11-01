#include <dos/dos.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <inline/exec.h>
#include <inline/graphics.h>
#include <inline/dos.h> 

#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#define offsetof(st, m) \
  ((ULONG)((char *)&((st *)0)->m - (char *)0))

extern struct ExecBase *SysBase;

inline int swap16(int a) {
  asm("swap %0": "=d" (a));
  return a;
}

int __nocommandline = 1;
int __initlibraries = 0;

struct ExceptionVector {
  APTR InitialSSP;             /*  0 */
  APTR InitialPC;              /*  1 */
  APTR BusError;               /*  2 */
  APTR AddressError;           /*  3 */
  APTR IllegalInstruction;     /*  4 */
  APTR DivideByZero;           /*  5 */
  APTR CheckInstruction;       /*  6 */
  APTR TrapInstruction;        /*  7 */
  APTR PrivilegeViolation;     /*  8 */
  APTR Trace;                  /*  9 */
  APTR UnimplementedOpcodeA;   /* 10 */
  APTR UnimplementedOpcodeF;   /* 11 */
  APTR reserved1[3];           /* 12 - 14 */
  APTR UninitializedInterrupt; /* 15 */
  APTR reserved2[8];           /* 16 - 23 */
  APTR SpuriousInterrupt;      /* 24 */
  APTR IntLevel1;              /* 25 */
  APTR IntLevel2;              /* 26 */
  APTR IntLevel3;              /* 27 */
  APTR IntLevel4;              /* 28 */
  APTR IntLevel5;              /* 29 */
  APTR IntLevel6;              /* 30 */
  APTR IntLevel7;              /* 31 */
  APTR Trap[16];               /* 32 - 47 */
  APTR reserved3[16];          /* 48 - 63 */
  APTR UserDefined[192];       /* 64 - 255 */
};

struct ExceptionVector *ExceptionVector = (APTR)0L;
struct DosLibrary *DOSBase = NULL;
struct GfxBase *GfxBase = NULL;

volatile struct Custom *custom = (APTR)0xdff000;
volatile struct CIA *ciaa = (APTR)0xbfe001;
volatile struct CIA *ciab = (APTR)0xbfd000;

struct ExceptionVector *GetVBR();

asm(".even\n"
    "_GetVBR:\n"
    "  movec vbr,d0\n"
    "  rte");

void WaitMouse() {
  while (ciaa->ciapra & CIAF_GAMEPORT0);
}

void WaitBlitter() {
  while (custom->dmaconr & DMAF_BLTDONE);
}

void WaitVBlank() {
  for (;;) {
    ULONG vpos = (*(volatile ULONG *)&custom->vposr) >> 8;

    if ((vpos & 0x1ff) == 312)
      break;
  }
}

typedef struct CopList {
  UWORD length;
  UWORD index;
  UWORD flags;
  UWORD pad;
  ULONG entry[0]; 
} CopListT;

__regargs CopListT *NewCopList(UWORD length) {
  CopListT *copList = AllocMem(sizeof(CopListT) + length * sizeof(ULONG),
                               MEMF_CHIP|MEMF_CLEAR);
  copList->length = length - 1;
  copList->index = 0;
  copList->flags = 0;

  return copList;
}

__regargs void DeleteCopList(CopListT *copList) {
  FreeMem(copList, sizeof(CopListT) + (copList->length + 1) * sizeof(ULONG));
}

__regargs void CopListActivate(CopListT *copList) {
  WaitVBlank();
  /* Write copper list address. */
  custom->cop1lc = (ULONG)copList->entry;
  /* Activate it immediately */
  custom->copjmp1 = 0;
  /* Enable copper DMA */
  custom->dmacon = DMAF_MASTER | DMAF_COPPER | DMAF_SETCLR;
}

__regargs void CopInit(CopListT *copList) {
  copList->index = 0;
  copList->flags = 0;
}

__regargs void CopWait(CopListT *copList, UWORD vp, UWORD hp) {
  if (vp <= 255) {
    if (copList->index < copList->length) {
      UWORD *word = (UWORD *)&copList->entry[copList->index++];

      word[0] = (vp << 8) | (hp & 0xff) | 1;
      word[1] = 0xfffe;
    }
  } else {
    if (copList->index - 1 < copList->length) {
      if (!copList->flags) {
        copList->entry[copList->index++] = 0xffdffffe;
        copList->flags |= 1;
      }

      {
        UWORD *word = (UWORD *)&copList->entry[copList->index++];

        word[0] = ((vp - 255) << 8) | (hp & 0xff) | 1;
        word[1] = 0xfffe;
      }
    }
  }
}

__regargs void CopMove16(CopListT *copList, UWORD reg, UWORD data) {
  if (copList->index < copList->length) {
    UWORD *word = (UWORD *)&copList->entry[copList->index++];
    
    word[0] = reg & 0x01fe;
    word[1] = data;
  }
}

__regargs void CopMove32(CopListT *copList, UWORD reg, ULONG data) {
  if (copList->index - 1 < copList->length) {
    UWORD *word = (UWORD *)&copList->entry[copList->index];

    word[0] = reg & 0x01fe;
    word[1] = data >> 16;
    word[2] = (reg + 2) & 0x01fe;
    word[3] = data;

    copList->index += 2;
  }
}

__regargs void CopEnd(CopListT *copList) {
  if (copList->index <= copList->length)
    copList->entry[copList->index++] = 0xfffffffe;
}

CopListT *cpMaybe = NULL;
ULONG frameNumber = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    custom->intreq = INTF_VERTB;

    frameNumber++;

    if (cpMaybe) {
      ((UWORD *)cpMaybe->entry)[1] = ((frameNumber & 63) < 32) ? 0x00f : 0x0f0;
    }
  }
}

#define INTF_LEVEL3 (INTF_VERTB | INTF_BLIT | INTF_COPER)

void Main() {
  APTR OldIntLevel3;

  OldIntLevel3 = ExceptionVector->IntLevel3;
  ExceptionVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_LEVEL3 | INTF_INTEN;

  {
    CopListT *cp = NewCopList(100);

    cpMaybe = cp;

    CopInit(cp);
    CopMove16(cp, offsetof(struct Custom, color[0]), 0xfff);
    CopWait(cp, 312/2, 0);
    CopMove16(cp, offsetof(struct Custom, color[0]), 0xf00);
    CopEnd(cp);
    CopListActivate(cp);

    WaitMouse();

    cpMaybe = NULL;

    DeleteCopList(cp);
  }

  custom->intena = INTF_LEVEL3;
  ExceptionVector->IntLevel3 = OldIntLevel3;
}

int main() {
  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 34))) {
    if ((GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 34))) {
      /* Get Vector Base Register */
      if (SysBase->AttnFlags & AFF_68010)
        ExceptionVector = (APTR)Supervisor((APTR)GetVBR);

      Forbid();

      {
        struct View *OldView = GfxBase->ActiView;
        UWORD OldDMAcon, OldIntena;

        LoadView(NULL);
        WaitBlit();
        OwnBlitter();

        OldDMAcon = custom->dmaconr;
        OldIntena = custom->intenar;

        /* prohibit dma & interrupts */
        custom->dmacon = 0x7fff;
        custom->intena = 0x7fff;

        Main();

        /* restore AmigaOS state of dma & interrupts */
        custom->dmacon = OldDMAcon | DMAF_SETCLR;
        custom->intena = OldIntena | INTF_SETCLR;

        /* restore old copper list */
        custom->cop1lc = (ULONG)GfxBase->copinit;

        DisownBlitter();
        LoadView(OldView);
      }

      Permit();

    }
    CloseLibrary((struct Library *)DOSBase);
  }
  return 0;
}
