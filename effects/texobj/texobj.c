#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <3d.h>
#include <fx.h>
#include <system/interrupt.h>
#include <system/memory.h>

#define WIDTH 128
#define HEIGHT 128
#define DEPTH 4

#include "data/cube.c"
#include "data/cube-bg.c"
#include "data/cube-tex0.c"
#include "data/cube-tex1.c"
#include "data/cube-tex2.c"
#include "data/cube-tex3.c"
#include "data/cube-tex4.c"

static __code BitmapT *screen[2];
static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code u_char *chunky;
static __code short active;
static __code volatile short c2p_phase;
static __code void **c2p_bpl;
static __code Object3D *object;
static __code short *texture[5];

/* [0 0 0 0 a0 a1 a2 a3] => [a0 a1 0 0 a2 a3 0 0] */
static const short Pixel[16] = {
  0x0000, 0x0404, 0x0808, 0x0c0c, 0x4040, 0x4444, 0x4848, 0x4c4c,
  0x8080, 0x8484, 0x8888, 0x8c8c, 0xc0c0, 0xc4c4, 0xc8c8, 0xcccc,
};

static const short texture_light[16] = {
  4, 4, 4, 4,
  3, 3, 3,
  2, 2, 2,
  1, 1, 1,
  0, 0, 0,
};

static void ScrambleBackground(void) {
  u_char *src = background_pixels;
  u_char *dst = background_pixels;
  short n = background_width * background_height;

  while (--n >= 0) {
    *dst++ = Pixel[*src++];
  }
}

static short *ScrambleTexture(u_char *src) {
  short *tex = MemAlloc(texture_0_width * texture_0_height * 2, MEMF_PUBLIC);
  short n = texture_0_width * texture_0_height;
  short *dst = tex;

  while (--n >= 0) {
    *dst++ = Pixel[*src++];
  }

  return tex;
}

typedef struct Corner {
  short x, y;
  short u, v;
  short yi;
} CornerT;

typedef struct Side {
  short dy;                     // 12.4 format
  short dxdy, dudy, dvdy;       // 8.8 format
  short x, u, v;                // 8.8 format
} SideT;

static inline int part_int(int num, int shift) {
  int r = abs(num) >> shift;
  return num >= 0 ? r : -r;
}

static inline int part_frac(int num, int shift) {
  int r = abs(num) & ((1 << shift) - 1);
  return 10000 * r >> shift;
}

#define FRAC(n, k) part_int((n), (k)), part_frac((n), (k))

static void InitSide(SideT *s, CornerT *pa, CornerT *pb) {
  short dy = pb->y - pa->y;
  short dx = pb->x - pa->x;
  short du = pb->u - pa->u;
  short dv = pb->v - pa->v;

  s->dy = dy;

#if 0
  Log("A: x: %d.%04d, y: %d.%04d, u: %d.%04d, v: %d.%04d\n",
      FRAC(pa->x, 4), FRAC(pa->y, 4), FRAC(pa->u, 4), FRAC(pa->v, 4));
  Log("B: x: %d.%04d, y: %d.%04d, u: %d.%04d, v: %d.%04d\n",
      FRAC(pb->x, 4), FRAC(pb->y, 4), FRAC(pb->u, 4), FRAC(pb->v, 4));
  Log("> dx: %d.%04d, dy: %d.%04d, du: %d.%04d, dv: %d.%04d\n",
      FRAC(s->dx, 4), FRAC(dy, 4), FRAC(s->du, 4), FRAC(s->dv, 4));
#endif

  if (pb->yi > pa->yi) {
    short prestep = ((pa->y + 7) & -16) + 8 - pa->y;

#if 0
    Log("> prestep: %d.%04d\n", FRAC(prestep, 4));
#endif

    s->dxdy = div16(0x80 + (dx << 8), dy);              // 20.12 / 12.4 = 8.8
    s->x = (pa->x << 4) + (s->dxdy * prestep >> 4);     // 8.8 * 12.4 = 20.12

    s->dudy = div16(0x80 + (du << 8), dy);
    s->u = (pa->u << 4) + (s->dudy * prestep >> 4);

    s->dvdy = div16(0x80 + (dv << 8), dy);
    s->v = (pa->v << 4) + (s->dvdy * prestep >> 4);
  } else {
    s->dxdy = 0;
    s->x = pa->x << 4;

    s->dudy = 0;
    s->u = pa->u << 4;

    s->dvdy = 0;
    s->v = pa->v << 4;
  }

  /* texture is streched in U by 2 */
  s->u *= 2;
  s->dudy *= 2;

#if 0
    Log("> dxdy: %d.%04d, x: %d.%04d\n", FRAC(s->dxdy, 8), FRAC(s->x, 8));
    Log("> dudy: %d.%04d, u: %d.%04d\n", FRAC(s->dudy, 8), FRAC(s->u, 8));
    Log("> dvdy: %d.%04d, v: %d.%04d\n", FRAC(s->dvdy, 8), FRAC(s->v, 8));
#endif
}

