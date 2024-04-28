#include "effect.h"
#include "copper.h"
#include "pixmap.h"
#include "blitter.h"
#include "gfx.h"

#define WIDTH 320
#define HEIGHT 176
#define DEPTH 3

#define LINE_W (WIDTH + 16)
#define W 16
#define H 16

#include "data/gradient.c"
#include "data/layer0.c"
#include "data/layer1-map.c"
#include "data/layer1-tiles.c"
#include "data/layer2-map.c"
#include "data/layer2-tiles.c"

static BitmapT *background_bm, *foreground_bm, *unscroll_1;
static BitmapT background[1], foreground[1], unscroll_0[1];
static CopListT *cp;
static CopInsT *bplcon1;
static CopInsT *bplptr[DEPTH + DEPTH];

static inline void CopyTileSetup(void) {
  WaitBlitter();

  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltamod = 0;
  custom->bltdmod = (LINE_W - W) >> 3;
}

static inline void CopyTileStart(void *dstbpt, void *srcbpt) {
  WaitBlitter();

  custom->bltapt = srcbpt;
  custom->bltdpt = dstbpt;
  custom->bltsize = (H << 6) | (W >> 4);
}

static __regargs void UpdateLayer1(int dstX, int srcX, short width) {
  void *const *src = layer1_tiles.planes;
  void *const *dst = background->planes + 1;
  short *tiles = &layer1_map[0][srcX];
  int dst_start = dstX * 2;
  short h = HEIGHT / 16;

  CopyTileSetup();

  do {
    short w = width;
    do {
      short tile = *tiles++;
      int src_start = tile * H * (W >> 3);
      CopyTileStart(dst[0] + dst_start, src[0] + src_start);
      CopyTileStart(dst[1] + dst_start, src[1] + src_start);
      dst_start += 2;
    } while (--w);
    tiles += layer1_map_width - width;
    dst_start += H * LINE_W / 8 - width * 2;
  } while (--h);
}

static __regargs void UpdateLayer2(int dstX, int srcX, short width) {
  void *const *src = layer2_tiles.planes;
  void *const *dst = foreground->planes;
  short *tiles = &layer2_map[0][srcX]; 
  int dst_start = dstX * 2;
  short h = HEIGHT / 16;

  CopyTileSetup();

  do {
    short w = width;
    do {
      short tile = *tiles++;
      int src_start = tile * H * (W >> 3);
      CopyTileStart(dst[0] + dst_start, src[0] + src_start);
      CopyTileStart(dst[1] + dst_start, src[1] + src_start);
      CopyTileStart(dst[2] + dst_start, src[2] + src_start);
      dst_start += 2;
    } while (--w);
    tiles += layer2_map_width - width;
    dst_start += H * LINE_W / 8 - width * 2;
  } while (--h);
}

static void CopSetupDualPlayfield(CopListT *list, CopInsT **bplptr,
                                  const BitmapT *pf1, const BitmapT *pf2)
{
  void **planes1 = pf1->planes;
  void **planes2 = pf2->planes;
  short n = pf1->depth + pf2->depth - 1;
  short i = 0;

  do {
    void *plane = i & 1 ? *planes1++ : *planes2++;
    CopInsT *ins = CopMove32(list, bplpt[i++], plane);

    if (bplptr)
      *bplptr++ = ins;
  } while (--n != -1);


  {
    short bpl1mod = (short)pf1->bytesPerRow * (short)(pf1->depth - 1);
    short bpl2mod = (short)pf2->bytesPerRow * (short)(pf2->depth - 1);

    CopMove16(list, bpl1mod, pf1->flags & BM_INTERLEAVED ? bpl1mod : 0);
    CopMove16(list, bpl2mod, pf2->flags & BM_INTERLEAVED ? bpl2mod : 0);
  }
}

