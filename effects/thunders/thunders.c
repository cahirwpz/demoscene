#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "memory.h"
#include "fx.h"
#include "random.h"
#include "color.h"
#include "pixmap.h"
#include "sprite.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 3
#define BGCOL 0x012

#define N 1024
#define SIZE 8
#define TILESIZE (N / SIZE)
#define GAP 24

/* These are selected in such a way so line lenght
 * (HEIGHT - FAR_Y) is divisible by 8. */
#define NEAR_Z (256 + 32)
#define FAR_Z 512
#define FAR_Y (HEIGHT * NEAR_Z / FAR_Z)
#define FAR_W (WIDTH * FAR_Z / 256)

static BitmapT *screen0, *screen1;
static CopListT *cp0, *cp1;
static u_short tileColor[SIZE * SIZE];
static short tileCycle[SIZE * SIZE];
static short tileEnergy[SIZE * SIZE];

#include "data/thunders.c"
#include "data/thunders-floor.c"

typedef struct {
  short x1, x2, y2;
  short pad;
} LineDataT;

static LineDataT vert[N];
static short horiz[N];

extern void InitColorTab(void);

static void FloorPrecalc(void) {
  short i;

  for (i = 0; i < N; i++) {
    short x = (i - N / 2) * FAR_W / N;
    short far_x = WIDTH / 2 + (x * 256) / FAR_Z;
    short near_x = WIDTH / 2 + (x * 640) / NEAR_Z;
    short near_y = HEIGHT - 1;
    
    if (near_x < 0) {
      near_y = FAR_Y + (0 - far_x) * ((HEIGHT - 1) - FAR_Y) / (near_x - far_x);
      near_x = 0;
    }
    if (near_x > WIDTH - 1) {
      near_y = FAR_Y + ((WIDTH - 1) - far_x) * ((HEIGHT - 1) - FAR_Y) / (near_x - far_x);
      near_x = WIDTH - 1;
    }
    
    vert[i].x1 = far_x;
    vert[i].x2 = near_x;
    vert[i].y2 = near_y;
  }

  for (i = 0; i < N; i++) {
    short z = FAR_Z + ((NEAR_Z - FAR_Z) * i) / N;
    
    horiz[i] = HEIGHT * NEAR_Z / z;
  }

  InitColorTab();
}

static void Load(void) {
  short i;

  for (i = 0; i < 320 / 16; i++) {
    short xo = X((WIDTH - 32) / 2) + (i & 1 ? 16 : 0);
    short yo = Y((HEIGHT - 128) / 2);

    UpdateSprite(thunder[i], xo, yo);
  }

  FloorPrecalc();

  ITER(i, 0, SIZE * SIZE - 1, tileCycle[i] = random() & SIN_MASK);
}

static void MakeCopperList(CopListT *cp, BitmapT *screen) {
  short j;

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, NULL, screen, DEPTH);
  CopSetupSprites(cp, NULL);
  for (j = 0; j < 8; j++)
    CopSetColor(cp, j, BGCOL);
  CopEnd(cp);
}

static void Init(void) {
  screen0 = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen1 = NewBitmap(WIDTH, HEIGHT, DEPTH);

  cp0 = NewCopList((HEIGHT - FAR_Y) * 16 + 200);
  cp1 = NewCopList((HEIGHT - FAR_Y) * 16 + 200);

  MakeCopperList(cp0, screen0);
  MakeCopperList(cp1, screen1);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  CopListActivate(cp1);
  EnableDMA(DMAF_RASTER | DMAF_SPRITE);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_SPRITE | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp0);
  DeleteCopList(cp1);

  DeleteBitmap(screen0);
  DeleteBitmap(screen1);
}

static void DrawLine(void *data asm("a2"), short x1 asm("d2"), short y1 asm("d3"), short x2 asm("d4"), short y2 asm("d5")) {
  u_short bltcon1 = LINEMODE | ONEDOT;
  void *start = data;
  short dx, dy, derr;

  /* Always draw the line downwards. */
  if (y1 > y2) {
    swapr(x1, x2);
    swapr(y1, y2);
  }

  /* Word containing the first pixel of the line. */
  start += y1 * (WIDTH / 8);
  start += (x1 >> 3) & ~1;

  dx = x2 - x1;
  dy = y2 - y1;

  if (dx < 0) {
    dx = -dx;
    if (dx >= dy) {
      bltcon1 |= AUL | SUD;
    } else {
      bltcon1 |= SUL;
      swapr(dx, dy);
    }
  } else {
    if (dx >= dy) {
      bltcon1 |= SUD;
    } else {
      swapr(dx, dy);
    }
  }

  derr = dy + dy - dx;
  if (derr < 0)
    bltcon1 |= SIGNFLAG;

  {
    u_short bltcon0 = rorw(x1 & 15, 4) | BC0F_LINE_EOR;
    u_short bltamod = derr - dx;
    u_short bltbmod = dy + dy;
    u_short bltsize = (dx << 6) + 66;

    WaitBlitter();

    custom->bltcon0 = bltcon0;
    custom->bltcon1 = bltcon1;
    custom->bltamod = bltamod;
    custom->bltbmod = bltbmod;
    custom->bltapt = (void *)(int)derr;
    custom->bltcpt = start;
    custom->bltdpt = data;
    custom->bltsize = bltsize;
  }
}