void DrawTriPart(u_char *line asm("a0"), short *texture asm("a1"),
                 SideT *left asm("a2"), SideT *right asm("a3"),
                 int du asm("d2"), int dv asm("d3"),
                 int ys asm("d6"), int ye asm("d7"));

static void DrawTriangle(CornerT *p0, CornerT *p1, CornerT *p2, int color) {
  short *tex = texture[color];

  // sort them by y
  if (p0->y > p1->y) { CornerT *t = p1; p1 = p0; p0 = t; }
  if (p0->y > p2->y) { CornerT *t = p2; p2 = p0; p0 = t; }
  if (p1->y > p2->y) { CornerT *t = p2; p2 = p1; p1 = t; }

  p0->yi = (p0->y + 7) >> 4;
  p1->yi = (p1->y + 7) >> 4;
  p2->yi = (p2->y + 7) >> 4;

  {
    static __code SideT s01, s02, s12;
    short du, dv;

    InitSide(&s01, p0, p1);
    InitSide(&s02, p0, p2);
    InitSide(&s12, p1, p2);

    {
      short u, v, x;

      if (s02.dxdy == s01.dxdy) {
        x = s02.dxdy - s12.dxdy;
        u = s02.dudy - s12.dudy;
        v = s02.dvdy - s12.dvdy;
      } else if (s02.dxdy == s12.dxdy) {
        x = s02.dxdy - s01.dxdy;
        u = s02.dudy - s01.dudy;
        v = s02.dvdy - s01.dvdy;
      } else if (s12.dy < s01.dy) {
        x = s02.dxdy - s01.dxdy;
        u = s02.dudy - s01.dudy;
        v = s02.dvdy - s01.dvdy;
      } else {
        x = s02.dxdy - s12.dxdy;
        u = s02.dudy - s12.dudy;
        v = s02.dvdy - s12.dvdy;
      }

      du = div16(u << 8, x);
      dv = div16(v << 8, x);
    }

    // ((s02.x < s01.x) || (s02.x == s01.x && s02.dxdy > s01.dxdy))
    if (s01.dxdy < s02.dxdy) {
      DrawTriPart(chunky, tex, &s01, &s02, du, dv, p0->yi, p1->yi);
      DrawTriPart(chunky, tex, &s12, &s02, du, dv, p1->yi, p2->yi);
    } else {
      DrawTriPart(chunky, tex, &s02, &s01, du, dv, p0->yi, p1->yi);
      DrawTriPart(chunky, tex, &s02, &s12, du, dv, p1->yi, p2->yi);
    }
  }
}

#define MULVERTEX(D) {                          \
  short t0 = (*v++) + y;                        \
  short t1 = (*v++) + x;                        \
  int t2 = (*v++) * z;                          \
  short t3 = (*v++);                            \
  D = normfx(t0 * t1 + t2 - x * y) + t3;        \
}

