#include "effect.h"

/* Search following header files for useful procedures. */
#include "custom.h"     /* custom registers definition and misc functions */   
#include "interrupt.h"  /* register & unregister an interrupt handler */
#include "blitter.h"    /* blitter handling routines */
#include "color.h"      /* operations on RGB colors */
#include "copper.h"     /* copper list construction */
#include "bitmap.h"     /* bitmap structure */
#include "palette.h"    /* palette structure */
#include "pixmap.h"     /* pixel map (chunky) structure */
#include "memory.h"     /* dynamic memory allocator */
#include "sprite.h"     /* sprite structure and copper list helpers */


// buffers / bitplanes which are displayed
#define BB_WIDTH 320
#define BB_HEIGHT 256
#define BB_BPR (BB_WIDTH / 8)

static __data_chip u_short bplbuf0[BB_BPR * BB_HEIGHT / 2];
static __data_chip u_short bplbuf1[BB_BPR * BB_HEIGHT / 2];

static BitmapT bitbuf0 = {
  .width = 320,
  .height = 256,
  .depth = 1,
  .bytesPerRow = 40,
  .bplSize = 10240,
  .flags = BM_DISPLAYABLE|BM_STATIC,
  .planes = {
    (void *)bplbuf0
  }
};

static BitmapT bitbuf1 = {
  .width = 320,
  .height = 256,
  .depth = 1,
  .bytesPerRow = 40,
  .bplSize = 10240,
  .flags = BM_DISPLAYABLE|BM_STATIC,
  .planes = {
    (void *)bplbuf1
  }
};

static BitmapT* bitmap_curr = &bitbuf0;
static BitmapT* bitmap_back = &bitbuf1;
static uint8_t minterm_curr = 0;

// external source image

#include "data/start.c"

static CopListT *cp;

static void Init(void) {
  custom->color[0] = 0;
  custom->color[1] = 0xfff;

  SetupPlayfield(MODE_LORES, 1, X(0), Y(0), BB_WIDTH, BB_HEIGHT);

  cp = NewCopList(80);
  CopInit(cp);
  CopSetupBitplanes(cp, NULL, bitmap_curr, 1);
  CopListActivate(cp);

  EnableDMA(DMAF_BLITTER | DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);
  DeleteCopList(cp);
}

// BB internal functions

// nonblocking
static void BLITZ_Load(u_short idx) {
    (void)idx;
    BitmapCopyFast(bitmap_back, 0, 0, &img_source);
}

// nonblocking
static void BLITZ_Clear(void) {
  BlitterClear(bitmap_back, 0);
}

// blocking
static void BLITZ_Render(void) {
  const BitmapT *dst = bitmap_back;
  const BitmapT *src = bitmap_curr;

  u_short bytesPerRow = (src->width - 32) >> 3;
  u_short blitmods = src->bytesPerRow - bytesPerRow;

  //__C__
  //_AD__  D = f(A, B, C)
  //___B_

  // offsets (hpixels, rows)
  // A = (-1,  0)
  // B = (+1, +1)
  // C = ( 0, -1)
  // can't have negative x offsets, so let's add 16:
  // A = (15,  0) -> start 0, shift 15
  // B = (17,  1) -> start 1, shift 1
  // C = (16, -1) -> start 1, shift 0

  WaitBlitter();
  // REMOVE CLEARING
  BlitterClear(bitmap_back, 0);
  WaitBlitter();

  custom->bltamod = blitmods;
  custom->bltbmod = blitmods;
  custom->bltcmod = blitmods;
  custom->bltdmod = blitmods;

  custom->bltafwm = FirstWordMask[15];
  custom->bltalwm = LastWordMask[15];

  //                  SHIFT A                                               MINTERM 
  // custom->bltcon0 = rorw((uint16_t)15, 4) | SRCA | SRCB | SRCC | DEST | (minterm_curr & 0xff);
  custom->bltcon0 = rorw((uint16_t)15, 4) | SRCA | SRCB | SRCC | DEST | A_TO_D;

  //                 SHIFT B
  custom->bltcon1 = rorw((uint16_t)1, 4);

  custom->bltapt = src->planes[0]     + src->bytesPerRow;
  custom->bltbpt = src->planes[0] + 2 + 2 * src->bytesPerRow;
  custom->bltcpt = src->planes[0] + 2;
  custom->bltdpt = dst->planes[0] + 2 + src->bytesPerRow;

  custom->bltsize = ((src->height-2) << 6) | (bytesPerRow >> 1);
}

#include "generated.c"


PROFILE(BlitzBlit);
static void Render(void) {
  ProfilerStart(BlitzBlit);
  {
    BLITZ_MainLoop();
  }
  ProfilerStop(BlitzBlit);


  TaskWaitVBlank();
  WaitBlitter();

  //swap buffers during vblank
  {
    BitmapT* tmp_ = bitmap_curr;
    bitmap_curr = bitmap_back;
    bitmap_back = tmp_;

    // update coplist
    CopInsSet32(&cp->entry[0], bitmap_curr->planes[0]);
  }
}

EFFECT(empty, NULL, NULL, Init, Kill, Render);
