#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "ffp.h"
#include "ilbm.h"
#include "tasks.h"

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
  mesh = LoadMesh3D("pilka.3d", SPFlt(65));
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
  EnableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG);
}

static void Kill() {
  DeleteCopList(cp);
  DeleteBitmap(screen);
  DeleteObject3D(cube);
}

static __regargs void UpdateFaceVisibilityFast(Object3D *object) {
  WORD *src = (WORD *)object->mesh->faceNormal;
  IndexListT **faces = object->mesh->face;
  BYTE *faceFlags = object->faceFlags;
  APTR vertex = object->mesh->vertex;
  WORD n = object->mesh->faces - 1;

  WORD cx = object->camera.x;
  WORD cy = object->camera.y;
  WORD cz = object->camera.z;

  do {
    IndexListT *face = *faces++;
    WORD px, py, pz;
    LONG f;

    {
      WORD *p = (WORD *)(vertex + (WORD)(*face->indices << 3));
      px = cx - *p++;
      py = cy - *p++;
      pz = cz - *p++;
    }

    {
      LONG x = *src++ * px;
      LONG y = *src++ * py;
      LONG z = *src++ * pz;
      f = x + y + z;
    }

    /* This depends on condition codes set by previous calculations! */
    asm volatile("smi %0@+" : "+a" (faceFlags) : "d" (f));

    src++;
  } while (--n != -1);
}

static __regargs void UpdateEdgeVisibility(Object3D *object) {
  BYTE *vertexFlags = object->vertexFlags;
  BYTE *edgeFlags = object->edgeFlags;
  BYTE *faceFlags = object->faceFlags;
  IndexListT **faces = object->mesh->face;
  IndexListT *face = *faces++;
  IndexListT **faceEdges = object->mesh->faceEdge;
  IndexListT *faceEdge = *faceEdges++;
  
  bzero(vertexFlags, object->mesh->vertices);
  bzero(edgeFlags, object->mesh->edges);

  do {
    if (*faceFlags++ >= 0) {
      WORD n = face->count - 3;
      WORD *vi = face->indices;
      WORD *ei = faceEdge->indices;

      /* Face has at least (and usually) three vertices / edges. */
      vertexFlags[*vi++] = -1;
      edgeFlags[*ei++] = -1;
      vertexFlags[*vi++] = -1;
      edgeFlags[*ei++] = -1;

      do {
        vertexFlags[*vi++] = -1;
        edgeFlags[*ei++] = -1;
      } while (--n != -1);
    }

    faceEdge = *faceEdges++;
    face = *faces++;
  } while (face);
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

static __regargs void DrawObject(Object3D *object, APTR bplpt,
                                 CustomPtrT custom asm("a6"))
{
  WORD *edge = (WORD *)object->mesh->edge;
  BYTE *edgeFlags = object->edgeFlags;
  Point3D *point = object->vertex;
  WORD n = object->mesh->edges - 1;

  WaitBlitter();
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltadat = 0x8000;
  custom->bltbdat = 0xffff; /* Line texture pattern. */
  custom->bltcmod = WIDTH / 8;
  custom->bltdmod = WIDTH / 8;

  do {
    if (*edgeFlags++) {
      APTR data;
      WORD x0, y0, x1, y1;

      {
        WORD *p0 = (APTR)point + *edge++;
        x0 = *p0++;
        y0 = *p0++;
      }
      
      {
        WORD *p1 = (APTR)point + *edge++;
        x1 = *p1++;
        y1 = *p1++;
      }

      if (y0 > y1) {
        swapr(x0, x1);
        swapr(y0, y1);
      }

      {
        WORD dmax = x1 - x0;
        WORD dmin = y1 - y0;
        WORD derr;
        UWORD bltcon1 = LINEMODE;

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
          WORD y0_ = y0 << 5;
          WORD x0_ = x0 >> 3;
          WORD start = (y0_ + x0_) & ~1;
          data = bplpt + start;
        }

        dmin <<= 1;
        derr = dmin - dmax;
        if (derr < 0)
          bltcon1 |= SIGNFLAG;
        bltcon1 |= rorw(x0 & 15, 4);

        {
          UWORD bltcon0 = rorw(x0 & 15, 4) | BC0F_LINE_OR;
          UWORD bltamod = derr - dmax;
          UWORD bltbmod = dmin;
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
  } while (--n != -1);
}

static void Render() {
  LONG lines = ReadLineCounter();

  BlitterClear(screen, active);

  {
    // LONG lines = ReadLineCounter();
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 8;

    UpdateObjectTransformation(cube);
    if (RightMouseButton())
      bzero(cube->faceFlags, cube->mesh->faces);
    else
      UpdateFaceVisibilityFast(cube);
    UpdateEdgeVisibility(cube);
    TransformVertices(cube);
    // Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  {
    // LONG lines = ReadLineCounter();
    DrawObject(cube, screen->planes[active], custom);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  {
    APTR *planes = screen->planes;
    WORD n = DEPTH;
    WORD i = active;

    while (--n >= 0) {
      CopInsSet32(bplptr[n], planes[i]);
      if (i == 0)
        i = DEPTH + 1;
      i--;
    }
  }

  Log("all: %ld\n", ReadLineCounter() - lines);

  TaskWait(VBlankEvent);

  active++;
  if (active > DEPTH)
    active = 0;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
