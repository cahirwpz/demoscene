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

/* Triple buffering. */
static BitmapT *screen[3];
static WORD active = 0;

static Point2D pos[3][3];
static BitmapT *background;
static BitmapT *metaball;
static BitmapT *carry;
static CopInsT *bplptr[5];
static CopListT *cp;

void Load() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  screen[2] = NewBitmap(WIDTH, HEIGHT, 5, FALSE);
  background = LoadILBM("data/metaball-bg.ilbm", FALSE);
  metaball = LoadILBM("data/metaball.ilbm", FALSE);
  carry = NewBitmap(SIZE + 16, SIZE, 2, FALSE);

  cp = NewCopList(100);
  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[active]);
  CopMakeDispWin(cp, 0x81, 0x2c, WIDTH, HEIGHT);
  CopLoadPal(cp, metaball->palette, 0);
  CopEnd(cp);
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(screen[2]);
  DeleteBitmap(carry);
  DeletePalette(background->palette);
  DeleteBitmap(background);
  DeletePalette(metaball->palette);
  DeleteBitmap(metaball);
}

static volatile LONG swapScreen = -1;
static ULONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    if (swapScreen >= 0) {
      BitmapT *buffer = screen[swapScreen];
      WORD n = 5;

      while (--n >= 0) {
        CopInsSet32(bplptr[n], buffer->planes[n]);
        custom->bplpt[n] = buffer->planes[n];
      }

      swapScreen = -1;
    }

    frameCount++;
  }

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

#define BLTOP_NAME ClearMetaball
#define BLTOP_DST_BM screen[active]
#define BLTOP_DST_WIDTH WIDTH
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 5
#include "bltop_clear_simple.h"

#define BLTOP_NAME CopyMetaball
#define BLTOP_SRC_BM metaball
#define BLTOP_DST_BM screen[active]
#define BLTOP_DST_WIDTH WIDTH
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 5
#include "bltop_copy_simple.h"

#define BLTOP_NAME AddMetaball
#define BLTOP_SRC_BM metaball
#define BLTOP_SRC_WIDTH SIZE
#define BLTOP_CARRY_BM carry
#define BLTOP_DST_BM screen[active]
#define BLTOP_DST_WIDTH WIDTH
#define BLTOP_HSIZE SIZE
#define BLTOP_VSIZE SIZE
#define BLTOP_BPLS 5
#include "bltop_adds.h"

static void ClearMetaballs() {
  Point2D *p = pos[active];
  WORD j;

  for (j = 0; j < 3; j++, p++)
    ClearMetaball(p->x, p->y);
}

static void PositionMetaballs() {
  LONG t = frameCount * 24;

  pos[active][0].x = (WIDTH - SIZE) / 2 + normfx(SIN(t) * SIZE * 3 / 4);
  pos[active][0].y = (HEIGHT - SIZE) / 2;

  pos[active][1].x = (WIDTH - SIZE) / 2 - normfx(SIN(t) * SIZE * 3 / 4);
  pos[active][1].y = (HEIGHT - SIZE) / 2;

  pos[active][2].x = (WIDTH - SIZE) / 2;
  pos[active][2].y = (HEIGHT - SIZE) / 2 + normfx(COS(t) * SIZE * 3 / 4);
}

BOOL Loop() {
  LONG lines = ReadLineCounter();

  // This takes about 100 lines. Could we do better?
  ClearMetaballs();
  PositionMetaballs();

  CopyMetaball(pos[active][0].x, pos[active][0].y);
  AddMetaball(pos[active][1].x, pos[active][1].y, 0, 0);
  AddMetaball(pos[active][2].x, pos[active][2].y, 0, 0);
  
  Log("loop: %ld\n", ReadLineCounter() - lines);

  swapScreen = active;

  active++;
  if (active > 2)
    active = 0;

  return !LeftMouseButton();
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;

  ITER(i, 0, 4, {
    BlitterCopySync(screen[0], i, 0, 0, background, i);
    BlitterCopySync(screen[1], i, 0, 0, background, i);
    BlitterCopySync(screen[2], i, 0, 0, background, i);
  });

  while (Loop());
}
