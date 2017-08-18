#include "startup.h"
#include "hardware.h"
#include "coplist.h"
#include "ilbm.h"
#include "blitter.h"
#include "sprite.h"
#include "fx.h"
#include "random.h"
#include "color.h"

STRPTR __cwdpath = "data";

#define WIDTH   320
#define HEIGHT  256
#define DEPTH   1
#define BGCOL   0x204

#define PFACES  10
#define PWIDTH  64
#define PRISMS  6

#define shl12   shift12

typedef struct {
  WORD angle, radius;
} PolarT;

typedef struct {
  WORD x, y, z;
} SpanT;

typedef struct {
  PolarT pos;   /* position from (0, 0) in polar coordinates */
  WORD rotate;  /* value of rotation of prism */
  WORD color;   /* base color (before shading) of each face */
  WORD nedges;
  PolarT edges[PFACES];
  SpanT spans[PFACES + 1];
} PrismT;

typedef struct {
  WORD depth;
  WORD width;
  WORD color;
} SpanInfoT;

static CopListT *cp[2];
static CopInsT *clines[2][HEIGHT];
static BitmapT *stripes;
static APTR rowAddr[WIDTH / 2];
static SpanInfoT spanInfo[HEIGHT];
static PrismT prisms[PRISMS];
static WORD active = 0;

static CopInsT *sprptr[8];
static SpriteT *sprite[8];
static PaletteT *spritePal;

static UWORD colorSet[4] = { 0xC0F, 0xF0C, 0x80F, 0xF08 };
static UWORD colorShades[4][32];

static void Load() {
  BitmapT *bm = LoadILBM("sprite.ilbm");
  ITER(i, 0, 7, sprite[i] = NewSpriteFromBitmap(24, bm, 16 * i, 0));
  spritePal = bm->palette;
  DeleteBitmap(bm);
}

static void UnLoad() {
  ITER(i, 0, 7, DeleteSprite(sprite[i]));
  DeletePalette(spritePal);
}

