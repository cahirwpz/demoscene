#include "startup.h"
#include "blitter.h"
#include "copper.h"
#include "memory.h"
#include "fx.h"
#include "random.h"
#include "color.h"
#include "pixmap.h"

#define WIDTH 320
#define HEIGHT 212
#define DEPTH 2
#define CPX 0x20

#define N 1024
#define SIZE 8
#define TILES (2 * SIZE)
#define TILESIZE (N / SIZE)

#define STRIDE ((SIZE * 2 + 1) * sizeof(CopInsT))

/* These are selected in such a way so line lenght
 * (HEIGHT - FAR_Y) is divisible by 8. */
#define NEAR_Z 102
#define FAR_Z 318
#define FAR_Y (HEIGHT * NEAR_Z / FAR_Z)
#define FAR_W (WIDTH * FAR_Z / 256)

static BitmapT *screen[2];
static u_short active = 0;
static CopListT *cp[2];

#include "data/floor-city.c"
#include "data/floor.c"

typedef struct {
  int delta;
  short x1, x2, y2;
  char pad[6];
} LineDataT;

static LineDataT vert[N];
static short horiz[N];
static u_char *linePos[2][SIZE];
static u_short *lineColor[2][SIZE];
static u_short tileColumn[HEIGHT];
static u_short tileColor[TILES * TILES];
static u_char cycleStart[TILES * TILES];

static void FloorPrecalc(void) {
  short i;

  for (i = 0; i < N; i++) {
    short x = (i - N / 2) * FAR_W / N;
    short far_x = WIDTH / 2 + (x * 256) / FAR_Z;
    short near_x = WIDTH / 2 + (x * 256) / NEAR_Z;
    short near_y = HEIGHT - 1;
    
    if (near_x < 0) {
      near_y = FAR_Y + (0 - far_x) * ((HEIGHT - 1) - FAR_Y) / (near_x - far_x);
      near_x = 0;
    }
    if (near_x > WIDTH - 1) {
      near_y = FAR_Y +  ((WIDTH - 1) - far_x) * ((HEIGHT - 1) - FAR_Y) / (near_x - far_x);
      near_x = WIDTH - 1;
    }
    
    vert[i].x1 = far_x;
    vert[i].x2 = near_x;
    vert[i].y2 = near_y;

    if (near_y != FAR_Y)
      vert[i].delta = ((X(near_x) / 2 - X(far_x) / 2) << 16) / (near_y - FAR_Y);
  }

  for (i = 0; i < N; i++) {
    short z = FAR_Z + ((NEAR_Z - FAR_Z) * i) / N;
    
    horiz[i] = HEIGHT * NEAR_Z / z;
  }
}

static void Load(void) {
  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  FloorPrecalc();

  ITER(i, 0, 255, cycleStart[i] = random() & 63);
}

static void UnLoad(void) {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static void MakeCopperList(CopListT *cp, short num) {
  CopInsT *ins;
  short i, j;

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen[num], DEPTH);
  CopLoadColor(cp, 0, 3, 0);

  for (i = 0; i < FAR_Y; i++) {
    short s = i * 15 / FAR_Y;

    CopWait(cp, Y(i), 0);
    CopSetColor(cp, 3, ColorTransition(0, 0xADF, s));
  }

  for (i = FAR_Y; i < HEIGHT; i++) {
    CopWait(cp, Y(i), CPX);
    for (j = 0; j < SIZE; j++) {
      ins = CopWait(cp, Y(i), CPX);
      if (i == FAR_Y)
        linePos[num][j] = ((u_char*)ins) + 1;
      ins = CopSetColor(cp, (j & 1) + 1, (j & 1) ? 0xff0 : 0x0ff);
      if (i == FAR_Y)
        lineColor[num][j] = ((u_short*)ins);
    }
  }

  CopWait(cp, Y(i) & 255, 0);
  CopLoadColor(cp, 0, 3, 0);
  CopEnd(cp);
}

static void Init(void) {
  short i;

  EnableDMA(DMAF_BLITTER);

  for (i = 0; i < 2; i++) {
    Area2D top = { 0, 0, WIDTH, 36 };
    Area2D bottom = { 0, 0, WIDTH, FAR_Y };
    BitmapClear(screen[i]);
    BlitterSetArea(screen[i], 0, &top, -1);
    BitmapCopy(screen[i], 0, 36, &city);
    BlitterSetArea(screen[i], 1, &bottom, -1);
  }

  cp[0] = NewCopList((HEIGHT - FAR_Y) * STRIDE / sizeof(CopInsT) + 300);
  cp[1] = NewCopList((HEIGHT - FAR_Y) * STRIDE / sizeof(CopInsT) + 300);

  ITER(j, 0, 1, MakeCopperList(cp[j], j));
  CopListActivate(cp[active]);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER | DMAF_BLITTER);

  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static __regargs void ClearLine(short k) {
  u_char *pos = linePos[active][k];
  u_char x = (k < 4 ? CPX : (X(WIDTH) >> 1)) | 1;
  short n = (HEIGHT - FAR_Y) / 8;

  while (--n >= 0) {
    pos[STRIDE * 0] = x;
    pos[STRIDE * 1] = x;
    pos[STRIDE * 2] = x;
    pos[STRIDE * 3] = x;
    pos[STRIDE * 4] = x;
    pos[STRIDE * 5] = x;
    pos[STRIDE * 6] = x;
    pos[STRIDE * 7] = x;
    pos += STRIDE * 8;
  }
}