static __regargs void DrawStripes(short xo, short kxo) {
  short k;

  /* Setup fast line drawing. */
  WaitBlitter();
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff;
  custom->bltcmod = WIDTH / 8;
  custom->bltdmod = WIDTH / 8;

  /* Draw wide stripes. */
  xo &= TILESIZE - 1;

  k = SIZE + 1;

  xo += (k - 1) * TILESIZE;
  kxo += k - 1;

  /* Draw from right to left. */ 
  for (; --k >= 0; xo -= TILESIZE, kxo--) {
    LineDataT *left, *right;

    {
      short xi = xo + 8 - TILESIZE;
      if (xi < 0)
        xi = 0;
      left = &vert[xi & (N - 1)];
    }

    {
      short xi = xo - 8;
      if (xi > N - 1)
        xi = N - 1;
      right = &vert[xi & (N - 1)];
    }
    
    {
      short i = DEPTH;
      short c = mod16(kxo, 7) + 1;

      while (--i >= 0) {
        void *plane = screen0->planes[i];

        if (c & (1 << i)) {
          DrawLine(plane, left->x1, FAR_Y, left->x2, left->y2);
          DrawLine(plane, right->x1, FAR_Y, right->x2, right->y2);

          if (right->x2 == WIDTH - 1)
            DrawLine(plane, WIDTH - 1, left->y2, WIDTH - 1, right->y2);
        }
      }
    }
  }
}

static __regargs void FillStripes(u_short plane) {
  void *bltpt = screen0->planes[plane] + (HEIGHT * WIDTH) / 8 - 2;
  u_short bltsize = ((HEIGHT - FAR_Y - 1) << 6) | (WIDTH >> 4);

  WaitBlitter();

  custom->bltapt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_OR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = bltsize;
}

void ControlTileColors(void) {
  u_short *src = texture.pixels, *dst = tileColor;
  short *energy = tileEnergy;
  short *cycle = tileCycle;
  short n = SIZE * SIZE ;

  while (--n >= 0) {
    short f = (SIN(frameCount * 16 + *cycle++) >> 10) - 4;
    short e = *energy;
    u_short c = *src++;
    short r, g, b;

    if (e > 0)
      *energy = e - 1;
    energy++;

    f += e;
    if (f > 15)
      f = 15;

    if (f < 0) {
      f = 16 + f;
      /* 000 <-> RGB */
      r = ((c >> 4) & 0x0f0) | f;
      g = (c & 0x0f0) | f;
      b = ((c << 4) & 0x0f0) | f;
    } else {
      /* RGB -> FFF */
      r = (c & 0xf00) | 0x0f0 | f;
      g = ((c << 4) & 0xf00) | 0x0f0 | f;
      b = ((c << 8) & 0xf00) | 0x0f0 | f;
    }

    *dst++ = (colortab[r] << 4) | (u_char)(colortab[g] | (colortab[b] >> 4));
  }
}

static __regargs void ColorizeUpperHalf(CopListT *cp, short yi, short kyo) {
  short k;
  short y0 = HEIGHT;
  void *pixels = tileColor;

  yi += (SIZE - 1) * TILESIZE - GAP;

  for (k = SIZE; k >= 0; k--, yi -= TILESIZE) {
    short column = ((k + kyo) & (SIZE - 1));
    u_short *colors = pixels + (column * SIZE + 1) * sizeof(u_short);

    CopWait(cp, Y(HEIGHT - y0), 0);
    CopSetColor(cp, 1, *colors++);
    CopSetColor(cp, 2, *colors++);
    CopSetColor(cp, 3, *colors++);
    CopSetColor(cp, 4, *colors++);
    CopSetColor(cp, 5, *colors++);
    CopSetColor(cp, 6, *colors++);
    CopSetColor(cp, 7, *colors++);

    y0 = (yi > 0) ? horiz[yi] : FAR_Y;

    {
      short yj = yi + GAP;
      short y1;

      if (yj < 0)
        yj = 0;
      y1 = horiz[yj];

      CopWait(cp, Y(HEIGHT - y1), 0);
      CopSetColor(cp, 1, BGCOL);
      CopSetColor(cp, 2, BGCOL);
      CopSetColor(cp, 3, BGCOL);
      CopSetColor(cp, 4, BGCOL);
      CopSetColor(cp, 5, BGCOL);
      CopSetColor(cp, 6, BGCOL);
      CopSetColor(cp, 7, BGCOL);
    }
  }
}