static void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  void *_objdat = object->objdat;
  short *group = object->vertexGroups;

  /* WARNING! This modifies camera matrix! */
  M->x -= normfx(M->m00 * M->m01);
  M->y -= normfx(M->m10 * M->m11);
  M->z -= normfx(M->m20 * M->m21);

  /*
   * A = m00 * m01
   * B = m10 * m11
   * C = m20 * m21 
   * yx = y * x
   *
   * (m00 + y) * (m01 + x) + m02 * z - yx + (mx - A)
   * (m10 + y) * (m11 + x) + m12 * z - yx + (my - B)
   * (m20 + y) * (m21 + x) + m22 * z - yx + (mz - C)
   */

  do {
    short i;

    while ((i = *group++)) {
      short *pt = (short *)POINT(i);
      short *v = (short *)M;
      short x, y, z;
      int xp, yp, zp;

      x = *pt++;
      y = *pt++;
      z = *pt++;

      MULVERTEX(xp);
      MULVERTEX(yp);
      MULVERTEX(zp);

      *pt++ = div16(xp << 12, zp) + fx4i(WIDTH / 2);
      *pt++ = div16(yp << 12, zp) + fx4i(HEIGHT / 2);
      *pt++ = zp;
    }
  } while (*group);
}

static void DrawObject(Object3D *object) {
  void *_objdat = object->objdat;
  short *group = object->faceGroups;

  static __code CornerT corners[3];

  do {
    short f;

    while ((f = *group++)) {
      if (FACE(f)->flags >= 0) {
        register short *index asm("a3") = (short *)(FACE(f)->indices);
        short n = FACE(f)->count - 1;
        int color = texture_light[(short)FACE(f)->flags];
        CornerT *corner = corners;

        do {
          short i;

          i = *index++; /* vertex */
          corner->x = VERTEX(i)->x;
          corner->y = VERTEX(i)->y;
          i = *index++; /* uvcoord */
          corner->u = UVCOORD(i)->u;
          corner->v = UVCOORD(i)->v;
          corner++;
        } while (--n != -1);

        DrawTriangle(&corners[0], &corners[1], &corners[2], color);
      }
    }
  } while (*group);
}

/* Calculate OCS blit size, `W` is in 16-bit words. */
#define BLTSIZE(W, H) (((W) & 63) | (((H) & 1023) << 6))

/* This is the size of a single bitplane in `screen`. */
#define BUFSIZE ((WIDTH / 8) * HEIGHT * 4) /* 8192 bytes */

/* If you think you can speed it up (I doubt it) please first look into
 * `c2p_2x1_4bpl_pixels_per_byte_blitter.py` in `prototypes/c2p`. */

#define C2P_LAST 12