static void ClearFloor(void) {
  BitmapT *buffer = screen[active];
  u_short bltsize = ((buffer->height - FAR_Y) << 6) + (buffer->width >> 4);
  void *bltpt = buffer->planes[0] + FAR_Y * WIDTH / 8;
  short i = SIZE / 2;

  WaitBlitter();
  custom->bltadat = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = DEST;
  custom->bltcon1 = 0;
  custom->bltdpt = bltpt;
  custom->bltsize = bltsize;

  bltpt += buffer->bplSize;
  while (i < SIZE)
    ClearLine(i++);

  WaitBlitter();
  custom->bltdpt = bltpt;
  custom->bltsize = bltsize;
}

static inline void CopperLine(u_char *pos, short x1, short y2, int delta) {
  if (y2 > FAR_Y) {
    short n = y2 - FAR_Y + 1;
    int x = (X(x1) / 2) << 16;
    register u_char one asm("d7") = 1;

    while (--n >= 0) {
      *pos = swap16(x) | one;
      pos += STRIDE;
      x += delta;
    }
  }
}

static __regargs void DrawStripesCopper(short xo) {
  /* Color switching with copper. */
  short xi = (xo & (TILESIZE - 1));
  u_char **activeLinePos = linePos[active];
  short n = SIZE;

  while (--n >= 0) {
    LineDataT *l = &vert[xi & (N - 1)];
    CopperLine(*activeLinePos++, l->x1, l->y2, l->delta);
    xi += TILESIZE;
  }
}

static __regargs void DrawStripes(short xo, short kxo) {
  LineDataT first = { 0, 0, WIDTH - 1, FAR_Y, {0} };
  LineDataT *l0, *l1;
  short k;

  xo &= TILESIZE - 1;

  /* Even stripes. */
  BlitterLineSetup(screen[active], 0, LINE_EOR|LINE_ONEDOT);

  l0 = &first;
  k = SIZE;

  while (--k >= 0) {
    short xi = xo + k * TILESIZE;
    short col = kxo + k;

    if (col & 1) {
      xi += 8;
      if (xi > N - 1)
        xi = N - 1;
    } else {
      xi -= 8;
      if (xi < 0)
        xi = 0;
    }

    l1 = &vert[xi & (N - 1)];

    BlitterLine(l1->x1, FAR_Y, l1->x2, l1->y2);

    if ((l0->x2 == WIDTH - 1) && (col & 1))
      BlitterLine(WIDTH - 1, l0->y2, WIDTH - 1, l1->y2);

    l0 = l1;
  }

  /* Odd stripes. */
  WaitBlitter();
  BlitterLineSetup(screen[active], 1, LINE_EOR|LINE_ONEDOT);

  l0 = &first;
  k = SIZE;

  while (--k >= 0) {
    short xi = xo + k * TILESIZE;
    short col = kxo + k;

    if (col & 1) {
      xi -= 8;
      if (xi < 0)
        xi = 0;
    } else {
      xi += 8;
      if (xi > N - 1)
        xi = N - 1;
    }

    l1 = &vert[xi & (N - 1)];

    BlitterLine(l1->x1, FAR_Y, l1->x2, l1->y2);

    if ((l0->x2 == WIDTH - 1) && (col & 1) == 0)
      BlitterLine(WIDTH - 1, l0->y2, WIDTH - 1, l1->y2);

    l0 = l1;
  }
}

__regargs static void AssignColorToTileColumn(short k, short kxo);

static void FillStripes(short kxo) {
  BitmapT *buffer = screen[active];
  u_short bltsize = ((buffer->height - FAR_Y) << 6) + (buffer->width >> 4);
  void *bltpt = buffer->planes[0] + buffer->bplSize - 1;

  WaitBlitter();
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltapt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltsize = bltsize;

  bltpt += buffer->bplSize;

  {
    short i = 0;

    while (i < SIZE)
      AssignColorToTileColumn(i++, kxo);
  }

  WaitBlitter();
  custom->bltapt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltsize = bltsize;
}

static void HorizontalStripes(short yo) {
  BitmapT *buffer = screen[active];
  short yi = yo & (TILESIZE - 1);
  short k, y1, y2;

  for (k = 0; k <= SIZE; k++, yi += TILESIZE) {
    y1 = horiz[(yi < N) ? yi : N - 1];
    y2 = horiz[(yi + 16 < N) ? yi + 16 : N - 1];

    if (y1 != y2) {
      u_short bltsize = ((y2 - y1) << 6) | (WIDTH >> 4);
      void *bltpt = buffer->planes[0] + y1 * WIDTH / 8;

      WaitBlitter();
      custom->bltadat = 0;
      custom->bltdmod = 0;
      custom->bltcon0 = DEST;
      custom->bltcon1 = 0;
      custom->bltdpt = bltpt;
      custom->bltsize = bltsize;

      bltpt += buffer->bplSize;

      WaitBlitter();
      custom->bltdpt = bltpt;
      custom->bltsize = bltsize;
    }
  }
}

