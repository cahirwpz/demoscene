#include "effect.h"
#include "copper.h"
#include "fx.h"
#include "color.h"
#include <stdlib.h>

#define WIDTH   320
#define HEIGHT  256
#define SIZE    128
#define STRIPES 20
#define BGCOL   0x204

typedef struct {
  short y, z;
  short color;
} StripeT;

static CopListT *cp[2];
static CopInsT *lineColor[2][HEIGHT];
static StripeT stripe[STRIPES];
static short active = 0;

static u_short colorSet[4] = { 0xC0F, 0xF0C, 0x80F, 0xF08 };
static u_short colorShades[4 * 32];

static void GenerateStripes(void) {
  short *s = (short *)stripe;
  short n = STRIPES;

  while (--n >= 0) {
    *s++ = (random() & (SIZE - 1)) - SIZE / 2;
    *s++ = (random() & (SIZE - 1)) - SIZE / 2;
    *s++ = random() & 0x60;
  }
}

static void GenerateColorShades(void) {
  short i, j;
  u_short *s = colorSet;
  u_short *d = colorShades;

  for (i = 0; i < 4; i++) {
    u_short c = *s++;

    for (j = 0; j < 16; j++)
      *d++ = ColorTransition(0x000, c, j);
    for (j = 0; j < 16; j++)
      *d++ = ColorTransition(c, 0xfff, j);
  }
}

static CopListT *MakeCopperList(CopInsT **line) {
  CopListT *cp = NewCopList(HEIGHT * 2 + 100);
  short i;

  CopSetColor(cp, 0, BGCOL);

  for (i = 0; i < HEIGHT; i++) {
    CopWaitSafe(cp, Y(i), 8);
    *line++ = CopSetColor(cp, 0, 0);
  }

  CopWait(cp, Y(256), 8);
  CopSetColor(cp, 0, BGCOL);
  return CopListFinish(cp);
}

static void Init(void) {
  GenerateStripes();
  GenerateColorShades();

  cp[0] = MakeCopperList(lineColor[0]);
  cp[1] = MakeCopperList(lineColor[1]);

  CopListActivate(cp[0]);
}

static void Kill(void) {
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static short centerY = 0;
static short centerZ = 192;

static void RotateStripes(short *d, short *s, short rotate) {
  short n = STRIPES;
  int cy = centerY << 8;
  short cz = centerZ;

  while (--n >= 0) {
    short sinA = SIN(rotate);
    short cosA = COS(rotate);
    short y = *s++;
    short z = *s++;
    int yp = (y * cosA - z * sinA) >> 4; 
    short zp = normfx(y * sinA + z * cosA);

    *d++ = div16(yp + cy, zp + cz);
    *d++ = zp;
    *d++ = *s++;
  }
}

static void ClearLineColor(void) {
  CopInsT **line = lineColor[active];
  short n = HEIGHT;

  while (--n >= 0)
    CopInsSet16(*line++, BGCOL);
}

static void SetLineColor(short *s) {
  CopInsT **lines = lineColor[active];
  short n = STRIPES;
  u_short *shades = colorShades;

  while (--n >= 0) {
    short y = *s++;
    short z = *s++;
    u_short color = *s++;

    short h = (short)(z + 128) >> 5;
    short l = (z >> 2) + 16;
    short i = y + ((short)(HEIGHT - h) >> 1);

    if (l < 0)
      l = 0;
    if (l > 31)
      l = 31;

    if ((i >= 0) && (i + h < HEIGHT)) {
      CopInsT **line = &lines[i];
      short c0 = shades[color | l];
      short c1 = shades[color | (l >> 1)];

      h -= 2;

      CopInsSet16(*line++, c1);

      while (--h >= 0)
        CopInsSet16(*line++, c0);

      CopInsSet16(*line++, c1);
    }
  }
}

static void SortStripes(StripeT *table) {
  StripeT *ptr = table + 1;
  register short n asm("d7") = STRIPES - 2;

  do {
    StripeT *curr = ptr;
    StripeT *prev = ptr - 1;
    StripeT this = *ptr++;
    while (prev >= table && prev->z > this.z)
      *curr-- = *prev--;
    *curr = this;
  } while (--n != -1);
}

static void RenderStripes(short rotate) {
  static StripeT temp[STRIPES];

  RotateStripes((short *)temp, (short *)stripe, rotate);
  SortStripes(temp);
  ClearLineColor();
  SetLineColor((short *)temp);
}

PROFILE(RenderStripes);

static void Render(void) {
  ProfilerStart(RenderStripes);
  {
    RenderStripes(SIN(frameCount * 4) * 2);
  }
  ProfilerStop(RenderStripes);

  CopListRun(cp[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Stripes, NULL, NULL, Init, Kill, Render, NULL);
