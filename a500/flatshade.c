#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "ffp.h"

#define WIDTH  256
#define HEIGHT 256
#define DEPTH  3

static Mesh3D *mesh;
static Object3D *cube;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen0, *screen1;
static BitmapT *buffer;

static void Load() {
  mesh = LoadLWO("data/codi2.lwo", SPFlt(384));
  // mesh = LoadLWO("data/new_2.lwo", SPFlt(80));
  // mesh = LoadLWO("data/cube.lwo", SPFlt(50));
  CalculateFaceNormals(mesh);
  CalculateEdges(mesh);
}

static void UnLoad() {
  DeleteMesh3D(mesh);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen0, DEPTH);
  CopSetRGB(cp, 0, 0x000);
  CopSetRGB(cp, 1, 0x333);
  CopSetRGB(cp, 2, 0x555);
  CopSetRGB(cp, 3, 0x777);
  CopSetRGB(cp, 4, 0x999);
  CopSetRGB(cp, 5, 0xBBB);
  CopSetRGB(cp, 6, 0xDDD);
  CopSetRGB(cp, 7, 0xFFF);
  CopEnd(cp);
}

static void Init() {
  cube = NewObject3D(mesh);
  cube->translate.z = fx4i(-250);

  screen0 = NewBitmap(WIDTH, HEIGHT, DEPTH);
  screen1 = NewBitmap(WIDTH, HEIGHT, DEPTH);
  buffer = NewBitmap(WIDTH, HEIGHT, 1);

  cp = NewCopList(80);
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG;
}

