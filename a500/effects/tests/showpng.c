#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "png.h"
#include "memory.h"
#include "c2p_1x1_4.h"

static PngT *image;
static PixmapT *pixmap;
static BitmapT *bitmap;
static CopListT *cp;

static void Load() {
  image = ReadPNG("data/curiousity_by_fool2.png", 0);
  PrintPNG(image);
}

static void UnLoad() {
  DeletePNG(image);
}

static void Init() {
  {
    LONG lines = ReadLineCounter();
    pixmap = PixmapFromPNG(image, MEMF_PUBLIC);
    lines = ReadLineCounter() - lines;
    Log("Png decoding took %ld raster lines.\n", (LONG)lines);
  }

  bitmap = NewBitmap(pixmap->width, pixmap->height, 4);
  bitmap->palette = PaletteFromPNG(image);

  {
    LONG lines = ReadLineCounter();
    c2p_1x1_4(pixmap->pixels, bitmap->planes[0], 
              pixmap->width, pixmap->height, bitmap->bplSize);
    lines = ReadLineCounter() - lines;
    Log("Chunky to planar took %ld raster lines.\n", (LONG)lines);
  }

  cp = NewCopList(100);

  {
    WORD w = bitmap->width;
    WORD h = bitmap->height;
    WORD xs = X((320 - w) / 2);
    WORD ys = Y((256 - h) / 2);

    CopInit(cp);
    CopSetupGfxSimple(cp, MODE_LORES, bitmap->depth, xs, ys, w, h);
    CopSetupBitplanes(cp, NULL, bitmap, bitmap->depth);
    CopLoadPal(cp, bitmap->palette, 0);
    CopEnd(cp);

    CopListActivate(cp);
    EnableDMA(DMAF_RASTER);
  }
}

static void Kill() {
  DeletePixmap(pixmap);
  DeleteBitmap(bitmap);
  DeleteCopList(cp);
}

EffectT Effect = { Load, UnLoad, Init, Kill, NULL };