static void ChunkyToPlanar(CustomPtrT custom_) {
  register void **bpl = c2p_bpl;

  /*
   * Our chunky buffer of size (WIDTH, HEIGHT) is stored in bpl[0] and bpl[1].
   * Each 32-bit long word of chunky buffer contains four pixels [a b c d]
   * in scrambled format described below.
   * Please note that a_i is the i-th least significant bit of a.
   *
   * [ a0 a1 -- -- a2 a3 -- -- | b0 b1 -- -- b2 b3 -- -- |
   *   c0 c1 -- -- c2 c3 -- -- | d0 d1 -- -- d2 d3 -- -- ]
   *
   * Chunky to planar is divided into the following major steps:
   * 
   * ...: TODO
   * ...: TODO
   * Swap 4x2: in two consecutive 16-bit words swap diagonally two bits,
   *           i.e. [b0 b1] <-> [c0 c1], [b2 b3] <-> [c2 c3].
   * Expand 2x1: [x0 x1 ...] is translated into [x0 x0 ...] and [x1 x1 ...]
   *             and copied to corresponding bitplanes, this step effectively
   *             stretches pixels to 2x1.
   *
   * Line doubling is performed using copper. Rendered bitmap will have size
   * (WIDTH*2, HEIGHT, DEPTH) and will be placed in bpl[2] and bpl[3].
   */

  custom_->intreq_ = INTF_BLIT;

  switch (c2p_phase) {
    case 0:
      /* Initialize chunky to planar. */
      custom_->bltafwm = -1;
      custom_->bltalwm = -1;

      custom_->bltamod = 0;
      custom_->bltbmod = 0;
      custom_->bltdmod = 0;
      custom_->bltcdat = 0x3300;

      custom_->bltapt = bpl[0] + BUFSIZE * 2 - 2;
      custom_->bltbpt = bpl[0] + BUFSIZE * 2 - 2;
      custom_->bltdpt = bpl[2] + BUFSIZE * 2 - 2;
      
      /* ((a << 6) & 0xCC00) | (b & ~0xCC00) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(6);
      custom_->bltcon1 = BLITREVERSE;

      /* overall size: BUFSIZE * 2 bytes (chunk buffer size) */
      custom_->bltsize = BLTSIZE(WIDTH / 2, HEIGHT);
      break; /* B */

    case 1:
      custom_->bltamod = 6;
      custom_->bltbmod = 6;
      custom_->bltdmod = 2;

      custom_->bltapt = bpl[2];
      custom_->bltbpt = bpl[2] + 4;
      custom_->bltdpt = bpl[0];
      custom_->bltcdat = 0xFF00;

      /* (a & 0xFF00) | ((b >> 8) & ~0xFF00) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom_->bltcon1 = BSHIFT(8);

    case 2:
      /* invoked twice, overall size: BUFSIZE / 2 bytes */
      custom_->bltsize = BLTSIZE(1, BUFSIZE / 8);
      break; /* C (even) */

    case 3:
      custom_->bltapt = bpl[2] + 2;
      custom_->bltbpt = bpl[2] + 6;
      custom_->bltdpt = bpl[0] + 2;

    case 4:
      /* invoked twice, overall size: BUFSIZE / 2 bytes */
      custom_->bltsize = BLTSIZE(1, BUFSIZE / 8);
      break; /* C (odd) */

    case 5:
      custom_->bltamod = 2;
      custom_->bltbmod = 2;
      custom_->bltdmod = 0;
      custom_->bltcdat = 0xF0F0;

      /* Swap 4x2, pass 1, high-bits. */
      custom_->bltapt = bpl[0];
      custom_->bltbpt = bpl[0] + 2;
      custom_->bltdpt = bpl[1] + BUFSIZE / 2;

      /* (a & 0xF0F0) | ((b >> 4) & ~0xF0F0) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom_->bltcon1 = BSHIFT(4);

    case 6:
      /* invoked twice, overall size: BUFSIZE / 2 bytes */
      custom_->bltsize = BLTSIZE(1, BUFSIZE / 8);
      break; /* D[0] */

    case 7:
      /* Swap 4x2, pass 2, low-bits. */
      custom_->bltapt = bpl[1] - 4;
      custom_->bltbpt = bpl[1] - 2;
      custom_->bltdpt = bpl[1] + BUFSIZE / 2 - 2;

      /* ((a << 4) & 0xF0F0) | (b & ~0xF0F0) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(4);
      custom_->bltcon1 = BLITREVERSE;

    case 8:
      /* invoked twice, overall size: BUFSIZE / 2 bytes */
      custom_->bltsize = BLTSIZE(1, BUFSIZE / 8);
      break;

    case 9:
      custom_->bltamod = 0;
      custom_->bltbmod = 0;
      custom_->bltdmod = 0;
      custom_->bltcdat = 0xAAAA;

      custom_->bltapt = bpl[1];
      custom_->bltbpt = bpl[1];
      custom_->bltdpt = bpl[3];

      /* (a & 0xAAAA) | ((b >> 1) & ~0xAAAA) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom_->bltcon1 = BSHIFT(1);

      /* overall size: BUFSIZE bytes */
      custom_->bltsize = BLTSIZE(4, BUFSIZE / 8);
      break;

    case 10:
      custom_->bltapt = bpl[1] + BUFSIZE - 2;
      custom_->bltbpt = bpl[1] + BUFSIZE - 2;
      custom_->bltdpt = bpl[2] + BUFSIZE - 2;
      custom_->bltcdat = 0xAAAA;

      /* ((a << 1) & 0xAAAA) | (b & ~0xAAAA) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(1);
      custom_->bltcon1 = BLITREVERSE;

      /* overall size: BUFSIZE bytes */
      custom_->bltsize = BLTSIZE(4, BUFSIZE / 8);
      break;

    case 11:
      CopInsSet32(&bplptr[0], bpl[2]);
      CopInsSet32(&bplptr[1], bpl[3]);
      CopInsSet32(&bplptr[2], bpl[2] + BUFSIZE / 2);
      CopInsSet32(&bplptr[3], bpl[3] + BUFSIZE / 2);

      /* initialize rendering buffer */
      custom_->bltdpt = bpl[0];
#if 1
      /* copy the background */
      custom_->bltapt = background_pixels;
      custom_->bltcon0 = (SRCA | DEST) | A_TO_D;
#else
      /* set all pixels to 0 */
      custom_->bltadat = 0;
      custom_->bltcon0 = DEST | A_TO_D;
#endif
      custom_->bltcon1 = 0;

      /* overall size: BUFSIZE * 2 bytes (chunk buffer size) */
      custom_->bltsize = BLTSIZE(WIDTH / 2, HEIGHT);
      break;

    default:
      break;
  }

  c2p_phase++;
}

