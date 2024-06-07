#include <strings.h>
#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "3d.h"
#include "fx.h"

#define WIDTH  256
#define HEIGHT 256
#define DEPTH 4

static Object3D *cube;
static CopListT *cp;
static BitmapT *screen;
static u_short active = 0;
static CopInsPairT *bplptr;

#include "data/wireframe-pal.c"
#include "data/pilka.c"

static Mesh3D *mesh = &pilka;

static void Init(void) {
  cube = NewObject3D(mesh);
  cube->translate.z = fx4i(-250);

  screen = NewBitmap(WIDTH, HEIGHT, DEPTH + 1, BM_CLEAR);

  SetupPlayfield(MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  LoadColors(wireframe_colors, 0);

  cp = NewCopList(80);
  bplptr = CopSetupBitplanes(cp, screen, DEPTH);
  CopListFinish(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DeleteCopList(cp);
  DeleteBitmap(screen);
  DeleteObject3D(cube);
}

static void SetFaceVisibility(Object3D *object) {
  short **vertexIndexList = object->faceVertexIndexList;
  short n = object->faces - 1;

  do {
    short *vertexIndex = *vertexIndexList++;
    vertexIndex[FV_FLAGS] = 0;
  } while (--n != -1);
}

static void UpdateFaceVisibilityFast(Object3D *object) {
  short *src = (short *)object->faceNormal;
  short **vertexIndexList = object->faceVertexIndexList;
  void *point = object->point;
  short n = object->faces - 1;

  short cx = object->camera.x;
  short cy = object->camera.y;
  short cz = object->camera.z;

  do {
    short *vertexIndex = *vertexIndexList++;
    short px, py, pz;
    int f;

    {
      short *p = (short *)(point + (short)(*vertexIndex << 3));
      px = cx - *p++;
      py = cy - *p++;
      pz = cz - *p++;
    }

    {
      int x = *src++ * px;
      int y = *src++ * py;
      int z = *src++ * pz;
      f = x + y + z;
    }

    /* This depends on condition codes set by previous calculations! */
    vertexIndex[FV_FLAGS] = (f < 0) ? f : 0;

    src++;
  } while (--n != -1);
}

static void UpdateEdgeVisibility(Object3D *object) {
  char *vertexFlags = &object->vertex[0].flags;
  char *edgeFlags = &object->edge[0].flags;
  short **vertexIndexList = object->faceVertexIndexList;
  short **edgeIndexList = object->faceEdgeIndexList;
  short f = object->faces;
  
  while (--f >= 0) {
    short *vertexIndex = *vertexIndexList++;
    short *edgeIndex = *edgeIndexList++;

    if (vertexIndex[FV_FLAGS] >= 0) {
      short n = vertexIndex[FV_COUNT] - 3;
      short i;

      /* Face has at least (and usually) three vertices / edges. */
      i = *vertexIndex++ << 3; vertexFlags[i] = -1;
      i = *edgeIndex++ << 4; edgeFlags[i] = -1;
      i = *vertexIndex++ << 3; vertexFlags[i] = -1;
      i = *edgeIndex++ << 4; edgeFlags[i] = -1;

      do {
        i = *vertexIndex++ << 3; vertexFlags[i] = -1;
        i = *edgeIndex++ << 4; edgeFlags[i] = -1;
      } while (--n != -1);
    }
  }
}

#define MULVERTEX1(D, E) {              \
  short t0 = (*v++) + y;                \
  short t1 = (*v++) + x;                \
  int t2 = (*v++) * z;                  \
  v++;                                  \
  D = ((t0 * t1 + t2 - xy) >> 4) + E;   \
}

#define MULVERTEX2(D) {                 \
  short t0 = (*v++) + y;                \
  short t1 = (*v++) + x;                \
  int t2 = (*v++) * z;                  \
  short t3 = (*v++);                    \
  D = normfx(t0 * t1 + t2 - xy) + t3;   \
}

