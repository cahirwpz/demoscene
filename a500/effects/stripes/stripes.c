#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "fx.h"
#include "random.h"
#include "color.h"

#define WIDTH   320
#define HEIGHT  256
#define SIZE    128
#define STRIPES 20
#define BGCOL   0x204

typedef struct {
  WORD y, z;
  WORD color;
} StripeT;

static CopListT *cp[2];
static CopInsT *lineColor[2][HEIGHT];
static StripeT stripe[STRIPES];
static WORD active = 0;

static UWORD colorSet[4] = { 0xC0F, 0xF0C, 0x80F, 0xF08 };
static UWORD colorShades[4 * 32];

static void GenerateStripes() {
  WORD *s = (WORD *)stripe;
  WORD n = STRIPES;

  while (--n >= 0) {
    *s++ = (random() & (SIZE - 1)) - SIZE / 2;
    *s++ = (random() & (SIZE - 1)) - SIZE / 2;
    *s++ = random() & 0x60;
  }
}

static void GenerateColorShades() {
  WORD i, j;
  UWORD *s = colorSet;
  UWORD *d = colorShades;

  for (i = 0; i < 4; i++) {
    UWORD c = *s++;

    for (j = 0; j < 16; j++)
      *d++ = ColorTransition(0x000, c, j);
    for (j = 0; j < 16; j++)
      *d++ = ColorTransition(c, 0xfff, j);
  }
}

static void MakeCopperList(CopListT *cp, CopInsT **line) {
  WORD i;

  CopInit(cp);
  CopSetRGB(cp, 0, BGCOL);

  for (i = 0; i < HEIGHT; i++) {
    CopWaitSafe(cp, Y(i), 8);
    *line++ = CopSetRGB(cp, 0, 0);
  }

  CopWait(cp, Y(256), 8);
  CopSetRGB(cp, 0, BGCOL);
  CopEnd(cp);
}

static void Init() {
  GenerateStripes();
  GenerateColorShades();

  cp[0] = NewCopList(HEIGHT * 2 + 100);
  cp[1] = NewCopList(HEIGHT * 2 + 100);

  MakeCopperList(cp[0], lineColor[0]);
  MakeCopperList(cp[1], lineColor[1]);

  CopListActivate(cp[0]);
}

static void Kill() {
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static WORD centerY = 0;
static WORD centerZ = 192;

static void RotateStripes(WORD *d, WORD *s, WORD rotate) {
  WORD n = STRIPES;
  LONG cy = centerY << 8;
  WORD cz = centerZ;

  while (--n >= 0) {
    WORD sinA = SIN(rotate);
    WORD cosA = COS(rotate);
    WORD y = *s++;
    WORD z = *s++;
    LONG yp = (y * cosA - z * sinA) >> 4; 
    WORD zp = normfx(y * sinA + z * cosA);

    *d++ = div16(yp + cy, zp + cz);
    *d++ = zp;
    *d++ = *s++;
  }
}

static void ClearLineColor() {
  CopInsT **line = lineColor[active];
  WORD n = HEIGHT;

  while (--n >= 0)
    CopInsSet16(*line++, BGCOL);
}

static void SetLineColor(WORD *s) {
  CopInsT **lines = lineColor[active];
  WORD n = STRIPES;
  UWORD *shades = colorShades;

  while (--n >= 0) {
    WORD y = *s++;
    WORD z = *s++;
    UWORD color = *s++;

    WORD h = (WORD)(z + 128) >> 5;
    WORD l = (z >> 2) + 16;
    WORD i = y + ((WORD)(HEIGHT - h) >> 1);

    if (l < 0)
      l = 0;
    if (l > 31)
      l = 31;

    {
      CopInsT **line = &lines[i];
      WORD c0 = shades[color | l];
      WORD c1 = shades[color | (l >> 1)];

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
  register WORD n asm("d7") = STRIPES - 2;

  do {
    StripeT *curr = ptr;
    StripeT *prev = ptr - 1;
    StripeT this = *ptr++;
    while (prev >= table && prev->z > this.z)
      *curr-- = *prev--;
    *curr = this;
  } while (--n != -1);
}

static void RenderStripes(WORD rotate) {
  static StripeT temp[STRIPES];

  RotateStripes((WORD *)temp, (WORD *)stripe, rotate);
  SortStripes(temp);
  ClearLineColor();
  SetLineColor((WORD *)temp);
}

static void Render() {
  // LONG lines = ReadLineCounter();
  RenderStripes(SIN(frameCount * 4) * 2);
  // Log("hstripes: %ld\n", ReadLineCounter() - lines);

  CopListActivate(cp[active]);
  WaitVBlank();
  active ^= 1;
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
