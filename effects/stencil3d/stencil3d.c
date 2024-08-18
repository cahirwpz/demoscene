#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "3d.h"
#include "fx.h"

#define WIDTH 256
#define HEIGHT 256
#define DEPTH 3

static __code Object3D *object;
static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code BitmapT *screen[2];
static __code BitmapT *buffer;
static __code int active = 0;

#include "data/background-data.c"
#include "data/background-pal.c"
#include "data/pattern-1-1.c"
#include "data/pattern-1-2.c"
#include "data/pattern-1-3.c"
#include "data/pattern-2-1.c"
#include "data/pattern-2-2.c"
#include "data/pattern-2-3.c"

#include "data/kurak-head.c"

static CopListT *MakeCopperList(void) {
  CopListT *cp =
    NewCopList(100 + background_height * (background_cols_width + 3));

  /* bitplane modulos for both playfields */
  CopMove16(cp, bpl1mod, 0);
  CopMove16(cp, bpl2mod, 0);

  CopWait(cp, Y(-1), 0);

  bplptr = CopMove32(cp, bplpt[0], screen[1]->planes[0]);
  CopMove32(cp, bplpt[1], background.planes[0]);
  CopMove32(cp, bplpt[2], screen[1]->planes[1]);
  CopMove32(cp, bplpt[3], background.planes[1]);
  CopMove32(cp, bplpt[4], screen[1]->planes[2]);

  {
    u_short *data = background_cols_pixels;
    short i;

    for (i = 0; i < background_height; i++) {
      short bgcol = *data++;

      /* Start exchanging palette colors at the end of previous line. */
      CopWaitSafe(cp, Y(i - 1), HP(320 - 32 - 4));
      CopMove16(cp, color[0], 0);

      CopWaitSafe(cp, Y(i), HP(0));
      CopMove16(cp, color[9], *data++);
      CopMove16(cp, color[10], *data++);
      CopMove16(cp, color[11], *data++);
      CopMove16(cp, color[0], bgcol);
    }
  }

  return CopListFinish(cp);
}

static void Init(void) {
  object = NewObject3D(&kurak);
  object->translate.z = fx4i(-256);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, 0);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, 0);
  buffer = NewBitmap(WIDTH, HEIGHT, 1, 0);

  /* keep the buffer as the last bitplane of both screens */
  screen[0]->planes[DEPTH] = buffer->planes[0];
  screen[1]->planes[DEPTH] = buffer->planes[0];

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);
  BitmapClear(screen[0]);
  BitmapClear(screen[1]);
  BitmapClear(buffer);
  WaitBlitter();

  SetupDisplayWindow(MODE_LORES, X(32), Y(0), WIDTH, HEIGHT);
  SetupBitplaneFetch(MODE_LORES, X(32), WIDTH);
  SetupMode(MODE_DUALPF, DEPTH + background_depth);
  LoadColors(pattern_1_colors, 0);
  LoadColors(pattern_2_colors, 4);

  /* reverse playfield priorities */
  custom->bplcon2 = 0;

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  BlitterStop();
  CopperStop();
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(buffer);
  DeleteCopList(cp);
  DeleteObject3D(object);
}

#define MULVERTEX1(D, E)                                                       \
  {                                                                            \
    short t0 = (*v++) + y;                                                     \
    short t1 = (*v++) + x;                                                     \
    int t2 = (*v++) * z;                                                       \
    v++;                                                                       \
    D = ((t0 * t1 + t2 - xy) >> 4) + E;                                        \
  }

#define MULVERTEX2(D)                                                          \
  {                                                                            \
    short t0 = (*v++) + y;                                                     \
    short t1 = (*v++) + x;                                                     \
    int t2 = (*v++) * z;                                                       \
    short t3 = (*v++);                                                         \
    D = normfx(t0 * t1 + t2 - xy) + t3;                                        \
  }

static void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  void *_objdat = object->objdat;
  short *group = object->vertexGroups;

  int m0 = (M->x << 8) - ((M->m00 * M->m01) >> 4);
  int m1 = (M->y << 8) - ((M->m10 * M->m11) >> 4);

  /* WARNING! This modifies camera matrix! */
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
      if (NODE3D(i)->flags) {
        short *pt = (short *)NODE3D(i);
        short *v = (short *)M;
        short x, y, z, zp;
        int xy, xp, yp;

        /* clear flags */
        *pt++ = 0;

        x = *pt++;
        y = *pt++;
        z = *pt++;
        xy = x * y;

        MULVERTEX1(xp, m0);
        MULVERTEX1(yp, m1);
        MULVERTEX2(zp);

        *pt++ = div16(xp, zp) + WIDTH / 2;  /* div(xp * 256, zp) */
        *pt++ = div16(yp, zp) + HEIGHT / 2; /* div(yp * 256, zp) */
        *pt++ = zp;
      }
    }
  } while (*group);
}

