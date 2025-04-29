#include "effect.h"
#include "copper.h"
#include "blitter.h"
#include "sprite.h"
#include "fx.h"
#include "color.h"
#include <stdlib.h>

#define WIDTH   320
#define HEIGHT  256
#define DEPTH   1
#define BGCOL   0x204

#define PFACES  10
#define PWIDTH  64
#define PRISMS  6
#define NCOLORS 4

#include "data/sprite.c"

#define shl12   shift12

typedef struct {
  short angle, radius;
} PolarT;

typedef struct {
  short x, y, z;
} SpanT;

typedef struct {
  PolarT pos;   /* position from (0, 0) in polar coordinates */
  short rotate;  /* value of rotation of prism */
  short color;   /* base color (before shading) of each face */
  short nedges;
  PolarT edges[PFACES];
  SpanT spans[PFACES + 1];
} PrismT;

typedef struct {
  short depth;
  short width;
  short color;
} SpanInfoT;

typedef struct {
  CopInsT color;
  CopInsT bplpri;
  CopInsPairT bplptr;
} CopLineT;

static CopListT *cp[2];
static CopLineT *copLines[2][HEIGHT];
static BitmapT *stripes;
static void *rowAddr[WIDTH / 2];
static SpanInfoT spanInfo[HEIGHT];
static PrismT prisms[PRISMS];
static short active = 0;

static u_short colorSet[NCOLORS] = {
  0xC0F, 0xF0C, 0x80F, 0xF08
};

static u_short colorShades[NCOLORS][32];

static void GeneratePrisms(void) {
  PrismT *prism = prisms;
  int i, j;

  for (i = 0; i < PRISMS; i++, prism++) {
    PolarT *edge = prism->edges;

    short rot = random();
    short ne = 3 + (random() & 7); /* max(ne) = PFACES */

    prism->pos.angle = div16(shl12(i), PRISMS);
    prism->pos.radius = 64;
    prism->rotate = (rot < 0) ? ((rot | 0xffc0) - 32) : ((rot & 0x003f) + 32);
    prism->color = i & 3;
    prism->nedges = ne;
    
    for (j = 0; j < ne; j++, edge++) {
      edge->angle = div16(shl12(j), ne);
      edge->radius = 16;
    }
  }
}

static void GenerateLines(void) {
  EnableDMA(DMAF_BLITTER);

  BlitterLineSetup(stripes, 0, LINE_OR|LINE_ONEDOT);
  BlitterLine(WIDTH / 2 - 1, 0, 0, WIDTH / 2 - 1);
  BlitterLine(WIDTH / 2, 0, WIDTH - 1, WIDTH / 2 - 1);
  BlitterFill(stripes, 0);
  WaitBlitter();

  DisableDMA(DMAF_BLITTER);

  {
    short i;

    for (i = 0; i < WIDTH / 2; i++)
      rowAddr[i] = stripes->planes[0] + i * stripes->bytesPerRow;
  }
}

static void GenerateColorShades(void) {
  short i, j;
  u_short *s = colorSet;
  u_short *d = (u_short *)colorShades;

  for (i = 0; i < NCOLORS; i++) {
    u_short c = *s++;

    for (j = 0; j < 16; j++)
      *d++ = ColorTransition(0x000, c, j);
    for (j = 0; j < 16; j++)
      *d++ = ColorTransition(c, 0xfff, j);
  }
}

static CopListT *MakeCopperList(CopLineT **line) {
  CopListT *cp = NewCopList(HEIGHT * 5 + 200);
  CopInsPairT *sprptr = CopSetupSprites(cp);
  short i;

  for (i = 0; i < HEIGHT; i++) {
    CopWait(cp, Y(i - 1), LASTHP);
    line[i] = (CopLineT *)CopSetColor(cp, 1, 0);
    CopMove16(cp, bplcon2, 0);
    CopMove32(cp, bplpt[0], rowAddr[0]);
  }

  for (i = 0; i < 8; i++) {
    CopInsSetSprite(&sprptr[i], &sprite[i]);
    SpriteUpdatePos(&sprite[i], X(96 + 16 * i), Y((256 - 24) / 2));
  }

  return CopListFinish(cp);
}

static void Init(void) {
  stripes = NewBitmap(WIDTH, WIDTH / 2, 1, BM_CLEAR);

  GeneratePrisms();
  GenerateColorShades();
  GenerateLines();

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  SetColor(0, BGCOL);
  LoadColors(sprite_colors, 16);
  LoadColors(sprite_colors, 20);
  LoadColors(sprite_colors, 24);
  LoadColors(sprite_colors, 28);

  cp[0] = MakeCopperList(copLines[0]);
  cp[1] = MakeCopperList(copLines[1]);

  CopListActivate(cp[0]);
  EnableDMA(DMAF_RASTER | DMAF_SPRITE);
}

