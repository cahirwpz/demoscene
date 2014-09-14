#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "random.h"

#define LINES 8
#define NEXT ((LINES * 2 + 3) * sizeof(CopInsT))

static CopListT *cp = NULL;
static UBYTE *linePos[LINES];
static UWORD *lineColor[LINES];
static UWORD colors[LINES][LINES];

static void Load() {
  CopInsT *ins;
  WORD i, j;

  cp = NewCopList(256 * (LINES * 2 + 3) + 100);

  CopInit(cp);
  CopSetRGB(cp, 0, 0);

  for (i = 0; i < 256; i++) {
    CopWait(cp, Y(i), 8);
    CopSetRGB(cp, 0, 0);
    if (Y(i) != 256)
      CopSetRGB(cp, 0, 0);
    for (j = 0; j < LINES; j++) {
      ins = CopWait(cp, Y(i), 8);
      if (i == 0)
        linePos[j] = ((UBYTE*)ins) + 1;
      ins = CopSetRGB(cp, 0, 0);
      if (i == 0)
        lineColor[j] = ((UWORD*)ins) + 1;
    }
  }

  CopWait(cp, Y(256), 8);
  CopSetRGB(cp, 0, 0);
  CopEnd(cp);

  for (i = 0; i < LINES; i++) 
    for (j = 0; j < LINES; j++) 
      colors[i][j] = random() & 0xfff;

  Log("Copper list entries: %ld.\n", (LONG)(cp->curr - cp->entry));
}

static void Kill() {
  DeleteCopList(cp);
}

static void CopperLine(UBYTE *lineptr, WORD x1, WORD y1, WORD x2, WORD y2) {
  x1 = X(x1) >> 1;
  x2 = X(x2) >> 1;

  lineptr += y1;

  {
    WORD dy = y2 - y1;
    WORD dx = x2 - x1;
    LONG z = x1 << 16;
    /* This is a long division! Precalculate if possible... */
    LONG dz = (dx << 16) / dy;
    register UBYTE one asm("d7") = 1;

    while (--dy >= 0) {
      UWORD x = swap16(z);
      *lineptr = x | one;
      lineptr += NEXT;
      z += dz;
    }
  }
}

static void SetLinesColor() {
  WORD i, j, k;

  for (i = 0; i < LINES; i++) {
    UWORD *lineptr = lineColor[i];
    UWORD *colptr = colors[i];
    
    j = 8;
    while (--j >= 0) {
      UWORD c = *colptr++;
      k = 32 / 4;
      while (--k >= 0) {
        *lineptr = c;
        lineptr += NEXT / sizeof(UWORD);
        *lineptr = c;
        lineptr += NEXT / sizeof(UWORD);
        *lineptr = c;
        lineptr += NEXT / sizeof(UWORD);
        *lineptr = c;
        lineptr += NEXT / sizeof(UWORD);
      }
    }
  }
}

static void Init() {
  CopListActivate(cp);
}

static void Loop() {
  while (!LeftMouseButton()) {
    {
      LONG lc = ReadLineCounter();
      ITER(i, 0, LINES - 1, CopperLine(linePos[i], 0 + i * 32, 0, 24 + i * 32, 256));
      Log("lines: %ld\n", ReadLineCounter() - lc);
    }

    {
      LONG lc = ReadLineCounter();
      SetLinesColor();
      Log("colors: %ld\n", ReadLineCounter() - lc);
    }
  }
}

EffectT Effect = { Load, Init, Kill, Loop };