static CopListT *MakeCopperList(short active) {
  CopListT *cp = NewCopList(HEIGHT * 2 * 3 + 50);
  short i;

  bplptr = CopSetupBitplanes(cp, screen[active], DEPTH);
  for (i = 0; i < HEIGHT * 2; i++) {
    CopWaitSafe(cp, Y(i), 0);
    /* Line doubling. */
    CopMove16(cp, bpl1mod, (i & 1) ? 0 : -(WIDTH * 2) / 8);
    CopMove16(cp, bpl2mod, (i & 1) ? 0 : -(WIDTH * 2) / 8);
  }
  return CopListFinish(cp);
}

static void Init(void) {
  object = NewObject3D(&cube);
  object->translate.z = fx4i(-250);

  screen[0] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH, BM_CLEAR);

  ScrambleBackground();

  texture[0] = ScrambleTexture(texture_0_pixels);
  texture[1] = ScrambleTexture(texture_1_pixels);
  texture[2] = ScrambleTexture(texture_2_pixels);
  texture[3] = ScrambleTexture(texture_3_pixels);
  texture[4] = ScrambleTexture(texture_4_pixels);

  SetupPlayfield(MODE_LORES, DEPTH, X(32), Y(0), WIDTH * 2, HEIGHT * 2);
  LoadColors(texture_colors, 0);

  cp = MakeCopperList(0);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER);

  active = 0;
  c2p_bpl = NULL;
  c2p_phase = 256;

  SetIntVector(INTB_BLIT, (IntHandlerT)ChunkyToPlanar, (void *)custom);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DisableINT(INTF_BLIT);
  ResetIntVector(INTB_BLIT);

  DeleteCopList(cp);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);

  MemFree(texture[0]);
  MemFree(texture[1]);
  MemFree(texture[2]);
  MemFree(texture[3]);
  MemFree(texture[4]);

  DeleteObject3D(object);
}

PROFILE(UpdateGeometry);
PROFILE(DrawObject);

static void Render(void) {
  chunky = screen[active]->planes[0];

  ProfilerStart(UpdateGeometry);
  {
    object->rotate.x = object->rotate.y = object->rotate.z = frameCount * 6;

    UpdateObjectTransformation(object);
    UpdateFaceVisibility(object);
    TransformVertices(object);
  }
  ProfilerStop(UpdateGeometry);

  ProfilerStart(DrawObject);
  {
    DrawObject(object);
  }
  ProfilerStop(DrawObject);

  while (c2p_phase < C2P_LAST)
    WaitBlitter();

  c2p_phase = 0;
  c2p_bpl = screen[active]->planes;
  ChunkyToPlanar(custom);
  active ^= 1;
}

EFFECT(TexTri, NULL, NULL, Init, Kill, Render, NULL);
