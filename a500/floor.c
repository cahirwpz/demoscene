#include "blitter.h"
#include "coplist.h"
#include "interrupts.h"
#include "memory.h"
#include "fx.h"

#define X(x) ((x) + 0x81)
#define Y(y) ((y) + 0x2c)

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 2

#define N 1024
#define M 1024
#define ROWS 8
#define COLS 8

#define NEAR_Z 100
#define FAR_Z 300
#define FAR_Y (HEIGHT * NEAR_Z / FAR_Z)
#define FAR_W (WIDTH * FAR_Z / 256)

static BitmapT *screen[2];
static CopInsT *bplptr[DEPTH];
static UWORD active = 0;
static CopListT *cp;

Line2D vert[N];
WORD horiz[M];

static void FloorPrecalc() {
  WORD i;

  for (i = 0; i < N; i++) {
    WORD x = (i - N / 2) * FAR_W / N;
    WORD far_x = WIDTH / 2 + (x * 256) / FAR_Z;
    WORD near_x = WIDTH / 2 + (x * 256) / NEAR_Z;
    WORD near_y = HEIGHT - 1;
    
    if (near_x < 0) {
      near_y = FAR_Y + (0 - far_x) * ((HEIGHT - 1) - FAR_Y) / (near_x - far_x);
      near_x = 0;
    }
    if (near_x > WIDTH - 1) {
      near_y = FAR_Y +  ((WIDTH - 1) - far_x) * ((HEIGHT - 1) - FAR_Y) / (near_x - far_x);
      near_x = WIDTH - 1;
    }
    
    vert[i].x1 = far_x;
    vert[i].y1 = FAR_Y;
    vert[i].x2 = near_x;
    vert[i].y2 = near_y;
  }

  for (i = 0; i < M; i++) {
    WORD z = FAR_Z + ((NEAR_Z - FAR_Z) * i) / M;
    
    horiz[i] = HEIGHT * NEAR_Z / z;
  }
}

void Load() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);

  cp = NewCopList(300);
  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen[active]);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0xff0);
  CopSetRGB(cp, 2, 0x0ff);
  {
    WORD i;

    for (i = 0; i <= FAR_Y; i++) {
      WORD c = i * 15 / FAR_Y;

      CopWait(cp, Y(i), 0);
      CopSetRGB(cp, 3, ((c / 2) << 8) | ((c / 2) << 4) | c );
    }

    CopSetRGB(cp, 3, 0);
  }
  CopEnd(cp);

  FloorPrecalc();
}

void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static volatile LONG swapScreen = -1;
static volatile LONG frameCount = 0;

