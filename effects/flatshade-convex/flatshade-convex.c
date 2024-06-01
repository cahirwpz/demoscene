#include <strings.h>
#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "3d.h"
#include "fx.h"

#define WIDTH  256
#define HEIGHT 256
#define DEPTH  4

static Object3D *cube;
static CopListT *cp;
static CopInsPairT *bplptr;
static BitmapT *screen[2];
static short active;

#include "data/flatshade-pal.c"
#include "data/pilka.c"

static void Init(void) {
  cube = NewObject3D(&pilka);
  cube->translate.z = fx4i(-250);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);

  SetupPlayfield(MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  LoadColors(flatshade_colors, 0);

  cp = NewCopList(80);
  bplptr = CopSetupBitplanes(cp, screen[0], DEPTH);
  CopListFinish(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
  DeleteObject3D(cube);
}

static void UpdateEdgeVisibilityConvex(Object3D *object) {
  char *vertexFlags = object->vertexFlags;
  char *edgeFlags = object->edgeFlags;
  char *faceFlags = object->faceFlags;
  short **faces = object->face;
  short **faceEdges = object->faceEdge;
  short *face;

  register char s asm("d7") = -1;

  bzero(vertexFlags, object->vertices);
  bzero(edgeFlags, object->edges);

  while ((face = *faces++)) {
    short *faceEdge = *faceEdges++;
    char f = *faceFlags++;

    if (f >= 0) {
      short n = face[-1] - 3;

      /* Face has at least (and usually) three vertices / edges. */
      vertexFlags[*face++] = s;
      edgeFlags[*faceEdge++] ^= f;
      vertexFlags[*face++] = s;
      edgeFlags[*faceEdge++] ^= f;

      do {
        vertexFlags[*face++] = s;
        edgeFlags[*faceEdge++] ^= f;
      } while (--n != -1);
    }
  }
}

#define MULVERTEX1(D, E) {                      \
  short t0 = (*v++) + y;                        \
  short t1 = (*v++) + x;                        \
  int t2 = (*v++) * z;                          \
  v++;                                          \
  D = ((t0 * t1 + t2 - x * y) >> 4) + E;        \
}

#define MULVERTEX2(D) {                         \
  short t0 = (*v++) + y;                        \
  short t1 = (*v++) + x;                        \
  int t2 = (*v++) * z;                          \
  short t3 = (*v++);                            \
  D = normfx(t0 * t1 + t2 - x * y) + t3;        \
}

static void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  short *v = (short *)M;
  short *src = (short *)object->point;
  short *dst = (short *)object->vertex;
  char *flags = object->vertexFlags;
  register short n asm("d7") = object->vertices - 1;

  int m0 = (M->x << 8) - ((M->m00 * M->m01) >> 4);
  int m1 = (M->y << 8) - ((M->m10 * M->m11) >> 4);

  short cnt = 0;

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
    if (*flags++) {
      short x = *src++;
      short y = *src++;
      short z = *src++;
      int xp, yp;
      short zp;

      pushl(v);
      MULVERTEX1(xp, m0);
      MULVERTEX1(yp, m1);
      MULVERTEX2(zp);
      popl(v);

      *dst++ = div16(xp, zp) + WIDTH / 2;  /* div(xp * 256, zp) */
      *dst++ = div16(yp, zp) + HEIGHT / 2; /* div(yp * 256, zp) */
      *dst++ = zp;

      src++;
      dst++;
      cnt++;
    } else {
      src += 4;
      dst += 4;
    }
  } while (--n != -1);
}

