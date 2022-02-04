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

static const BitmapT bitmap0 = {
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

static const BitmapT bitmap1 = {
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

// static bool bb_show_zero = true;

// external source image

#include "data/start.c"

static CopListT *cp;

static void Init(void) {
  custom->color[0] = 0;
  custom->color[1] = 0xfff;

  SetupPlayfield(MODE_LORES, 1, X(0), Y(0), BB_WIDTH, BB_HEIGHT);

  cp = NewCopList(80);
  CopInit(cp);
  CopSetupBitplanes(cp, NULL, &img_source, 1);

  CopListActivate(cp);

  EnableDMA(DMAF_BLITTER | DMAF_RASTER);
}

static void Kill(void) {
}

PROFILE(BlitzBlit);


static void Render(void) {
  ProfilerStart(BlitzBlit);
  {
  static u_short c = 0;
  custom->color[0] = c++;
  }
  ProfilerStop(BlitzBlit);

  TaskWaitVBlank();
}

EFFECT(empty, NULL, NULL, Init, Kill, Render);