static __code void **patterns[2][3] = {
  {
    (void **)pattern_2_3.planes,
    (void **)pattern_2_2.planes,
    (void **)pattern_2_1.planes,
  },
  {
    (void **)pattern_1_3.planes,
    (void **)pattern_1_2.planes,
    (void **)pattern_1_1.planes,
  },
};

static __code short pattern_shade[16] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 2, 2, 2, 2
};

static void DrawObject(Object3D *object, void **planes,
                       CustomPtrT custom_ asm("a6")) {
  register SortItemT *item asm("a3") = object->visibleFace;
  void *_objdat = object->objdat;

  custom_->bltafwm = -1;
  custom_->bltalwm = -1;

  for (; item->index >= 0; item++) {
    short ii = item->index;

    {
      register short *index asm("a4") = (short *)&FACE(ii)->count;
      short m = (*index++) - 1;

      do {
        /* Draw edge. */
        short bltcon0, bltcon1, bltsize, bltbmod, bltamod;
        int bltapt, bltcpt;
        short x0, y0, x1, y1;
        short dmin, dmax, derr;

        /* skip vertex index */
        index++;

        {
          short i = *index++; /* edge index */
          short *edge = &EDGE(i)->point[0];
          short *vertex;

          vertex = &VERTEX(*edge++)->x;
          x0 = *vertex++;
          y0 = *vertex++;

          vertex = &VERTEX(*edge++)->x;
          x1 = *vertex++;
          y1 = *vertex++;
        }

        if (y0 == y1)
          continue;

        if (y0 > y1) {
          swapr(x0, x1);
          swapr(y0, y1);
        }

        dmax = x1 - x0;
        if (dmax < 0)
          dmax = -dmax;

        dmin = y1 - y0;
        if (dmax >= dmin) {
          if (x0 >= x1)
            bltcon1 = AUL | SUD | LINEMODE | ONEDOT;
          else
            bltcon1 = SUD | LINEMODE | ONEDOT;
        } else {
          if (x0 >= x1)
            bltcon1 = SUL | LINEMODE | ONEDOT;
          else
            bltcon1 = LINEMODE | ONEDOT;
          swapr(dmax, dmin);
        }

        bltcpt = (int)planes[DEPTH] + (short)(((y0 << 5) + (x0 >> 3)) & ~1);

        bltcon0 = rorw(x0 & 15, 4) | BC0F_LINE_EOR;
        bltcon1 |= rorw(x0 & 15, 4);

        dmin <<= 1;
        derr = dmin - dmax;
        if (derr < 0)
          bltcon1 |= SIGNFLAG;

        bltamod = derr - dmax;
        bltbmod = dmin;
        bltsize = (dmax << 6) + 66;
        bltapt = derr;

        _WaitBlitter(custom_);

        custom_->bltbdat = 0xffff;
        custom_->bltadat = 0x8000;
        custom_->bltcon0 = bltcon0;
        custom_->bltcon1 = bltcon1;
        custom_->bltcpt = (void *)bltcpt;
        custom_->bltapt = (void *)bltapt;
        custom_->bltdpt = planes[DEPTH];
        custom_->bltcmod = WIDTH / 8;
        custom_->bltbmod = bltbmod;
        custom_->bltamod = bltamod;
        custom_->bltdmod = WIDTH / 8;
        custom_->bltsize = bltsize;
      } while (--m != -1);
    }

    {
      short bltstart, bltend;
      u_short bltmod, bltsize;

      {
        short minX, minY, maxX, maxY;

        register short *index asm("a4") = (short *)&FACE(ii)->count;
        short m = (*index++) - 2;
        short *vertex;

        vertex = &VERTEX(*index++)->x;
        index++; /* skip edge index */

        minX = maxX = *vertex++;
        minY = maxY = *vertex++;

        /* Calculate area. */
        do {
          short x, y;

          vertex = &VERTEX(*index++)->x;
          index++; /* skip edge index */

          x = *vertex++;
          y = *vertex++;

          if (x < minX)
            minX = x;
          else if (x > maxX)
            maxX = x;
          if (y < minY)
            minY = y;
          else if (y > maxY)
            maxY = y;
        } while (--m != -1);

        /* Align to word boundary. */
        minX = (minX & ~15) >> 3;
        /* to avoid case where a line is on right edge */
        maxX = ((maxX + 16) & ~15) >> 3;

        {
          short w = maxX - minX;
          short h = maxY - minY + 1;

          bltstart = minX + minY * (WIDTH / 8);
          bltend = maxX + maxY * (WIDTH / 8) - 2;
          bltsize = (h << 6) | (w >> 1);
          bltmod = (WIDTH / 8) - w;
        }
      }

      /* Fill face. */
      {
        void *dst = planes[DEPTH] + bltend;

        _WaitBlitter(custom_);

        custom_->bltcon0 = (SRCA | DEST) | A_TO_D;
        custom_->bltcon1 = BLITREVERSE | FILL_XOR;
        custom_->bltapt = dst;
        custom_->bltdpt = dst;
        custom_->bltamod = bltmod;
        custom_->bltbmod = bltmod;
        custom_->bltcmod = bltmod;
        custom_->bltdmod = bltmod;
        custom_->bltsize = bltsize;
      }

      /* Copy filled face to screen. */
      {
        void *dst = planes[2] + bltstart;
        void *mask = planes[DEPTH] + bltstart;

        u_short bltcon0;

        if (FACE(ii)->material & 1) {
          bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
        } else {
          bltcon0 = (SRCA | SRCB | DEST) | A_AND_NOT_B;
        }

        _WaitBlitter(custom_);

        custom_->bltcon0 = bltcon0;
        custom_->bltcon1 = 0;
        custom_->bltapt = dst;
        custom_->bltbpt = mask;
        custom_->bltdpt = dst;
        custom_->bltsize = bltsize;
      }

      {
        void **srcbpl;
        void **dstbpl = planes;
        void *mask = planes[DEPTH] + bltstart;
        short shade = pattern_shade[(short)FACE(ii)->flags];
        short pat = FACE(ii)->material & 1;
        short i;

        srcbpl = patterns[pat][shade];

        for (i = 0; i < pattern_1_1_depth; i++) {
          void *src = *srcbpl++;
          void *dst = *dstbpl++ + bltstart;
          u_short bltcon0 =
            (SRCA | SRCB | SRCC | DEST) | (ABC | ABNC | NABC | NANBC);

          _WaitBlitter(custom_);

          custom_->bltcon0 = bltcon0;
          custom_->bltcon1 = 0;
          custom_->bltapt = mask;
          custom_->bltbpt = src;
          custom_->bltcpt = dst;
          custom_->bltdpt = dst;
          custom_->bltsize = bltsize;
        }
      }

      /* Clear working area. */
      {
        void *data = planes[DEPTH] + bltstart;

        _WaitBlitter(custom_);

        custom_->bltcon0 = (DEST | A_TO_D);
        custom_->bltadat = 0;
        custom_->bltdpt = data;
        custom_->bltsize = bltsize;
      }
    }
  }
}