static void GeneratePrisms() {
  PrismT *prism = prisms;
  LONG i, j;

  for (i = 0; i < PRISMS; i++, prism++) {
    PolarT *edge = prism->edges;

    WORD rot = random();
    WORD ne = 3 + (random() & 7); /* max(ne) = PFACES */

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

static void GenerateLines() {
  EnableDMA(DMAF_BLITTER);

  BlitterLineSetup(stripes, 0, LINE_OR|LINE_ONEDOT);
  BlitterLine(WIDTH / 2 - 1, 0, 0, WIDTH / 2 - 1);
  BlitterLine(WIDTH / 2, 0, WIDTH - 1, WIDTH / 2 - 1);
  BlitterFill(stripes, 0);
  WaitBlitter();

  DisableDMA(DMAF_BLITTER);

  {
    WORD i;

    for (i = 0; i < WIDTH / 2; i++)
      rowAddr[i] = stripes->planes[0] + i * stripes->bytesPerRow;
  }
}

static void GenerateColorShades() {
  WORD i, j;
  UWORD *s = colorSet;
  UWORD *d = (UWORD *)colorShades;

  for (i = 0; i < 4; i++) {
    UWORD c = *s++;

    for (j = 0; j < 16; j++)
      *d++ = ColorTransition(0x000, c, j);
    for (j = 0; j < 16; j++)
      *d++ = ColorTransition(c, 0xfff, j);
  }
}

static void MakeCopperList(CopListT *cp, CopInsT **cline) {
  WORD i;

  CopInit(cp);
  CopSetupSprites(cp, sprptr);
  CopLoadPal(cp, spritePal, 16);
  CopLoadPal(cp, spritePal, 20);
  CopLoadPal(cp, spritePal, 24);
  CopLoadPal(cp, spritePal, 28);

  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  CopSetRGB(cp, 0, BGCOL);

  for (i = 0; i < HEIGHT; i++) {
    CopWait(cp, Y(i - 1), 0xDE);
    cline[i] = CopSetRGB(cp, 1, 0);
    CopMove16(cp, bplcon2, 0);
    CopMove32(cp, bplpt[0], rowAddr[0]);
  }

  CopEnd(cp);

  ITER(i, 0, 7, CopInsSet32(sprptr[i], sprite[i]->data));
}

static void Init() {
  stripes = NewBitmap(WIDTH, WIDTH / 2, 1);

  GeneratePrisms();
  GenerateColorShades();
  GenerateLines();

  cp[0] = NewCopList(HEIGHT * 5 + 200);
  cp[1] = NewCopList(HEIGHT * 5 + 200);

  MakeCopperList(cp[0], clines[0]);
  MakeCopperList(cp[1], clines[1]);

  ITER(i, 0, 7, UpdateSprite(sprite[i], X(96 + 16 * i), Y((256 - 24) / 2)));

  CopListActivate(cp[0]);
  EnableDMA(DMAF_RASTER | DMAF_SPRITE);
}

static void Kill() {
  DeleteBitmap(stripes);
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static const WORD centerY = 128;
static const WORD centerZ = 192;

static void RotatePrism(PrismT *prism, WORD rotate) {
  WORD p_angle = prism->pos.angle + rotate;
  WORD p_radius = prism->pos.radius;

  /* position of prism center in fx12i format */
  LONG p_y = COS(p_angle) * p_radius;
  LONG p_z = SIN(p_angle) * p_radius;

  PolarT *edge = prism->edges;
  SpanT *span = prism->spans;
  WORD n = prism->nedges;

  while (--n >= 0) {
    WORD s_angle = edge->angle + prism->rotate;
    WORD s_radius = edge->radius;

    LONG x = PWIDTH << 8;
    LONG y = (COS(s_angle) * s_radius + p_y) >> 4;
    WORD z = normfx(SIN(s_angle) * s_radius + p_z) + centerZ;

    span->x = div16(x, z);
    span->y = div16(y, z) + centerY;
    span->z = z;

    edge++, span++;
  }

  /* closing span */
  *span = prism->spans[0];
}

static void ClearSpanInfo() {
  SpanInfoT *li = spanInfo;
  WORD n = HEIGHT;
  register WORD z asm("d1") = 0x7fff;

  while (--n >= 0) {
    *((WORD *)li)++ = z; /* depth */
    *((LONG *)li)++ = 0; /* width & color */
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
  WORD ns = prism->nedges;

  while (--ns >= 0) {
    WORD y0 = span0->y;
    WORD y1 = span1->y;

    /* skip faces that are turned backwards to the eye */
    if (y0 < y1) {
      SpanInfoT *si = &spanInfo[y0];

      LONG x0 = span0->x << 16;
      LONG x1 = span1->x << 16;
      LONG z0 = span0->z << 16;
      LONG z1 = span1->z << 16;
      LONG dx = div16((x1 - x0) >> 8, y1 - y0) << 8;
      LONG dz = div16((z1 - z0) >> 8, y1 - y0) << 8;

      WORD color; 
      WORD shade;

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
          if (si->depth <= (WORD)z0) {
            si++;
          } else {
            *((WORD *)si)++ = z0;       /* depth */
            *((WORD *)si)++ = x0;       /* width */
            *((WORD *)si)++ = color;    /* color */
          }
          x0 = swap16(x0);
          z0 = swap16(z0);
          /* Log("%ld: (%ld, %ld)\n", (LONG)i, (LONG)y0, (LONG)(x0 >> 16)); */
          x0 += dx;
          z0 += dz;
          y0++;
        }
      }
    }

    span0++, span1++;
  }
}


static void DrawVisibleSpans(SpanInfoT *si, CopInsT **cline) {
  WORD n = HEIGHT;

  while (--n >= 0) {
    WORD depth = *((WORD *)si)++;
    WORD width = *((WORD *)si)++;
    WORD color = *((WORD *)si)++;
    WORD bplcon2 = (depth > centerZ) ? BPLCON2_PF1P2|BPLCON2_PF2P2 : 0;
    CopInsT *ins = *cline++;

    CopInsSet16(ins, color);
    CopInsSet16(ins + 1, bplcon2);
    CopInsSet32(ins + 2, (APTR)getlong(rowAddr, width));
  }
}

static void RenderPrisms(WORD rotate) {
  PrismT *prism = prisms;
  WORD n = PRISMS;

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

  DrawVisibleSpans(spanInfo, clines[active]);
}

static void Render() {
  LONG start = ReadLineCounter();
  RenderPrisms(frameCount << 3);
  Log("prisms: %ld\n", ReadLineCounter() - start);

  CopListActivate(cp[active]);
  WaitVBlank();
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
