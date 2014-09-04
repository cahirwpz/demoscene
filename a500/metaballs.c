#include "blitter.h"
#include "coplist.h"
#include "interrupts.h"
#include "memory.h"
#include "ilbm.h"
#include "2d.h"
#include "fx.h"

#define WIDTH 320
#define HEIGHT 256
#define SIZE 80

static BitmapT *metaball;
static BitmapT *carry;
static BitmapT *screen[2];
static UWORD active = 0;
static CopInsT *bplptr[5];
static CopListT *cp;
static Point2D pos[2][3];

void Load() {
  metaball = LoadILBM("data/metaball.ilbm", FALSE);
  carry = NewBitmap(SIZE + 16, SIZE, 2, FALSE);
  screen[0] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  cp = NewCopList(100);

  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[0]);
  CopMakeDispWin(cp, 0x81, 0x2c, WIDTH, HEIGHT);
  CopLoadPal(cp, metaball->palette, 0);
  CopEnd(cp);
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(carry);
  DeletePalette(metaball->palette);
  DeleteBitmap(metaball);
}

static ULONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB)
    frameCount++;

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

__regargs static void ClearMetaballs() {
  Point2D *p = pos[active ^ 1];
  int i, j;

  for (j = 0; j < 3; j++) {
    ULONG start = ((p[j].x & ~15) >> 3) + (p[j].y * WIDTH / 8);
    for (i = 0; i < 5; i++) {
      custom->bltadat = 0;
      custom->bltdpt = screen[active]->planes[i] + start;
      custom->bltdmod = (WIDTH - (SIZE + 16)) / 8;
      custom->bltcon0 = DEST;
      custom->bltcon1 = 0;
      custom->bltsize = ((SIZE + 1) << 6) | ((SIZE + 16) >> 4);
      WaitBlitter();
    }
  }
}

__regargs void CopyMetaball(LONG x, LONG y) {
  ULONG dstart = ((x & ~15) >> 3) + ((WORD)y * WIDTH / 8);
  APTR *src = metaball->planes;
  APTR *dst = screen[active]->planes;
  WORD n = 5;

  custom->bltamod = -2;
  custom->bltdmod = (WIDTH - (SIZE + 16)) / 8;
  custom->bltcon0 = SRCA | DEST | A_TO_D | ((x & 15) << ASHIFTSHIFT);
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = 0;

  while (n--) {
    custom->bltapt = *src++;
    custom->bltdpt = *dst++ + dstart;
    custom->bltsize = (SIZE << 6) | ((SIZE + 16) >> 4);
    WaitBlitter();
  }
}

#define HALF_ADDER ((SRCA | SRCB | DEST) | A_XOR_B)
#define HALF_ADDER_CARRY ((SRCA | SRCB | DEST) | A_AND_B)
#define FULL_ADDER ((SRCA | SRCB | SRCC | DEST) | (NANBC | NABNC | ANBNC | ABC))
#define FULL_ADDER_CARRY ((SRCA | SRCB | SRCC | DEST) | (NABC | ANBC | ABNC | ABC))

__regargs static void AddMetaball(LONG x, LONG y) {
  ULONG start = ((x & ~15) >> 3) + ((WORD)y * WIDTH / 8);
  UWORD shift = (x & 15) << ASHIFTSHIFT;
  BitmapT *buffer = screen[active];
  LONG i, k;

  /* Bitplane adder with saturation. */
  custom->bltamod = -2;
  custom->bltbmod = (WIDTH - (SIZE + 16)) / 8;
  custom->bltcmod = 0;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = 0;

  /* Bitplane 0: half adder with carry. */
  custom->bltapt = metaball->planes[0];
  custom->bltbpt = buffer->planes[0] + start;
  custom->bltdpt = carry->planes[0];
  custom->bltdmod = 0;
  custom->bltcon0 = HALF_ADDER_CARRY | shift;
  custom->bltsize = (SIZE << 6) + ((SIZE + 16) >> 4);
  WaitBlitter();

  custom->bltapt = metaball->planes[0];
  custom->bltbpt = buffer->planes[0] + start;
  custom->bltdpt = buffer->planes[0] + start;
  custom->bltdmod = (WIDTH - (SIZE + 16)) / 8;
  custom->bltcon0 = HALF_ADDER | shift;
  custom->bltsize = (SIZE << 6) + ((SIZE + 16) >> 4);
  WaitBlitter();

  /* Bitplane 1-5: full adder with carry. */
  for (i = 1, k = 0; i < 5; i++, k ^= 1) {
    custom->bltapt = metaball->planes[i];
    custom->bltbpt = buffer->planes[i] + start;
    custom->bltcpt = carry->planes[k];
    custom->bltdpt = carry->planes[k ^ 1];
    custom->bltdmod = 0;
    custom->bltcon0 = FULL_ADDER_CARRY | shift;
    custom->bltsize = (SIZE << 6) + ((SIZE + 16) >> 4);
    WaitBlitter();

    custom->bltapt = metaball->planes[i];
    custom->bltbpt = buffer->planes[i] + start;
    custom->bltcpt = carry->planes[k];
    custom->bltdpt = buffer->planes[i] + start;
    custom->bltdmod = (WIDTH - (SIZE + 16)) / 8;
    custom->bltcon0 = FULL_ADDER | shift;
    custom->bltsize = (SIZE << 6) + ((SIZE + 16) >> 4);
    WaitBlitter();
  }

  /* Apply saturation bits. */
  {
    WORD n = 5;

    custom->bltamod = (WIDTH - (SIZE + 16)) / 8;
    custom->bltbmod = 0;
    custom->bltdmod = (WIDTH - (SIZE + 16)) / 8;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltalwm = -1;

    while (--n >= 0) {
      custom->bltapt = buffer->planes[n] + start;
      custom->bltbpt = carry->planes[k];
      custom->bltdpt = buffer->planes[n] + start;
      custom->bltsize = (SIZE << 6) + ((SIZE + 16) >> 4);
      WaitBlitter();
    }
  }
}

static void PositionMetaballs() {
  LONG t = frameCount * 16;

  pos[active][0].x = (WIDTH - SIZE) / 2 + normfx(SIN(t) * SIZE * 2 / 3);
  pos[active][0].y = (HEIGHT - SIZE) / 2;

  pos[active][1].x = (WIDTH - SIZE) / 2 - normfx(SIN(t) * SIZE * 2 / 3);
  pos[active][1].y = (HEIGHT - SIZE) / 2;

  pos[active][2].x = (WIDTH - SIZE) / 2;
  pos[active][2].y = (HEIGHT - SIZE) / 2 + normfx(COS(t) * SIZE);
}

BOOL Loop() {
  LONG lines = ReadLineCounter();

  PositionMetaballs();

  // This takes about 100 lines. Could we do better?
  ClearMetaballs();

  CopyMetaball(pos[active][0].x, pos[active][0].y);
  AddMetaball(pos[active][1].x, pos[active][1].y);
  AddMetaball(pos[active][2].x, pos[active][2].y);
  
  Log("loop: %ld\n", ReadLineCounter() - lines);

  WaitBlitter();
  WaitVBlank();

  {
    int i;
    for (i = 0; i < 5; i++) {
      CopInsSet32(bplptr[i], screen[active]->planes[i]);
      custom->bplpt[i] = screen[active]->planes[i];
    }
  }

  active ^= 1;

  return !LeftMouseButton();
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;

  while (Loop());
}
