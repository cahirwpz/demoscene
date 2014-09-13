#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "interrupts.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

#define WIDTH 80
#define HEIGHT 128

static BitmapT *screen;
static CopListT *cp;
static PixmapT *chunky;

CopInsT *lines[256];

__interrupt_handler void IntLevel3Handler() {
  static UWORD frameNumber = 0;

  if (custom->intreqr & INTF_VERTB)
    frameNumber++;

  custom->intreq = INTF_LEVEL3;
  custom->intreq = INTF_LEVEL3;
}

void Load() {
  cp = NewCopList((WIDTH + 2) * HEIGHT + 100);
  screen = NewBitmap(WIDTH * 4, HEIGHT * 2, 1, FALSE);
  chunky = NewPixmap(WIDTH + 2, HEIGHT, PM_RGB4, MEMF_CHIP);

  /*
   * Chunky buffer has one "unused" pixel at the beginning of each half-line.
   * Actually it's the last word of copper wait instruction.
   *
   * Moreover the buffer is scrambled. Even pixels are moved to even half-lines
   * and odd pixels to odd half-lines respectively.
   *
   * This way I can quickly copy whole buffer to copper list with blitter.
   *
   * Note that normally, effect will (probably) render directly to copper list.
   */
  {
    PixmapT *image = LoadTGA("data/img-rgb.tga", PM_RGB4, MEMF_PUBLIC);
    UWORD *src = (WORD *)image->pixels;
    UWORD *even = (UWORD *)chunky->pixels;
    UWORD *odd = (UWORD *)chunky->pixels + (WIDTH + 2) / 2;
    WORD h = HEIGHT;

    while (h--) {
      WORD w = WIDTH / 4;

      *even++ = 0xfffe;
      *odd++ = 0xfffe;

      while (w--) {
        *even++ = *src++;
        *odd++ = *src++;
        *even++ = *src++;
        *odd++ = *src++;
      }

      even += (WIDTH + 2) / 2;
      odd += (WIDTH + 2) / 2;
    }

    DeletePixmap(image);
  }

  {
    UWORD *plane = screen->planes[0];
    UWORD i, j;

    for (j = 0; j < HEIGHT * 2; j += 2) {
      for (i = 0; i < WIDTH * 4 / 16; i++)
        *plane++ = 0x5f5f;
      for (i = 0; i < WIDTH * 4 / 16; i++)
        *plane++ = 0xf5f5;
    }
  }

  {
    UWORD i, j;

    CopInit(cp);
    CopMakePlayfield(cp, NULL, screen);
    CopMakeDispWin(cp, X(3), Y(0), screen->width, screen->height);
    for (i = 0; i < 32; i++)
      CopSetRGB(cp, i, 0);
    for (j = 0; j < HEIGHT * 2; j += 2) {
      lines[j] = CopWait(cp, Y(j) & 255, 0x3e);
      for (i = 0; i < WIDTH / 2; i++)
        CopSetRGB(cp, 1, 0);

      lines[j + 1] = CopWait(cp, Y(j + 1) & 255, 0x40);
      for (i = 0; i < WIDTH / 2; i++)
        CopSetRGB(cp, 1, 0);
    }
    CopEnd(cp);
  }

  Log("Copper list entries: %ld.\n", (LONG)(cp->curr - cp->entry));
}

void Kill() {
  DeleteCopList(cp);
  DeletePixmap(chunky);
  DeleteBitmap(screen);
}

static void ChunkyToCopList() {
  UWORD *pixels = chunky->pixels;
  WORD h = HEIGHT / 8;

  custom->bltamod = 0;
  custom->bltdmod = 2;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltapt = pixels;
  custom->bltdpt = (APTR)(lines[0]) + 2;

  while (h--) {
    custom->bltsize = (((WIDTH + 2) * 8) << 6) | 1;
    WaitBlitter();
  }
}

void Init() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_LEVEL3;

  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER;
}

void Main() {
  while (!LeftMouseButton()) {
    LONG lines = ReadLineCounter();
    ChunkyToCopList();
    Log("copy: %ld\n", ReadLineCounter() - lines);

    WaitVBlank();
  }
}