__interrupt_handler void IntLevel3Handler() {
  if (custom->intreqr & INTF_VERTB) {
    if (swapScreen >= 0) {
      BitmapT *buffer = screen[swapScreen];
      WORD n = DEPTH;

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

static void ClearFloor() {
  BitmapT *buffer = screen[active];
  WORD n = DEPTH;

  custom->bltadat = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;

  while (--n >= 0) {
    custom->bltdpt = buffer->planes[n] + FAR_Y * WIDTH / 8;
    custom->bltsize = ((buffer->height - FAR_Y) << 6) + (buffer->width >> 4);
    WaitBlitter();
  }
}

static void DrawStripes() {
  Line2D first = { 0, 0, WIDTH - 1, FAR_Y };
  Line2D *l[2];

  WORD xo = (N / 4) + normfx(SIN(frameCount * 16) * (N / 4));
  WORD kxo = 7 - xo * COLS / N;
  WORD k;

  BlitterLineSetup(screen[active], 0, LINE_EOR, LINE_ONEDOT);

  for (k = COLS - 1, l[0] = &first; k >= 0; k--) {
    WORD xi = (xo & (N / COLS - 1)) + k * (N / COLS);
    WORD col = k + kxo;

    if ((col & 1) == 0) {
      xi -= 8;
      if (xi < 0)
        xi = 0;
    } else {
      xi += 8;
      if (xi > N - 1)
        xi = N - 1;
    }

    l[1] = &vert[xi & (N - 1)];

    BlitterLine(l[1]->x1, l[1]->y1, l[1]->x2, l[1]->y2);
    WaitBlitter();

    if (((col & 1) == 1) && (l[0]->x2 == WIDTH - 1)) {
      BlitterLine(WIDTH - 1, l[0]->y2, WIDTH - 1, l[1]->y2);
      WaitBlitter();
    }

    l[0] = l[1];
  }

  BlitterLineSetup(screen[active], 1, LINE_EOR, LINE_ONEDOT);

  for (k = COLS - 1, l[0] = &first; k >= 0; k--) {
    WORD xi = (xo & (N / COLS - 1)) + k * (N / COLS);
    WORD col = k + kxo;

    if ((col & 1) == 1) {
      xi -= 8;
      if (xi < 0)
        xi = 0;
    } else {
      xi += 8;
      if (xi > N - 1)
        xi = N - 1;
    }

    l[1] = &vert[xi & (N - 1)];

    BlitterLine(l[1]->x1, l[1]->y1, l[1]->x2, l[1]->y2);
    WaitBlitter();

    if (((col & 1) == 0) && (l[0]->x2 == WIDTH - 1)) {
      BlitterLine(WIDTH - 1, l[0]->y2, WIDTH - 1, l[1]->y2);
      WaitBlitter();
    }

    l[0] = l[1];
  }
}

static void FillStripes() {
  BitmapT *buffer = screen[active];
  WORD n = DEPTH;

  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  while (--n >= 0) {
    UBYTE *bpl = buffer->planes[n] + buffer->bplSize - 1;

    custom->bltapt = bpl;
    custom->bltdpt = bpl;
    custom->bltsize = ((buffer->height - FAR_Y) << 6) + (buffer->width >> 4);
    WaitBlitter();
  }
}

static void HorizontalStripes() {
  BitmapT *buffer = screen[active];
  WORD yo = (M / 2) + normfx(COS(frameCount * 16) * (M / 2));
  // WORD kyo = 7 - yo * ROWS / M;
  WORD yi = yo & (M / ROWS - 1);
  WORD n, k, y1, y2;

  for (k = 0; k <= ROWS; k++, yi += M / ROWS) {
    //WORD row = k + kyo;
    y1 = horiz[(yi < M) ? yi : M - 1];
    y2 = horiz[(yi + 16 < M) ? yi + 16 : M - 1];

    if (y1 == y2)
      continue;

    custom->bltadat = 0;
    custom->bltdmod = 0;
    custom->bltcon0 = DEST;
    custom->bltcon1 = 0;

    n = 2;

    while (--n >= 0) {
      custom->bltdpt = buffer->planes[n] + y1 * WIDTH / 8;
      custom->bltsize = ((y2 - y1) << 6) + (WIDTH >> 4);
      WaitBlitter();
    }
  }
}

BOOL Loop() {
  LONG lines;

  lines = ReadLineCounter();

  ClearFloor();
  DrawStripes();
  FillStripes();
  HorizontalStripes();

  Log("loop: %ld\n", ReadLineCounter() - lines);

  swapScreen = active;

  WaitVBlank();
  active ^= 1;

  return !LeftMouseButton();
}

void Main() {
  InterruptVector->IntLevel3 = IntLevel3Handler;
  custom->intena = INTF_SETCLR | INTF_VERTB;
  
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;


  {
    WORD i;

    for (i = 0; i < 2; i++) {
      BlitterLineSetup(screen[i], 0, LINE_EOR, LINE_ONEDOT);
      BlitterLine(WIDTH - 1, 0, WIDTH - 1, FAR_Y);
      WaitBlitter();
      BlitterFill(screen[i], 0);
      WaitBlitter();

      BlitterLineSetup(screen[i], 1, LINE_EOR, LINE_ONEDOT);
      BlitterLine(WIDTH - 1, 0, WIDTH - 1, FAR_Y);
      WaitBlitter();
      BlitterFill(screen[i], 1);
      WaitBlitter();
    }
  }

  while (Loop());
}