__regargs static void CalculateTileColumns(short yo, short kyo) {
  short k;
  short y0 = FAR_Y;
  short yi = (yo & (TILESIZE - 1)) + 16;
  u_short *tileCol = &tileColumn[y0];

  for (k = 0; k <= SIZE && y0 < HEIGHT; k++, yi += TILESIZE) {
    short column = ((k + kyo) & (TILES - 1)) * 2;
    short y1 = (yi < N) ? horiz[yi] : HEIGHT;
    short n = y1 - y0;

    while (--n >= 0) 
      *tileCol++ = column;

    y0 = y1;
  }
}

#define STRIDE2 (STRIDE / sizeof(short))

__regargs static void AssignColorToTileColumn(short k, short kxo) {
  u_short *color = lineColor[active][k];
  u_short column = (k + kxo) & (TILES - 1);
  void *textureRow = &tileColor[column * TILES];
  u_short *tileCol = &tileColumn[FAR_Y];
  short n = (HEIGHT - FAR_Y) >> 3;
  u_short reg = 2 * (column & 1) + 0x182;
  short c;

  while (--n >= 0) {
    color[0 + STRIDE2 * 0] = reg; c = *tileCol++;
    color[1 + STRIDE2 * 0] = *(u_short *)(textureRow + c);
    color[0 + STRIDE2 * 1] = reg; c = *tileCol++;
    color[1 + STRIDE2 * 1] = *(u_short *)(textureRow + c);
    color[0 + STRIDE2 * 2] = reg; c = *tileCol++;
    color[1 + STRIDE2 * 2] = *(u_short *)(textureRow + c);
    color[0 + STRIDE2 * 3] = reg; c = *tileCol++;
    color[1 + STRIDE2 * 3] = *(u_short *)(textureRow + c);
    color[0 + STRIDE2 * 4] = reg; c = *tileCol++;
    color[1 + STRIDE2 * 4] = *(u_short *)(textureRow + c);
    color[0 + STRIDE2 * 5] = reg; c = *tileCol++;
    color[1 + STRIDE2 * 5] = *(u_short *)(textureRow + c);
    color[0 + STRIDE2 * 6] = reg; c = *tileCol++;
    color[1 + STRIDE2 * 6] = *(u_short *)(textureRow + c);
    color[0 + STRIDE2 * 7] = reg; c = *tileCol++;
    color[1 + STRIDE2 * 7] = *(u_short *)(textureRow + c);
    color += 8 * STRIDE2;
  }
}

static void ControlTileColors(void) {
  u_short *src = texture.pixels, *dst = tileColor;
  u_char *cycle = cycleStart;
  short n = TILES * TILES;

  while (--n >= 0) {
    u_short f = (frameCount / 2 + *cycle++) & 63;
    u_short c = *src++;
    short r, g, b;

    if (f < 16) {
      r = ((c >> 4) & 0x0f0) | f;
      g = (c & 0x0f0) | f;
      b = ((c << 4) & 0x0f0) | f;
    } else if (f < 32) {
      r = (c & 0xf00) | 0x0f0 | (f - 16);
      g = ((c << 4) & 0xf00) | 0x0f0 | (f - 16);
      b = ((c << 8) & 0xf00) | 0x0f0 | (f - 16);
    } else if (f < 48) {
      r = 0xf00 | ((c >> 4) & 0x0f0) | (f - 32);
      g = 0xf00 | (c & 0x0f0) | (f - 32);
      b = 0xf00 | ((c << 4) & 0x0f0) | (f - 32);
    } else {
      r = (c & 0xf00) | (f - 48);
      g = ((c << 4) & 0xf00) | (f - 48);
      b = ((c << 8) & 0xf00) | (f - 48);
    }

    *dst++ = (colortab[r] << 4) | colortab[g] | (colortab[b] >> 4);
  }
}

static void Render(void) {
  // int lines = ReadLineCounter();

  ControlTileColors();
  {
    short xo = (N / 4) + normfx(SIN(frameCount * 16) * (N / 4));
    short yo = (N / 4) + normfx(COS(frameCount * 16) * (N / 4));
    short kxo = 7 - xo * SIZE / N;
    short kyo = 7 - yo * SIZE / N;

    ClearFloor();
    DrawStripesCopper(xo);
    DrawStripes(xo, kxo);
    CalculateTileColumns(yo, kyo);
    HorizontalStripes(yo);
    FillStripes(kxo);
  }

  // Log("floor: %d\n", ReadLineCounter() - lines);

  CopListRun(cp[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
