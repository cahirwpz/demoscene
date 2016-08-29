#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "ffp.h"
#include "ilbm.h"

STRPTR __cwdpath = "data";

#define WIDTH  256
#define HEIGHT 256
#define DEPTH 4

static Mesh3D *mesh;
static Object3D *cube;
static CopListT *cp;
static PaletteT *palette;
static BitmapT *screen;
static UWORD active = 0;
static CopInsT *bplptr[DEPTH];

static void Load() {
  palette = LoadPalette("wireframe-pal.ilbm");
  mesh = LoadMesh3D("pilka.3d", SPFlt(50));
  CalculateVertexFaceMap(mesh);
  CalculateFaceNormals(mesh);
  CalculateEdges(mesh);
}

static void UnLoad() {
  DeletePalette(palette);
  DeleteMesh3D(mesh);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen, DEPTH);
  CopLoadPal(cp, palette, 0);
  CopEnd(cp);
}

static void Init() {
  cube = NewObject3D(mesh);
  cube->translate.z = fx4i(-250);

  screen = NewBitmap(WIDTH, HEIGHT, DEPTH + 1);

  cp = NewCopList(80);
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER;
}

static void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
  DeleteObject3D(cube);
}

static __regargs void UpdateEdgeVisibility(Object3D *object) {
  BYTE *vertexFlags = object->vertexFlags;
  BYTE *edgeFlags = object->edgeFlags;
  BYTE *faceFlags = object->faceFlags;
  IndexListT **faces = object->mesh->face;
  IndexListT *face = *faces++;
  IndexListT **faceEdges = object->mesh->faceEdge;
  IndexListT *faceEdge = *faceEdges++;

  memset(vertexFlags, 0, object->mesh->vertices);
  memset(edgeFlags, 0, object->mesh->edges);

  do {
    if (*faceFlags++) {
      WORD n = face->count - 3;
      WORD *vi = face->indices;
      WORD *ei = faceEdge->indices;

      /* Face has at least (and usually) three vertices / edges. */
      vertexFlags[*vi++] = -1;
      edgeFlags[*ei++]++;
      vertexFlags[*vi++] = -1;
      edgeFlags[*ei++]++;
      vertexFlags[*vi++] = -1;
      edgeFlags[*ei++]++;

      while (--n >= 0) {
        vertexFlags[*vi++] = -1;
        edgeFlags[*ei++]++;
      }
    }

    faceEdge = *faceEdges++;
  } while ((face = *faces++));
}

#define MULVERTEX1(D, E) {            \
  WORD t0 = (*v++) + y;               \
  WORD t1 = (*v++) + x;               \
  LONG t2 = (*v++) * z;               \
  v++;                                \
  D = ((t0 * t1 + t2 - xy) >> 4) + E; \
}

#define MULVERTEX2(D) {               \
  WORD t0 = (*v++) + y;               \
  WORD t1 = (*v++) + x;               \
  LONG t2 = (*v++) * z;               \
  WORD t3 = (*v++);                   \
  D = normfx(t0 * t1 + t2 - xy) + t3; \
}

static __regargs void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  WORD *v = (WORD *)M;
  WORD *src = (WORD *)object->mesh->vertex;
  WORD *dst = (WORD *)object->vertex;
  BYTE *flags = object->vertexFlags;
  register WORD n asm("d7") = object->mesh->vertices - 1;

  LONG m0 = (M->x - normfx(M->m00 * M->m01)) << 8;
  LONG m1 = (M->y - normfx(M->m10 * M->m11)) << 8;
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
      WORD x = *src++;
      WORD y = *src++;
      WORD z = *src++;
      LONG xy = x * y;
      LONG xp, yp;
      WORD zp;

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
    } else {
      src += 4;
      dst += 4;
    }
  } while (--n != -1);
}

static __regargs void DrawObject(Object3D *object, APTR start) {
  WORD *edge = (WORD *)object->mesh->edge;
  BYTE *edgeFlags = object->edgeFlags;
  Point3D *point = object->vertex;
  WORD n = object->mesh->edges;

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltcmod = WIDTH / 8;
  custom->bltdmod = WIDTH / 8;

  do {
    if (*edgeFlags++ > 0) {
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
    } else {
      edge += 2;
    }
  } while (--n > 0);
}

static void Render() {
  BlitterClear(screen, active);

  {
    // LONG lines = ReadLineCounter();
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 8;

    UpdateObjectTransformation(cube);
    if (RightMouseButton())
      memset(cube->faceFlags, -1, cube->mesh->faces);
    else
      UpdateFaceVisibility(cube);
    UpdateEdgeVisibility(cube);
    TransformVertices(cube);
    // Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  WaitBlitter();

  {
    // LONG lines = ReadLineCounter();
    DrawObject(cube, screen->planes[active]);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();

  {
    WORD n = DEPTH;

    while (--n >= 0) {
      WORD i = (active + n + 1 - DEPTH) % (DEPTH + 1);
      if (i < 0)
        i += DEPTH + 1;
      CopInsSet32(bplptr[n], screen->planes[i]);
    }
  }

  active = (active + 1) % (DEPTH + 1);
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