static void BitmapClearFast(BitmapT *dst) {
  u_short height = (short)dst->height * (short)dst->depth;
  u_short bltsize = (height << 6) | (dst->bytesPerRow >> 1);
  void *bltpt = dst->planes[0];

  WaitBlitter();

  custom->bltcon0 = DEST | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0;
  custom->bltdmod = 0;
  custom->bltdpt = bltpt;
  custom->bltsize = bltsize;
}

PROFILE(Transform);
PROFILE(Draw);

static void Render(void) {
  BitmapClearFast(screen[active]);

  ProfilerStart(Transform);
  {
    object->rotate.x = object->rotate.y = object->rotate.z = frameCount * 8;
    UpdateObjectTransformation(object);
    UpdateFaceVisibility(object);
    UpdateVertexVisibility(object);
    TransformVertices(object);
    SortFaces(object);
  }
  ProfilerStop(Transform); // Average: 163

  ProfilerStart(Draw);
  DrawObject(object, screen[active]->planes, custom);
  ProfilerStop(Draw); // Average: 671

  CopInsSet32(&bplptr[0], screen[active]->planes[0]);
  CopInsSet32(&bplptr[2], screen[active]->planes[1]);
  CopInsSet32(&bplptr[4], screen[active]->planes[2]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Stencil3D, NULL, NULL, Init, Kill, Render, NULL);
