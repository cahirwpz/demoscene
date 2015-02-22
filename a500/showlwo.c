#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "ffp.h"
#include "memory.h"
#include "ilbm.h"

#define WIDTH  256
#define HEIGHT 256
#define DEPTH 4

static Object3D *cube;
static CopListT *cp;
static PaletteT *palette;
static BitmapT *screen;
static UWORD active = 0;
static CopInsT *bplptr[DEPTH];

static void Load() {
  // cube = LoadLWO("data/new_2.lwo", SPFlt(80));
  palette = LoadPalette("data/showlwo-pal.ilbm");
  cube = LoadLWO("data/codi.lwo", SPFlt(256));
  CalculateEdges(cube);
}

static void UnLoad() {
  DeletePalette(palette);
  DeleteObject3D(cube);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakePlayfield(cp, bplptr, screen, DEPTH);
  CopMakeDispWin(cp, X(32), Y(0), WIDTH, HEIGHT);
  CopLoadPal(cp, palette, 0);
  CopEnd(cp);
}

static void Init() {
  screen = NewBitmap(WIDTH, HEIGHT, DEPTH + 1);

  cp = NewCopList(80);
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

static void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
}

#define MULVERTEX1(D, E) {               \
  WORD t0 = (*v++) + y;                  \
  WORD t1 = (*v++) + x;                  \
  LONG t2 = (*v++) * z;                  \
  v++;                                   \
  D = ((t0 * t1 + t2 - x * y) >> 4) + E; \
}

#define MULVERTEX2(D) {                  \
  WORD t0 = (*v++) + y;                  \
  WORD t1 = (*v++) + x;                  \
  LONG t2 = (*v++) * z;                  \
  WORD t3 = (*v++);                      \
  D = normfx(t0 * t1 + t2 - x * y) + t3; \
}

static __regargs void Transform3D_2(Matrix3D *M, Point3D *out, Point3D *in, WORD n) {
  WORD *src = (WORD *)in;
  WORD *dst = (WORD *)out;
  LONG m0, m1;

  M->x -= normfx(M->m00 * M->m01);
  M->y -= normfx(M->m10 * M->m11);
  M->z -= normfx(M->m20 * M->m21);

  m0 = M->x << 8;
  m1 = M->y << 8;

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
    WORD *v = (WORD *)M;
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;
    LONG xp, yp;
    WORD zp;

    MULVERTEX1(xp, m0);
    MULVERTEX1(yp, m1);
    MULVERTEX2(zp);

    *dst++ = div16(xp, zp) + WIDTH / 2;  /* div(xp * 256, zp) */
    *dst++ = div16(yp, zp) + HEIGHT / 2; /* div(yp * 256, zp) */
    dst++;
  } while (--n > 0);
}

__regargs static void DrawObject(Object3D *object) {
  Point3D *point = object->cameraPoint;
  WORD *edge = (WORD *)object->edge;
  WORD edges = object->edges;
  APTR start = screen->planes[active];

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltcmod = WIDTH / 8;
  custom->bltdmod = WIDTH / 8;

  while (--edges >= 0) {
    WORD *p0 = (APTR)point + *edge++;
    WORD *p1 = (APTR)point + *edge++;
    WORD x0 = *p0++, y0 = *p0++;
    WORD x1 = *p1++, y1 = *p1++;

    if (y0 > y1) {
      swapr(x0, x1);
      swapr(y0, y1);
    }

    {
      APTR data = start + (((y0 << 5) + (x0 >> 3)) & ~1);
      WORD dmax = x1 - x0;
      WORD dmin = y1 - y0;
      WORD derr;
      UWORD bltcon1 = LINE_SOLID;

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

      derr = 2 * dmin - dmax;
      if (derr < 0)
        bltcon1 |= SIGNFLAG;
      bltcon1 |= rorw(x0 & 15, 4);

      {
        UWORD bltcon0 = rorw(x0 & 15, 4) | LINE_OR;
        UWORD bltamod = derr - dmax;
        UWORD bltbmod = 2 * dmin;
        UWORD bltsize = (dmax << 6) + 66;
        APTR bltapt = (APTR)(LONG)derr;

        WaitBlitter();

        custom->bltcon0 = bltcon0;
        custom->bltcon1 = bltcon1;
        custom->bltamod = bltamod;
        custom->bltbmod = bltbmod;
        custom->bltapt = bltapt;
        custom->bltcpt = data;
        custom->bltdpt = data;
        custom->bltsize = bltsize;
      }
    }
  }
}

static Point3D rotate = { 0, 0, 0 };

static void Render() {
  Matrix3D t;

  rotate.x += 16;
  rotate.y += 16;
  rotate.z += 16;

  BlitterClearSync(screen, active);

  {
    // LONG lines = ReadLineCounter();
    LoadRotate3D(&t, rotate.x, rotate.y, rotate.z);
    Translate3D(&t, 0, 0, fx4i(-250));
    Transform3D_2(&t, cube->cameraPoint, cube->vertex, cube->vertices);
    // Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  {
    // LONG lines = ReadLineCounter();
    DrawObject(cube);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();

  {
    WORD n = DEPTH;

    while (--n >= 0) {
      WORD i = (active + n + 1 - DEPTH) % 5;
      if (i < 0)
        i += DEPTH + 1;
      CopInsSet32(bplptr[n], screen->planes[i]);
    }
  }

  active = (active + 1) % 5;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