static void DrawObject(void *planes, Object3D *object,
                       CustomPtrT custom_ asm("a6"))
{
  short *edge = (short *)object->edge;
  char *edgeFlags = object->edgeFlags;
  Point3D *point = object->vertex;
  short n = object->edges - 1;

  WaitBlitter();
  custom_->bltafwm = -1;
  custom_->bltalwm = -1;
  custom_->bltadat = 0x8000;
  custom_->bltbdat = 0xffff; /* Line texture pattern. */
  custom_->bltcmod = WIDTH / 8;
  custom_->bltdmod = WIDTH / 8;

  do {
    if (*edgeFlags) {
      short bltcon0, bltcon1, bltsize, bltbmod, bltamod;
      int bltapt, bltcpt;

      {
        short x0, y0, x1, y1;
        short dmin, dmax, derr;

        {
          short *p0 = (void *)point + *edge++;
          x0 = *p0++;
          y0 = *p0++;
        }

        {
          short *p1 = (void *)point + *edge++;
          x1 = *p1++;
          y1 = *p1++;
        }

        if (y0 == y1) {
          edgeFlags++;
          continue;
        }
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

        bltcpt = (int)planes + (short)(((y0 << 5) + (x0 >> 3)) & ~1);

        bltcon0 = rorw(x0 & 15, 4) | BC0F_LINE_EOR;
        bltcon1 |= rorw(x0 & 15, 4);

        dmin <<= 1;
        derr = dmin - dmax;

        bltamod = derr - dmax;
        bltbmod = dmin;
        bltsize = (dmax << 6) + 66;
        bltapt = derr;
      }

#define DRAWLINE()                              \
      WaitBlitter();                            \
      custom_->bltcon0 = bltcon0;               \
      custom_->bltcon1 = bltcon1;               \
      custom_->bltcpt = (void *)bltcpt;         \
      custom_->bltapt = (void *)bltapt;         \
      custom_->bltdpt = planes;                 \
      custom_->bltbmod = bltbmod;               \
      custom_->bltamod = bltamod;               \
      custom_->bltsize = bltsize;

      {
        char edgeColor = *edgeFlags++;

        if (edgeColor & 1) { DRAWLINE(); }
        bltcpt += WIDTH * HEIGHT / 8;
        if (edgeColor & 2) { DRAWLINE(); }
        bltcpt += WIDTH * HEIGHT / 8;
        if (edgeColor & 4) { DRAWLINE(); }
        bltcpt += WIDTH * HEIGHT / 8;
        if (edgeColor & 8) { DRAWLINE(); }
      }
    } else {
      edge += 2;
      edgeFlags++;
    }
  } while (--n != -1);
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

static void BitmapFillFast(BitmapT *dst) {
  void *bltpt = dst->planes[0] + (dst->bplSize * DEPTH) - 2;
  u_short bltsize = (0 << 6) | (WIDTH >> 4);

  WaitBlitter();

  custom->bltapt = bltpt;
  custom->bltdpt = bltpt;
  custom->bltamod = 0;
  custom->bltdmod = 0;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = BLITREVERSE | FILL_XOR;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltsize = bltsize;

  WaitBlitter();
}

PROFILE(Transform);
PROFILE(Draw);
PROFILE(Fill);

static void Render(void) {
  BitmapClearFast(screen[active]);

  /* ball: 92 points, 180 polygons, 270 edges */
  cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 8;

  ProfilerStart(Transform);
  {
    UpdateObjectTransformation(cube); // 18 lines
    UpdateFaceVisibility(cube); // 211 lines O(faces)
    UpdateEdgeVisibilityConvex(cube); // 78 lines O(edge)
    TransformVertices(cube); // 89 lines O(vertex)
  }
  ProfilerStop(Transform);

  ProfilerStart(Draw);
  {
    DrawObject(screen[active]->planes[0], cube, custom); // 237 lines
  }
  ProfilerStop(Draw);

  ProfilerStart(Fill);
  {
    BitmapFillFast(screen[active]); // 287 lines
  }
  ProfilerStop(Fill);

  CopUpdateBitplanes(bplptr, screen[active], DEPTH);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(FlatShadeConvex, NULL, NULL, Init, Kill, Render, NULL);