static void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  short *mx = (short *)M;
  short *src = (short *)object->point;
  short *dst = (short *)object->vertex;
  register short n asm("d7") = object->vertices - 1;

  int m0 = (M->x - normfx(M->m00 * M->m01)) << 8;
  int m1 = (M->y - normfx(M->m10 * M->m11)) << 8;
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
    if (((Point3D *)dst)->flags) {
      short x = *src++;
      short y = *src++;
      short z = *src++;
      short *v = mx;
      int xy = x * y;
      int xp, yp;
      short zp;

      MULVERTEX1(xp, m0);
      MULVERTEX1(yp, m1);
      MULVERTEX2(zp);

      *dst++ = div16(xp, zp) + WIDTH / 2;  /* div(xp * 256, zp) */
      *dst++ = div16(yp, zp) + HEIGHT / 2; /* div(yp * 256, zp) */
      *dst++ = zp;

      src++;
      *dst++ = 0;
    } else {
      src += 4;
      dst += 4;
    }
  } while (--n != -1);
}

static void DrawObject(Object3D *object, void *bplpt,
                       CustomPtrT custom_ asm("a6"))
{
  EdgeT *edge = object->edge;
  short n = object->edges - 1;

  WaitBlitter();
  custom_->bltafwm = -1;
  custom_->bltalwm = -1;
  custom_->bltadat = 0x8000;
  custom_->bltbdat = 0xffff; /* Line texture pattern. */
  custom_->bltcmod = WIDTH / 8;
  custom_->bltdmod = WIDTH / 8;

  do {
    if (edge->flags) {
      void *data;
      short x0, y0, x1, y1;

      {
        short *p = &edge->point[0]->x;
        x0 = *p++;
        y0 = *p++;
      }
      
      {
        short *p = &edge->point[1]->x;
        x1 = *p++;
        y1 = *p++;
      }

      /* clear visibility */
      edge->flags = 0;

      if (y0 > y1) {
        swapr(x0, x1);
        swapr(y0, y1);
      }

      {
        short dmax = x1 - x0;
        short dmin = y1 - y0;
        short derr;
        u_short bltcon1 = LINEMODE;

        if (dmax < 0)
          dmax = -dmax;

        if (dmax >= dmin) {
          if (x0 >= x1)
            bltcon1 |= (AUL | SUD);
          else
            bltcon1 |= SUD;
        } else {
          if (x0 >= x1)
            bltcon1 |= SUL;
          swapr(dmax, dmin);
        }

        {
          short y0_ = y0 << 5;
          short x0_ = x0 >> 3;
          short start = (y0_ + x0_) & ~1;
          data = bplpt + start;
        }

        dmin <<= 1;
        derr = dmin - dmax;
        if (derr < 0)
          bltcon1 |= SIGNFLAG;
        bltcon1 |= rorw(x0 & 15, 4);

        {
          u_short bltcon0 = rorw(x0 & 15, 4) | BC0F_LINE_OR;
          u_short bltamod = derr - dmax;
          u_short bltbmod = dmin;
          u_short bltsize = (dmax << 6) + 66;
          void *bltapt = (void *)(int)derr;

          WaitBlitter();

          custom_->bltcon0 = bltcon0;
          custom_->bltcon1 = bltcon1;
          custom_->bltamod = bltamod;
          custom_->bltbmod = bltbmod;
          custom_->bltapt = bltapt;
          custom_->bltcpt = data;
          custom_->bltdpt = data;
          custom_->bltsize = bltsize;
        }
      }
    }

    edge++;
  } while (--n != -1);
}

PROFILE(Transform);
PROFILE(Draw);

static void Render(void) {
  BlitterClear(screen, active);

  ProfilerStart(Transform);
  {
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 8;

    UpdateObjectTransformation(cube);
    if (RightMouseButton())
      SetFaceVisibility(cube);
    else
      UpdateFaceVisibilityFast(cube);
    UpdateEdgeVisibility(cube);
    TransformVertices(cube);
  }
  ProfilerStop(Transform);

  ProfilerStart(Draw);
  {
    DrawObject(cube, screen->planes[active], custom);
  }
  ProfilerStop(Draw);

  {
    void **planes = screen->planes;
    short n = DEPTH;
    short i = active;

    while (--n >= 0) {
      CopInsSet32(&bplptr[n], planes[i]);
      if (i == 0)
        i = DEPTH + 1;
      i--;
    }
  }

  TaskWaitVBlank();

  active++;
  if (active > DEPTH)
    active = 0;
}

EFFECT(Wireframe, NULL, NULL, Init, Kill, Render, NULL);