static void Init(void) {
  const short ys = (256 - layer0.height) / 2;
  short i;

  background_bm = NewBitmapCustom(LINE_W, HEIGHT, DEPTH, BM_DISPLAYABLE);
  foreground_bm = NewBitmapCustom(LINE_W, HEIGHT + 6, DEPTH, BM_DISPLAYABLE);
  unscroll_1 = NewBitmapCustom(LINE_W, HEIGHT, 1, BM_DISPLAYABLE);

  InitSharedBitmap(background, LINE_W, HEIGHT, DEPTH, background_bm);
  InitSharedBitmap(foreground, LINE_W, HEIGHT, DEPTH, foreground_bm);
  InitSharedBitmap(unscroll_0, LINE_W, HEIGHT, 1, background_bm);

  cp = NewCopList(100 + HEIGHT * 5);

  CopInit(cp);
  CopSetupMode(cp, MODE_LORES|MODE_DUALPF, 2 * DEPTH);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(ys), WIDTH, HEIGHT);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(0) - 16, LINE_W);

  /* Palette */
  CopSetColor(cp, 0, 0x000);
  CopSetColor(cp, 1, 0x000);
  for (i = 1; i < layer1_pal.count; i++) {
    CopSetColor(cp, 2*i, layer1_pal.colors[i]);
    CopSetColor(cp, 2*i+1, layer1_pal.colors[i]);
  }
  CopLoadPal(cp, &layer2_pal, 8);

  CopSetupDualPlayfield(cp, bplptr, foreground, background);
  bplcon1 = CopMove16(cp, bplcon1, 0);

  {
    u_short *color = gradient.pixels;

    for (i = 0; i < HEIGHT; i++) {
      u_short c0 = color[HEIGHT - i - 1];
      u_short c1 = ~color[i];
      CopWait(cp, VP(ys + i), HP(0) - 14);
      CopSetColor(cp, 1, c1);
      CopSetColor(cp, 0, c0);
      CopWait(cp, VP(ys + i), HP(WIDTH) - 1);
      CopSetColor(cp, 0, 0);
    }
  }

  CopSetColor(cp, 0, 0x000);
  CopEnd(cp);

  CopListActivate(cp);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  UpdateLayer1(0, 0, LINE_W / 16);
  UpdateLayer2(0, 0, LINE_W / 16);

  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp);
  DeleteBitmap(foreground_bm);
  DeleteBitmap(background_bm);
  DeleteBitmap(unscroll_1);
}

static short ScrollLayer1(short t) {
  short shift = 15 - (t % 16);

  if (shift == 15) {
    short k = (t >> 4) % (layer1_map_width - WIDTH / 16);
    short i;

    for (i = 1; i < DEPTH; i++)
      background->planes[i] = background_bm->planes[i] + k * 2;

    if (k == 0)
      UpdateLayer1(0, 0, LINE_W / 16);
    else
      UpdateLayer1(WIDTH / 16, WIDTH / 16 + k, 1);

    CopInsSet32(bplptr[2], background->planes[1]);
    CopInsSet32(bplptr[4], background->planes[2]);
  }

  return shift;
}

static short ScrollLayer2(short t) {
  short shift = 15 - (t % 16);

  if (shift == 15) {
    short k = (t >> 4) % (layer2_map_width - WIDTH / 16);
    short i;

    for (i = 0; i < DEPTH; i++)
      foreground->planes[i] = foreground_bm->planes[i] + k * 2;

    if (k == 0)
      UpdateLayer2(0, 0, LINE_W / 16);
    else
      UpdateLayer2(WIDTH / 16, WIDTH / 16 + k, 1);

    CopInsSet32(bplptr[1], foreground->planes[0]);
    CopInsSet32(bplptr[3], foreground->planes[1]);
    CopInsSet32(bplptr[5], foreground->planes[2]);
  }

  return shift;
}

static void Render(void) {
  // int lines = ReadLineCounter();
  {
    short l1_shift = ScrollLayer1(frameCount / 2);
    short l2_shift = ScrollLayer2(frameCount);
    short shift = (l2_shift << 4) | l1_shift;

    {
      BitmapT *unscroll = (l1_shift % 2) ? unscroll_0 : unscroll_1;
      BitmapCopyFast(unscroll, 15 - l1_shift, 0, &layer0);
      CopInsSet32(bplptr[0], unscroll->planes[0]);
    }

    CopInsSet16(bplcon1, shift);
  }
  // Log("neoncity: %d\n", ReadLineCounter() - lines);

  TaskWaitVBlank();
}

EFFECT(neoncity, NULL, NULL, Init, Kill, Render);
