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

static void GenerateStripes() {
  WORD *s = (WORD *)stripe;
  WORD n = STRIPES;

  while (--n >= 0) {
    *s++ = (random() & (SIZE - 1)) - SIZE / 2;
    *s++ = (random() & (SIZE - 1)) - SIZE / 2;
    *s++ = colorSet[random() & 3];
  }
}

static void MakeCopperList(CopListT *cp, CopInsT **line) {
  WORD i;

  CopInit(cp);
  CopSetRGB(cp, 0, BGCOL);

  for (i = 0; i < HEIGHT; i++) {
    CopWait(cp, Y(i), 8);
    *line++ = CopSetRGB(cp, 0, 0);
  }

  CopWait(cp, Y(256), 8);
  CopSetRGB(cp, 0, BGCOL);
  CopEnd(cp);
}

void InitColorTab();

static void Init() {
  GenerateStripes();

  cp[0] = NewCopList(HEIGHT * 2 + 100);
  cp[1] = NewCopList(HEIGHT * 2 + 100);

  MakeCopperList(cp[0], lineColor[0]);
  MakeCopperList(cp[1], lineColor[1]);

  CopListActivate(cp[0]);
  InitColorTab();
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

  while (--n >= 0) {
    WORD y = *s++;
    WORD z = *s++;
    UWORD c = *s++;

    WORD h = (z + 128) / 32;
    WORD s = z * 32 / SIZE + 16;
    WORD i = y + (HEIGHT - h) / 2;

    if (s < 0)
      c = 0;
    else if (s < 16)
      c = ColorTransition(0x000, c, s);
    else if (s < 32)
      c = ColorTransition(c, 0xfff, s - 16);
    else
      c = 0xfff;
    
    {
      CopInsT **line = &lines[i];

      while (--h >= 0)
        CopInsSet16(*line++, c);
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
