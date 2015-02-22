#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "ffp.h"

#define WIDTH  256
#define HEIGHT 256
#define DEPTH  1

static Mesh3D *mesh;
static Object3D *cube;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen[2];
static UWORD active = 0;

static void Load() {
  mesh = LoadLWO("data/codi.lwo", SPFlt(256));
  // mesh = LoadLWO("data/new_2.lwo", SPFlt(80));
  CalculateVertexFaceMap(mesh);
  CalculateFaceNormals(mesh);
}

static void UnLoad() {
  DeleteMesh3D(mesh);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakeDispWin(cp, X(32), Y(0), WIDTH, HEIGHT);
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH);
  CopSetRGB(cp,  0, 0x000);
  CopSetRGB(cp,  1, 0xFFF);
  CopEnd(cp);
}

static void Init() {
  cube = NewObject3D(mesh);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH);

  cp = NewCopList(80);
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

static void Kill() {
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteCopList(cp);
  DeleteObject3D(cube);
}

static inline void DrawLine(APTR start, Point2D *p0, Point2D *p1) {
  WORD x0, y0, x1, y1;

  if (p0->y > p1->y)
    swapr(p0, p1);

  x0 = p0->x;
  y0 = p0->y;
  x1 = p1->x;
  y1 = p1->y;

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

static __regargs void UpdateFaceVisibility(Object3D *object) {
  WORD *src = (WORD *)object->mesh->faceNormal;
  BYTE *vertexFlags = object->vertexFlags;
  BYTE *faceFlags = object->faceFlags;
  IndexListT **faces = object->mesh->face;
  IndexListT *face = *faces++;

  WORD m20 = object->world.m20;
  WORD m21 = object->world.m21;
  WORD m22 = object->world.m22;

  memset(vertexFlags, 0, object->mesh->vertices);

  do {
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;
    BYTE visible = (m20 * x + m21 * y + m22 * z >= 0) ? -1 : 0;

    *faceFlags++ = visible;

    if (visible) {
      WORD n = face->count - 1;
      WORD *index = face->indices;
      
      do {
        vertexFlags[*index++] = -1;
      } while (--n != -1);
    }
  } while ((face = *faces++));
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

static __regargs void CustomTransform3D(Object3D *object) {
  Matrix3D *M = &object->world;
  WORD *v = (WORD *)M;
  WORD *src = (WORD *)object->mesh->vertex;
  WORD *dst = (WORD *)object->point;
  BYTE *flags = object->vertexFlags;
  WORD n = object->mesh->vertices;
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
    if (*flags++) {
      WORD x = *src++;
      WORD y = *src++;
      WORD z = *src++;
      LONG xp, yp;
      WORD zp;

      pushl(v);
      MULVERTEX1(xp, m0);
      MULVERTEX1(yp, m1);
      MULVERTEX2(zp);
      popl(v);

      *dst++ = div16(xp, zp) + WIDTH / 2;  /* div(xp * 256, zp) */
      *dst++ = div16(yp, zp) + HEIGHT / 2; /* div(yp * 256, zp) */
    } else {
      src += 3;
      dst += 2;
    }
  } while (--n > 0);
}

static __regargs void DrawObject(Object3D *object, APTR start) {
  IndexListT **faces = object->mesh->face;
  IndexListT *face;
  UBYTE *faceFlags = object->faceFlags;
  Point2D *point = object->point;

  while ((face = *faces++)) {
    if (*faceFlags++) {
      WORD *index = face->indices;
      WORD n = face->count;
      Point2D *p0 = &point[index[n - 1]];
      Point2D *p1;

      while (--n >= 0) {
        p1 = &point[*index++];
        DrawLine(start, p0, p1);
        p0 = p1; 
      }
    }
  }
}

static void Render() {
  LONG a = ReadFrameCounter() * 8;

  BlitterClearSync(screen[active], 0);

  {
    // LONG lines = ReadLineCounter();
    LoadRotate3D(&cube->world, a, a, a);
    Translate3D(&cube->world, 0, 0, fx4i(-250));
    UpdateFaceVisibility(cube);
    CustomTransform3D(cube);
    // Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltcmod = WIDTH / 8;
  custom->bltdmod = WIDTH / 8;

  {
    // LONG lines = ReadLineCounter();
    DrawObject(cube, screen[active]->planes[0]);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();
  CopInsSet32(bplptr[0], screen[active]->planes[0]);
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
