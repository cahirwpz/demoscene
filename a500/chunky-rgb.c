#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "interrupts.h"

static BitmapT *screen;
static CopListT *cp;
static PixmapT *image;

CopInsT *lines[256];

__interrupt_handler void IntLevel3Handler() {
  static UWORD frameNumber = 0;

  if (custom->intreqr & INTF_VERTB)
    frameNumber++;

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

void Load() {
  cp = NewCopList(256 * 42);
  screen = NewBitmap(320, 256, 1, FALSE);
  image = LoadTGA("data/img-rgb.tga", PM_RGB4);

  {
    UWORD *plane = screen->planes[0];
    UWORD i, j;

    for (j = 0; j < 256; j += 2) {
      for (i = 0; i < 320 / 16; i++)
        *plane++ = 0x5f5f;
      for (i = 0; i < 320 / 16; i++)
        *plane++ = 0xf5f5;
    }
  }

  {
    UWORD i, j;
    UWORD *pixels = image->pixels;

    CopInit(cp);
    CopMakePlayfield(cp, NULL, screen);
    CopMakeDispWin(cp, 0x84, 0x2c, screen->width, screen->height);
    for (i = 0; i < 32; i++)
      CopSetRGB(cp, i, 0);
    for (j = 0; j < 128; j++) {
      CopWait(cp, 0x2c + j * 2, 0x3e);
      for (i = 0; i < 40; i++)
        CopSetRGB(cp, 1, pixels[j * 80 + 2 * i]);

      CopWait(cp, 0x2c + j * 2 + 1, 0x40);
      for (i = 0; i < 40; i++)
        CopSetRGB(cp, 1, pixels[j * 80 + 2 * i + 1]);
    }
    CopEnd(cp);
  }

  Log("Copper list entries: %ld.\n", (LONG)(cp->curr - cp->entry));
}

void Kill() {
  DeleteCopList(cp);
  DeletePixmap(image);
  DeleteBitmap(screen);
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_LEVEL3;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;

  WaitMouse();
}
