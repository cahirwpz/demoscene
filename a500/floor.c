#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "fx.h"
#include "random.h"

#define WIDTH 320
#define HEIGHT 212
#define DEPTH 2
#define CPX 0x20

#define N 1024
#define SIZE 8
#define TILES (2 * SIZE)
#define TILESIZE (N / SIZE)

#define STRIDE ((SIZE * 2 + 1) * sizeof(CopInsT))

#define NEAR_Z 100
#define FAR_Z 300
#define FAR_Y (HEIGHT * NEAR_Z / FAR_Z)
#define FAR_W (WIDTH * FAR_Z / 256)

static BitmapT *screen[2];
static UWORD active = 0;
static CopListT *cp[2];

static Line2D vert[N];
static WORD horiz[N];
static UBYTE *linePos[2][SIZE];
static UWORD *lineColor[2][SIZE];
static UWORD tileColumn[HEIGHT];
static UWORD texture[TILES * TILES];

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

  for (i = 0; i < N; i++) {
    WORD z = FAR_Z + ((NEAR_Z - FAR_Z) * i) / N;
    
    horiz[i] = HEIGHT * NEAR_Z / z;
  }
}

static void MakeCopperList(CopListT *cp, WORD num) {
  CopInsT *ins;
  WORD i, j;

  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), WIDTH, HEIGHT);
  CopShowPlayfield(cp, screen[num]);
  CopSetRGB(cp, 0, 0);
  CopSetRGB(cp, 1, 0);
  CopSetRGB(cp, 2, 0);
  CopSetRGB(cp, 3, 0);

  for (i = 0; i < FAR_Y; i++) {
    WORD c = i * 15 / FAR_Y;

    CopWait(cp, Y(i), 0);
    CopSetRGB(cp, 3, ((c / 2) << 8) | ((c / 2) << 4) | c );
  }

  for (i = FAR_Y; i < HEIGHT; i++) {
    CopWait(cp, Y(i), CPX);
    for (j = 0; j < SIZE; j++) {
      ins = CopWait(cp, Y(i), CPX);
      if (i == FAR_Y)
        linePos[num][j] = ((UBYTE*)ins) + 1;
      ins = CopSetRGB(cp, (j & 1) + 1, (j & 1) ? 0xff0 : 0x0ff);
      if (i == FAR_Y)
        lineColor[num][j] = ((UWORD*)ins);
    }
  }

  CopWait(cp, Y(i) & 255, 0);
  CopSetRGB(cp, 0, 0);
  CopSetRGB(cp, 1, 0);
  CopSetRGB(cp, 2, 0);
  CopSetRGB(cp, 3, 0);
  CopEnd(cp);
}

static void Load() {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, FALSE);

  cp[0] = NewCopList((256 - FAR_Y) * STRIDE / sizeof(CopInsT) + 300);
  MakeCopperList(cp[0], 0);
  cp[1] = NewCopList((256 - FAR_Y) * STRIDE / sizeof(CopInsT) + 300);
  MakeCopperList(cp[1], 1);

  FloorPrecalc();

  ITER(i, 0, TILES * TILES - 1, texture[i] = random() & 0xfff);
}