static __regargs void ColorizeLowerHalf(CopListT *cp, short yi, short kyo) {
  short k;
  short y0 = FAR_Y;
  void *pixels = tileColor;

  for (k = 0; k <= SIZE; k++, yi += TILESIZE) {
    short column = ((k + kyo) & (SIZE - 1));
    u_short *colors = pixels + (column * SIZE + 1) * sizeof(u_short);

    CopWaitSafe(cp, Y(y0), 0);
    CopSetColor(cp, 1, *colors++);
    CopSetColor(cp, 2, *colors++);
    CopSetColor(cp, 3, *colors++);
    CopSetColor(cp, 4, *colors++);
    CopSetColor(cp, 5, *colors++);
    CopSetColor(cp, 6, *colors++);
    CopSetColor(cp, 7, *colors++);

    {
      short yj = yi - GAP;
      short y1;

      if (yj < 0)
        yj = 0;
      y1 = horiz[yj];

      CopWaitSafe(cp, Y(y1), 0);
      CopSetColor(cp, 1, BGCOL);
      CopSetColor(cp, 2, BGCOL);
      CopSetColor(cp, 3, BGCOL);
      CopSetColor(cp, 4, BGCOL);
      CopSetColor(cp, 5, BGCOL);
      CopSetColor(cp, 6, BGCOL);
      CopSetColor(cp, 7, BGCOL);
    }

    y0 = (yi < N) ? horiz[yi] : HEIGHT;
  }
}

static void MakeFloorCopperList(short yo, short kyo) {
  CopListT *cp = cp0;

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  {
    void **planes = screen0->planes;
    CopMove32(cp, bplpt[0], (*planes++) + WIDTH * (HEIGHT - 1) / 8);
    CopMove32(cp, bplpt[1], (*planes++) + WIDTH * (HEIGHT - 1) / 8);
    CopMove32(cp, bplpt[2], (*planes++) + WIDTH * (HEIGHT - 1) / 8);
  }
  CopMove16(cp, bpl1mod, - (WIDTH * 2) / 8);
  CopMove16(cp, bpl2mod, - (WIDTH * 2) / 8);

  CopSetupSprites(cp, NULL);
 
  {
    short i = mod16(frameCount, 10) * 2;
    u_short *thunder0 = thunder[i]->data;
    u_short *thunder1 = thunder[i+1]->data;

    CopMove32(cp, sprpt[0], thunder0);
    CopMove32(cp, sprpt[1], thunder1);
    CopMove32(cp, sprpt[2], NullSprite);
    CopMove32(cp, sprpt[3], NullSprite);
    CopMove32(cp, sprpt[4], NullSprite);
    CopMove32(cp, sprpt[5], NullSprite);
    CopMove32(cp, sprpt[6], NullSprite);
    CopMove32(cp, sprpt[7], NullSprite);
  }

  /* Clear out the colors. */
  CopSetColor(cp, 0, BGCOL);
  CopLoadPal(cp, &thunder_pal, 16);

  FillStripes(1);
  ColorizeUpperHalf(cp, yo, kyo);

  CopWaitV(cp, Y(HEIGHT / 2 - 1));
  CopMove16(cp, bpl1mod, 0);
  CopMove16(cp, bpl2mod, 0);

  FillStripes(2);
  ColorizeLowerHalf(cp, yo, kyo);

  CopEnd(cp);
}

static void Render(void) {
  // PROFILE_BEGIN(floor);

  BitmapClearArea(screen0, STRUCT(Area2D, 0, FAR_Y, WIDTH, HEIGHT - FAR_Y));

  {
    short xo = (N / 4) + normfx(SIN(frameCount * 16) * N * 15 / 64);
    short yo = (N / 4) + normfx(COS(frameCount * 16) * N / 4);
    short kxo = 7 - xo * SIZE / N;
    short kyo = 7 - yo * SIZE / N;

    {
      short cyo = 7 - (yo + SIZE * 4) * SIZE / N;
      tileEnergy[((cyo - 3) & 7) * 8 + ((kxo - 2) & 7)] = 32;
    }

    DrawStripes(xo, kxo);
    FillStripes(0);
    ControlTileColors();
    MakeFloorCopperList(yo & (TILESIZE - 1), kyo);
  }

  // PROFILE_END(floor);

  CopListRun(cp0);
  TaskWaitVBlank();
  { CopListT *tmp = cp0; cp0 = cp1; cp1 = tmp; }
  { BitmapT *tmp = screen0; screen0 = screen1; screen1 = tmp; }
}

EFFECT(thunders, Load, NULL, Init, Kill, Render);