static void Kill() {
  DeleteBitmap(screen0);
  DeleteBitmap(screen1);
  DeleteBitmap(buffer);
  DeleteCopList(cp);
  DeleteObject3D(cube);
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

static __regargs void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  WORD *v = (WORD *)M;
  WORD *src = (WORD *)object->mesh->vertex;
  WORD *dst = (WORD *)object->vertex;
  BYTE *flags = object->vertexFlags;
  register WORD n asm("d7") = object->mesh->vertices - 1;

  LONG m0 = (M->x << 8) - ((M->m00 * M->m01) >> 4);
  LONG m1 = (M->y << 8) - ((M->m10 * M->m11) >> 4);

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

#define MoveLong(reg, hi, lo) \
    *(ULONG *)(&custom->reg) = (((hi) << 16) | (lo))

static __regargs void DrawLine(WORD x0, WORD y0, WORD x1, WORD y1) {
  if (y0 > y1) {
    swapr(x0, x1);
    swapr(y0, y1);
  }

  {
    WORD dmax = x1 - x0;
    WORD dmin = y1 - y0;
    WORD derr;
    UWORD bltcon1 = LINE_ONEDOT;

    if (dmax < 0)
      dmax = -dmax;

    if (dmax >= dmin) {
      if (x0 >= x1)
        bltcon1 |= AUL;
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
      APTR src = buffer->planes[0];
      WORD start = ((y0 << 5) + (x0 >> 3)) & ~1;
      APTR dst = src + start;
      UWORD bltcon0 = rorw(x0 & 15, 4) | LINE_EOR;
      UWORD bltamod = derr - dmax;
      UWORD bltbmod = 2 * dmin;
      UWORD bltsize = (dmax << 6) + 66;
      APTR bltapt = (APTR)(LONG)derr;

      WaitBlitter();

      custom->bltadat = 0x8000;
      custom->bltbdat = 0xffff; /* Line texture pattern. */
      custom->bltcon0 = bltcon0;
      custom->bltcon1 = bltcon1;
      custom->bltamod = bltamod;
      custom->bltbmod = bltbmod;
      custom->bltcmod = WIDTH / 8;
      custom->bltdmod = WIDTH / 8;
      custom->bltapt = bltapt;
      custom->bltcpt = dst;
      custom->bltdpt = src;
      custom->bltsize = bltsize;
    }
  }
}

static Box2D area;

static __regargs void DrawFace(IndexListT *face, APTR point) {
  WORD *i = face->indices;
  WORD m = face->count - 1;
  WORD *ptr = (WORD *)(point + (WORD)(i[m] << 3));
  WORD x0 = *ptr++;
  WORD y0 = *ptr++;

  area.minX = x0;
  area.minY = y0;
  area.maxX = x0;
  area.maxY = y0;

  do {
    WORD *ptr = (WORD *)(point + (WORD)(*i++ << 3));
    WORD x1 = *ptr++;
    WORD y1 = *ptr++;

    /* Estimate the size of rectangle that contains a face. */
    if (x1 < area.minX)
      area.minX = x1;
    else if (x1 > area.maxX)
      area.maxX = x1;
    if (y1 < area.minY)
      area.minY = y1;
    else if (y1 > area.maxY)
      area.maxY = y1;

    DrawLine(x0, y0, x1, y1);

    x0 = x1; y0 = y1;
  } while (--m != -1);
}

static __regargs void DrawObject(Object3D *object, APTR *screen, APTR buffer) {
  BYTE *faceFlags = object->faceFlags;
  IndexListT **faces = object->mesh->face;
  SortItemT *item = object->visibleFace;
  WORD n = object->visibleFaces;

  custom->bltafwm = -1;
  custom->bltalwm = -1;

  while (--n >= 0) {
    LONG index = item->index;
    IndexListT *face = faces[index];

    WORD bltstart, bltend;
    UWORD bltmod, bltsize;

    DrawFace(face, object->vertex);
    
    {
      /* Align to word boundary. */
      WORD minX = (area.minX & ~15) >> 3;
      /* to avoid case where a line is on right edge */
      WORD maxX = ((area.maxX + 16) & ~15) >> 3;
      WORD minY = area.minY;
      WORD maxY = area.maxY;

      WORD w = maxX - minX;
      WORD h = maxY - minY + 1;

      bltstart = minX + minY * (WIDTH / 8);
      bltend = maxX + maxY * (WIDTH / 8) - 2;
      bltsize = (h << 6) | (w >> 1);
      bltmod = (WIDTH / 8) - w;
    }

    /* Fill face. */
    {
      APTR src = buffer + bltend;

      WaitBlitter();

      custom->bltcon0 = (SRCA | DEST) | A_TO_D;
      custom->bltcon1 = BLITREVERSE | FILL_XOR;
      custom->bltapt = src;
      custom->bltdpt = src;
      custom->bltamod = bltmod;
      custom->bltdmod = bltmod;
      custom->bltsize = bltsize;
    }

    /* Copy filled face to screen. */
    {
      APTR src = buffer + bltstart;
      BYTE mask = 1 << (DEPTH - 1);
      WORD n = DEPTH;
      BYTE color = (faceFlags[index] + 1) >> 1;

      while (--n >= 0) {
        APTR dst = screen[n] + bltstart;
        UWORD bltcon0;

        if (color & mask)
          bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
        else
          bltcon0 = (SRCA | SRCB | DEST) | (NABC | NABNC);

        WaitBlitter();

        custom->bltcon0 = bltcon0;
        custom->bltcon1 = 0;
        custom->bltapt = src;
        custom->bltbpt = dst;
        custom->bltdpt = dst;
        custom->bltamod = bltmod;
        custom->bltbmod = bltmod;
        custom->bltdmod = bltmod;
        custom->bltsize = bltsize;

        mask >>= 1;
      }
    }

    /* Clear working area. */
    {
      APTR data = buffer + bltstart;

      WaitBlitter();

      custom->bltcon0 = (DEST | A_TO_D);
      custom->bltcon1 = 0;
      custom->bltadat = 0;
      custom->bltdmod = bltmod;
      custom->bltdpt = data;
      custom->bltsize = bltsize;
    }

    item++;
  }
}

static void Render() {
  BitmapClear(screen0, DEPTH);

  {
    // LONG lines = ReadLineCounter();
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 8;
    UpdateObjectTransformation(cube);
    UpdateFaceVisibility(cube);
    UpdateVertexVisibility(cube);
    TransformVertices(cube);
    // Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  {
    // LONG lines = ReadLineCounter();
    SortFaces(cube);
    // Log("sort: %ld\n", ReadLineCounter() - lines);
  }

  {
    // LONG lines = ReadLineCounter();
    DrawObject(cube, screen0->planes, buffer->planes[0]);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();

  {
    WORD n = DEPTH;

    while (--n >= 0)
      CopInsSet32(bplptr[n], screen0->planes[n]);
  }

  swapr(screen0, screen1);
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
