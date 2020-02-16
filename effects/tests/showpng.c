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
    int lines = ReadLineCounter();
    pixmap = PixmapFromPNG(image, MEMF_PUBLIC);
    lines = ReadLineCounter() - lines;
    Log("Png decoding took %d raster lines.\n", lines);
  }

  bitmap = NewBitmap(pixmap->width, pixmap->height, 4);
  bitmap->palette = PaletteFromPNG(image);

  {
    int lines = ReadLineCounter();
    c2p_1x1_4(pixmap->pixels, bitmap->planes[0], 
              pixmap->width, pixmap->height, bitmap->bplSize);
    lines = ReadLineCounter() - lines;
    Log("Chunky to planar took %d raster lines.\n", lines);
  }

  cp = NewCopList(100);

  {
    short w = bitmap->width;
    short h = bitmap->height;
    short xs = X((320 - w) / 2);
    short ys = Y((256 - h) / 2);

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