static void Kill() {
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static void ClearFloor() {
  BitmapT *buffer = screen[active];
  WORD n = DEPTH;

  WaitBlitter();
  custom->bltadat = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;

  while (--n >= 0) {
    APTR bltdpt = buffer->planes[n] + FAR_Y * WIDTH / 8;
    UWORD bltsize = ((buffer->height - FAR_Y) << 6) + (buffer->width >> 4);

    WaitBlitter();
    custom->bltdpt = bltdpt;
    custom->bltsize = bltsize;
  }
}

static __regargs void CopperLine(UBYTE *pos, WORD x1, WORD y1, WORD x2, WORD y2) {
  x1 = X(x1) / 2;
  x2 = X(x2) / 2;
  y1 -= FAR_Y; /* this one is always going to be zero */
  y2 -= FAR_Y;

  if (y1 != y2) {
    WORD dy = y2 - y1;
    WORD dx = x2 - x1;
    LONG z = x1 << 16;
    /* This is a long division! Precalculate if possible... */
    LONG dz = (dx << 16) / dy;
    register UBYTE one asm("d7") = 1;

    while (--dy >= -1) {
      UWORD x = swap16(z);
      *pos = x | one;
      pos += STRIDE;
      z += dz;
    }
  }
}

static void DrawStripes(WORD xo, WORD kxo) {
  Line2D first = { 0, 0, WIDTH - 1, FAR_Y };
  Line2D *l[2];
  WORD k;

  /* Even stripes. */
  WaitBlitter();
  BlitterLineSetup(screen[active], 0, LINE_EOR, LINE_ONEDOT);

  for (k = SIZE - 1, l[0] = &first; k >= 0; k--) {
    WORD xi = (xo & (TILESIZE - 1)) + k * TILESIZE;
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

    BlitterLineSync(l[1]->x1, l[1]->y1, l[1]->x2, l[1]->y2);

    if (((col & 1) == 1) && (l[0]->x2 == WIDTH - 1))
      BlitterLineSync(WIDTH - 1, l[0]->y2, WIDTH - 1, l[1]->y2);

    l[0] = l[1];
  }

  /* Odd stripes. */
  WaitBlitter();
  BlitterLineSetup(screen[active], 1, LINE_EOR, LINE_ONEDOT);

  for (k = SIZE - 1, l[0] = &first; k >= 0; k--) {
    WORD xi = (xo & (TILESIZE - 1)) + k * TILESIZE; // + TILESIZE / 2;
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

    BlitterLineSync(l[1]->x1, l[1]->y1, l[1]->x2, l[1]->y2);

    if (((col & 1) == 0) && (l[0]->x2 == WIDTH - 1))
      BlitterLineSync(WIDTH - 1, l[0]->y2, WIDTH - 1, l[1]->y2);

    l[0] = l[1];
  }

  /* Color switching with copper. */
  {
    WORD xi = (xo & (TILESIZE - 1));
    UBYTE **activeLinePos = linePos[active];

    for (k = 0; k < SIZE; k++, xi += TILESIZE) {
      Line2D *l = &vert[xi & (N - 1)];
      CopperLine(activeLinePos[k], l->x1, l->y1, l->x2, l->y2);
    }
  }
}

static void FillStripes() {
  BitmapT *buffer = screen[active];
  WORD n = DEPTH;

  WaitBlitter();
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  while (--n >= 0) {
    UBYTE *bpl = buffer->planes[n] + buffer->bplSize - 1;
    UWORD bltsize = ((buffer->height - FAR_Y) << 6) + (buffer->width >> 4);

    WaitBlitter();
    custom->bltapt = bpl;
    custom->bltdpt = bpl;
    custom->bltsize = bltsize;
  }
}

static void HorizontalStripes(WORD yo) {
  BitmapT *buffer = screen[active];
  WORD yi = yo & (TILESIZE - 1);
  WORD n, k, y1, y2;

  for (k = 0; k <= SIZE; k++, yi += TILESIZE) {
    y1 = horiz[(yi < N) ? yi : N - 1];
    y2 = horiz[(yi + 16 < N) ? yi + 16 : N - 1];

    if (y1 == y2)
      continue;

    WaitBlitter();
    custom->bltadat = 0;
    custom->bltdmod = 0;
    custom->bltcon0 = DEST;
    custom->bltcon1 = 0;

    n = DEPTH;

    while (--n >= 0) {
      APTR bltdpt = buffer->planes[n] + y1 * WIDTH / 8;
      UWORD bltsize = ((y2 - y1) << 6) + (WIDTH >> 4);

      WaitBlitter();
      custom->bltdpt = bltdpt;
      custom->bltsize = bltsize;
    }
  }
}

__regargs static void CalculateTileColumns(WORD yo, WORD kyo) {
  WORD k;
  WORD y0 = FAR_Y;
  WORD yi = (yo & (TILESIZE - 1)) + 16;
  UWORD *tileCol = &tileColumn[y0];

  for (k = 0; k <= SIZE && y0 < HEIGHT; k++, yi += TILESIZE) {
    WORD column = (k + kyo) & (TILES - 1);
    WORD y1 = (yi < N) ? horiz[yi] : HEIGHT;

    for (; y0 < y1; y0++)
      *tileCol++ = column;
  }
}

__regargs static void AssignColorToTiles(WORD kxo) {
  UWORD **activeLineColor = lineColor[active];
  WORD k;

  for (k = 0; k < SIZE; k++) {
    UWORD *color = activeLineColor[k];
    UWORD column = (k + kxo) & (TILES - 1);
    UWORD *textureRow = &texture[column * TILES];
    UWORD *tileCol = &tileColumn[FAR_Y];
    WORD n = HEIGHT - FAR_Y;
    UWORD reg = 2 * (column & 1) + 0x182;

    while (--n >= 0) {
      color[0] = reg;
      color[1] = textureRow[*tileCol++];
      color += STRIDE / sizeof(UWORD);
    }
  }
}

static void ClearLines() {
  UBYTE **activeLinePos = linePos[active];
  WORD k;

  for (k = 0; k < SIZE; k++) {
    UBYTE *pos = activeLinePos[k];
    UBYTE x = (k < 4 ? CPX : (X(WIDTH) >> 1)) | 1;
    WORD n = HEIGHT - FAR_Y;
    while (--n >= 0) {
      *pos = x;
      pos += STRIDE;
    }
  }
}

static void Init() {
  CopListActivate(cp[active]);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG;

  {
    WORD i;

    for (i = 0; i < 2; i++) {
      BlitterLineSetup(screen[i], 0, LINE_EOR, LINE_ONEDOT);
      BlitterLine(WIDTH - 1, 0, WIDTH - 1, FAR_Y - 1);
      WaitBlitter();
      BlitterLine(WIDTH - 1, FAR_Y - 1, WIDTH - 1, HEIGHT);
      WaitBlitter();
      BlitterFill(screen[i], 0);
      WaitBlitter();

      BlitterLineSetup(screen[i], 1, LINE_EOR, LINE_ONEDOT);
      BlitterLine(WIDTH - 1, 0, WIDTH - 1, FAR_Y - 1);
      WaitBlitter();
      BlitterFill(screen[i], 1);
      WaitBlitter();
    }
  }
}

static void Loop() {
  while (!LeftMouseButton()) {
    LONG frameCount = ReadFrameCounter();

    WORD xo = (N / 4) + normfx(SIN(frameCount * 16) * (N / 4));
    WORD yo = (N / 2) + normfx(COS(frameCount * 16) * (N / 2));
    WORD kxo = 7 - xo * SIZE / N;
    WORD kyo = 7 - yo * SIZE / N;

    ClearLines();
    ClearFloor();
    DrawStripes(xo, kxo);
    FillStripes();
    HorizontalStripes(yo);
    CalculateTileColumns(yo, kyo);
    AssignColorToTiles(kxo);

    CopListRun(cp[active]);
    WaitVBlank();
    active ^= 1;
  }
}

EffectT Effect = { Load, Init, Kill, Loop };
