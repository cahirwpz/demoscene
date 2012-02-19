#include "std/types.h"

#include <proto/graphics.h>
#include <proto/intuition.h>
#include <graphics/videocontrol.h>
#include <graphics/gfxbase.h>
#include <intuition/screens.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>

#include "std/debug.h"
#include "std/memory.h"
#include "display.h"

typedef struct ViewPort ViewPortT;
typedef struct Screen ScreenT;

typedef struct ScreenPalette {
  UWORD Count;
  UWORD Start;
  ULONG Colors[256*3+1];
} ScreenPaletteT;

typedef struct Display {
  ScreenPaletteT *Palette;
  BitMapT *Bitmap[2];
  ScreenT *Screen;

  LONG CurrentBitMap;
} DisplayT;

static DisplayT *Display = NULL;

/*
 * Screen handling functions.
 */

void LoadPalette(PaletteT *palette) {
  size_t j = 0;

  Display->Palette->Start = palette->start;
  Display->Palette->Count = 0;

  while (palette) {
    size_t i;

    for (i = 0; i < palette->count; i++) {
      Display->Palette->Colors[j++] = palette->colors[i].r << 24;
      Display->Palette->Colors[j++] = palette->colors[i].g << 24;
      Display->Palette->Colors[j++] = palette->colors[i].b << 24;
    }

    Display->Palette->Count += palette->count;
 
    palette = palette->next;
  }

  Display->Palette->Colors[j] = 0;

  LoadRGB32(&Display->Screen->ViewPort, (ULONG *)Display->Palette);
}

/*
 * Display handling functions.
 */

static struct TagItem VideoCtrlTags[] = {
  { VTAG_BORDERBLANK_SET, TRUE },
  { VTAG_BORDERSPRITE_SET, TRUE },
  { VTAG_SPRITERESN_SET, SPRITERESN_ECS },
  { VTAG_END_CM, 0L }
};

static struct TagItem ScreenTags[] = {
  { SA_Width, 0 },
  { SA_Height, 0 },
  { SA_Depth, 0 },
  { SA_BitMap, 0 },
  { SA_Colors32, 0 },
  { SA_VideoControl, (ULONG)VideoCtrlTags },
  { SA_DisplayID, PAL_MONITOR_ID },
  { SA_Type, CUSTOMSCREEN | CUSTOMBITMAP | SCREENQUIET },
  { SA_ShowTitle, FALSE },
  { SA_Draggable, FALSE },
  { TAG_END, 0 }
};

bool InitDisplay(int width, int height, int depth) {
  if (!Display) {
    size_t i, j;

    Display = NEW_S(DisplayT);

    for (i = 0; i < 2; i++)
      Display->Bitmap[i] = AllocBitMap(width, height, depth,
                                       BMF_DISPLAYABLE|BMF_CLEAR, NULL);

    Display->Palette = NEW_S(ScreenPaletteT);
    Display->Palette->Count = 256;

    for (i = 0, j = 0; i < 256; i++) {
      Display->Palette->Colors[j++] = i << 24;
      Display->Palette->Colors[j++] = i << 24;
      Display->Palette->Colors[j++] = i << 24;
    }

    ScreenTags[0].ti_Data = width;
    ScreenTags[1].ti_Data = height;
    ScreenTags[2].ti_Data = depth;
    ScreenTags[3].ti_Data = (ULONG)Display->Bitmap[0];
    ScreenTags[4].ti_Data = (ULONG)Display->Palette;

    if ((Display->Screen = OpenScreenTagList(0L, ScreenTags))) {
      Display->CurrentBitMap = 1;
      return TRUE;
    } else {
      KillDisplay();
    }
  }

  return FALSE;
}

void KillDisplay() {
  if (Display) {
    size_t i;

    CloseScreen(Display->Screen);

    for (i = 0; i < 2; i++)
      FreeBitMap(Display->Bitmap[i]);

    DELETE(Display->Palette);
    DELETE(Display);

    Display = NULL;
  }
}

void DisplaySwap() {
  struct Custom *custom = (void *)0xdff000;

  Display->Screen->ViewPort.RasInfo->BitMap = GetCurrentBitMap();
  Display->CurrentBitMap ^= 1;
  Display->Screen->RastPort.BitMap = GetCurrentBitMap();

  MakeScreen(Display->Screen);
  RethinkDisplay();
  custom->dmacon = BITCLR|DMAF_SPRITE;
}

BitMapT *GetCurrentBitMap() {
  return Display->Bitmap[Display->CurrentBitMap];
}

RastPortT *GetCurrentRastPort() {
  return &Display->Screen->RastPort;
}
