#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "gfx.h"
#include "ilbm.h"
#include "bltop.h"
#include "fx.h"

#define WIDTH 256
#define HEIGHT 144
#define DEPTH 4

static BitmapT *member[5], *logo, *disco, *floor, *dance[8], *foreground;
static CopListT *cp0, *cp1;

static BitmapT *lower;
static Point2D lower_pos;
static Area2D lower_area;

STRPTR __cwdpath = "nigiri";

static void Load() {
  WORD i;

  logo = LoadILBM("16_credits_logo.iff");
  floor = LoadILBM("16_floor.iff");
  disco = LoadILBM("16_discoball.iff");

  dance[0] = LoadILBM("16_01_cahir.iff");
  dance[1] = LoadILBMCustom("16_02_slayer.iff", BM_DISPLAYABLE);
  dance[2] = LoadILBMCustom("16_03_jazzcat.iff", BM_DISPLAYABLE);
  dance[3] = LoadILBMCustom("16_04_dkl.iff", BM_DISPLAYABLE);
  dance[4] = LoadILBMCustom("16_05_dance1.iff", BM_DISPLAYABLE);
  dance[5] = LoadILBMCustom("16_06_dance2.iff", BM_DISPLAYABLE);
  dance[6] = LoadILBMCustom("16_07_dance3.iff", BM_DISPLAYABLE);
  dance[7] = LoadILBMCustom("16_08_dance4.iff", BM_DISPLAYABLE);

  member[0] = LoadILBM("16_txt_cahir.iff");
  member[1] = LoadILBMCustom("16_txt_slay.iff", BM_DISPLAYABLE);
  member[2] = LoadILBMCustom("16_txt_jazz.iff", BM_DISPLAYABLE);
  member[3] = LoadILBMCustom("16_txt_dkl.iff", BM_DISPLAYABLE);
  member[4] = LoadILBMCustom("16_txt_codi.iff", BM_DISPLAYABLE);

  for (i = 1; i < 5; i++)
    member[i]->palette = member[0]->palette;
}

static void UnLoad() {
  WORD i;

  DeletePalette(logo->palette);
  DeleteBitmap(logo);
  DeletePalette(disco->palette);
  DeleteBitmap(disco);
  DeletePalette(floor->palette);
  DeleteBitmap(floor);
  
  DeletePalette(member[0]->palette);
  for (i = 0; i < 5; i++)
    DeleteBitmap(member[i]);

  DeletePalette(dance[0]->palette);
  for (i = 0; i < 8; i++)
    DeleteBitmap(dance[i]);
}

#define DISCO_X X((320 - disco->width) / 2)
#define DISCO_Y Y(0)

#define LOGO_Y Y(256 - 64)

#define FLOOR_X X((320 - floor->width) / 2)
#define FLOOR_Y Y(64)

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0), Y(0), 320, 256); 
  CopMove16(cp, dmacon, DMAF_RASTER);

  /* Display disco ball. */
  CopWait(cp, DISCO_Y - 1, 0);
  CopLoadPal(cp, disco->palette, 0);
  CopSetupMode(cp, MODE_LORES, disco->depth);
  CopSetupBitplanes(cp, NULL, disco, disco->depth);
  CopSetupBitplaneFetch(cp, MODE_LORES, DISCO_X, disco->width);

  CopWait(cp, DISCO_Y, 0);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  CopWait(cp, DISCO_Y + disco->height, 0);
  CopMove16(cp, dmacon, DMAF_RASTER);

  /* Display logo & credits. */
  CopWait(cp, FLOOR_Y - 1, 0);
  CopLoadPal(cp, floor->palette, 0);
  CopLoadPal(cp, dance[0]->palette, 8);
  CopSetupMode(cp, MODE_DUALPF, 6);
  {
    APTR *planes0 = floor->planes;
    APTR *planes1 = foreground->planes;
    WORD i;

    for (i = 0; i < 6;) {
      CopMove32(cp, bplpt[i++], *planes0++);
      CopMove32(cp, bplpt[i++], *planes1++);
    }

    CopMove16(cp, bpl1mod, 0);
    CopMove16(cp, bpl2mod, 0);
  }
  CopSetupBitplaneFetch(cp, MODE_LORES, FLOOR_X, floor->width);

  CopWait(cp, FLOOR_Y, 0);
  CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
  CopWait(cp, FLOOR_Y + floor->height, 0);
  CopMove16(cp, dmacon, DMAF_RASTER);

  /* Display logo and textual credits. */
  if (lower) {
    CopWait(cp, LOGO_Y - 1, 0);
    CopLoadPal(cp, lower->palette, 0);
    CopSetupMode(cp, MODE_LORES, lower->depth);
    CopSetupBitplaneArea(cp, MODE_LORES, lower->depth,
                         lower, X(lower_pos.x), Y(lower_pos.y), &lower_area);

    CopWait(cp, LOGO_Y, 0);
    CopMove16(cp, dmacon, DMAF_SETCLR | DMAF_RASTER);
    CopWait(cp, LOGO_Y + lower->height, 0);
    CopMove16(cp, dmacon, DMAF_RASTER);
  }

  CopEnd(cp);
}

static void Init() {
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_BLITHOG;

  foreground = NewBitmap(max(floor->width, dance[0]->width),
                         max(floor->height, dance[0]->height),
                         floor->depth);
  BitmapClear(foreground);

  lower = NULL;

  cp0 = NewCopList(300);
  cp1 = NewCopList(300);
  MakeCopperList(cp0);
  CopListActivate(cp0);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;

  SetFrameCounter(0);
}

static void Kill() {
  custom->dmacon = DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG;
  DeleteCopList(cp0);
  DeleteCopList(cp1);
  DeleteBitmap(foreground);
}

static void Render() {
  if (frameCount > 600) {
    WORD i = div16(frameCount - 250, 8) & 3;
    lower = logo;
    BitmapCopy(foreground, 80, 0, dance[i + 4]);
  } else if (frameCount > 500) {
    lower = member[4];
    BitmapCopy(foreground, 80, 0, dance[4]);
  } else if (frameCount > 400) {
    lower = member[3];
    BitmapCopy(foreground, 80, 0, dance[3]);
  } else if (frameCount > 300) {
    lower = member[2];
    BitmapCopy(foreground, 80, 0, dance[2]);
  } else if (frameCount > 200) {
    lower = member[1];
    BitmapCopy(foreground, 80, 0, dance[1]);
  } else if (frameCount > 100) {
    lower = member[0];
    BitmapCopy(foreground, 80, 0, dance[0]);
  } else {
    lower = NULL;
  }

  if (lower) {
    static const Box2D window = { 0, 0, 319, 255 }; 

    lower_pos.x = ((320 - lower->width) / 2) + (SIN(frameCount * 32) >> 6);
    lower_pos.y = 256 - 64;
    lower_area.x = 0;
    lower_area.y = 0;
    lower_area.w = lower->width;
    lower_area.h = lower->height;

    if (!ClipBitmap(&window, &lower_pos, &lower_area))
      lower = NULL;
  }

  MakeCopperList(cp1);
  CopListActivate(cp1);
  swapr(cp0, cp1);
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