static void Kill(void) {
  DeleteBitmap(stripes);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static const short centerY = 128;
static const short centerZ = 192;

static void RotatePrism(PrismT *prism, short rotate) {
  short p_angle = prism->pos.angle + rotate;
  short p_radius = prism->pos.radius;

  /* position of prism center in fx12i format */
  int p_y = COS(p_angle) * p_radius;
  int p_z = SIN(p_angle) * p_radius;

  PolarT *edge = prism->edges;
  SpanT *span = prism->spans;
  short n = prism->nedges;

  while (--n >= 0) {
    short s_angle = edge->angle + prism->rotate;
    short s_radius = edge->radius;

    int x = PWIDTH << 8;
    int y = (COS(s_angle) * s_radius + p_y) >> 4;
    short z = normfx(SIN(s_angle) * s_radius + p_z) + centerZ;

    span->x = div16(x, z);
    span->y = div16(y, z) + centerY;
    span->z = z;

    edge++, span++;
  }

  /* closing span */
  *span = prism->spans[0];
}

static void ClearSpanInfo(void) {
  SpanInfoT *li = spanInfo;
  short n = HEIGHT;
  register short z asm("d1") = 0x7fff;
  short *wp = (short *)li;
  int *lp;

  while (--n >= 0) {
    *wp++ = z; /* depth */
    lp = (int *)wp;
    *lp++ = 0; /* width & color */
    wp = (short *)lp;
  }
}

#define SAFECHECK 0

#if SAFECHECK == 1
#define ONSCREEN(y) (((y) & -256) == 0)
#else
#define ONSCREEN(y) (1)
#endif

static void DrawPrismFaces(PrismT *prism, SpanInfoT *spanInfo) {
  SpanT *span0 = &prism->spans[0];
  SpanT *span1 = &prism->spans[1];
  short ns = prism->nedges;

  while (--ns >= 0) {
    short y0 = span0->y;
    short y1 = span1->y;

    /* skip faces that are turned backwards to the eye */
    if (y0 < y1) {
      SpanInfoT *si = &spanInfo[y0];

      int x0 = span0->x << 16;
      int x1 = span1->x << 16;
      int z0 = span0->z << 16;
      int z1 = span1->z << 16;
      int dx = div16((x1 - x0) >> 8, y1 - y0) << 8;
      int dz = div16((z1 - z0) >> 8, y1 - y0) << 8;

      short color; 
      short shade;

      shade = dz >> 12;

      if (shade < 0)
        shade = -shade;

      shade = 12 - shade;

      if (shade < -12)
        shade = -12;
      if (shade > 12)
        shade = 12;
      
      color = colorShades[prism->color][16 + shade];

      /* draw face */
      while (y0 < y1) {
        /* check if span Y is within [0, 255] */
        if (ONSCREEN(y0)) {
          x0 = swap16(x0);
          z0 = swap16(z0);
          if (si->depth <= (short)z0) {
            si++;
          } else {
            short *wp = (short *)si;
            *wp++ = z0;       /* depth */
            *wp++ = x0;       /* width */
            *wp++ = color;    /* color */
            si = (SpanInfoT *)wp;
          }
          x0 = swap16(x0);
          z0 = swap16(z0);
          // Log("%d: (%d, %d)\n", i, y0, x0 >> 16);
          x0 += dx;
          z0 += dz;
          y0++;
        }
      }
    }

    span0++, span1++;
  }
}


static void DrawVisibleSpans(SpanInfoT *si, CopLineT **lineTab) {
  short n = HEIGHT;

  while (--n >= 0) {
    short *wp = (short *)si;
    short depth = *wp++;
    short width = *wp++;
    short color = *wp++;
    short bplcon2 = (depth > centerZ)
      ? (BPLCON2_PF1P_BOTTOM | BPLCON2_PF2P_BOTTOM)
      : (BPLCON2_PF1P_SP07 | BPLCON2_PF2P_SP07);
    CopLineT *lineIns = *lineTab++;

    CopInsSet16(&lineIns->color, color);
    CopInsSet16(&lineIns->bplpri, bplcon2);
    CopInsSet32(&lineIns->bplptr, (void *)getlong(rowAddr, width));

    si = (SpanInfoT *)wp;
  }
}

static void RenderPrisms(short rotate) {
  PrismT *prism = prisms;
  short n = PRISMS;

  ClearSpanInfo();

  while (--n >= 0) {
    if (n & 1)
      prism->rotate = rotate * 2;
    else
      prism->rotate = -rotate * 2;
    RotatePrism(prism, rotate);
    DrawPrismFaces(prism, spanInfo);
    prism++;
  }

  DrawVisibleSpans(spanInfo, copLines[active]);
}

PROFILE(RenderPrisms);

static void Render(void) {
  ProfilerStop(RenderPrisms); 
  {
    RenderPrisms(frameCount << 3);
  }
  ProfilerStop(RenderPrisms);

  CopListRun(cp[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Prisms, NULL, NULL, Init, Kill, Render, NULL);
