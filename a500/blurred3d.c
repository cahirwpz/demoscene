#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "3d.h"
#include "fx.h"
#include "ffp.h"
#include "ilbm.h"
#include "tga.h"
#include "memory.h"

#define WIDTH  176
#define HEIGHT 176
#define DEPTH  4

#define STARTX ((320 - WIDTH) / 2)
#define STARTY ((256 - HEIGHT * 5 / 4) / 2)

static Mesh3D *mesh;
static Object3D *cube;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen0, *screen1;
static BitmapT *scratchpad;
static BitmapT *carry;
static PixmapT *gradient;

static void Load() {
  mesh = LoadLWO("data/szescian.lwo", SPFlt(93));
  gradient = LoadTGA("data/blurred3d-pal.tga", PM_RGB4, MEMF_PUBLIC);
  CalculateEdges(mesh);
  CalculateFaceNormals(mesh);
}

static void UnLoad() {
  DeletePixmap(gradient);
  DeleteMesh3D(mesh);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakeDispWin(cp, X(STARTX), Y(STARTY), WIDTH, HEIGHT + HEIGHT / 4);
  CopMakePlayfield(cp, bplptr, screen0, DEPTH);

  {
    WORD *pixels = gradient->pixels;
    WORD i, j;

    for (i = 0; i < HEIGHT / 8; i++) {
      CopWait(cp, Y(STARTY + i * 8 - 1), 0xde);
      for (j = 0; j < 16; j++) {
        CopSetRGB(cp, j, *pixels++);
      }
    }

    CopWait(cp, Y(STARTY + HEIGHT - 1), 0xde);
    CopMove16(cp, bpl1mod, - WIDTH * 5 / 8);
    CopMove16(cp, bpl2mod, - WIDTH * 5 / 8);

    for (i = 0; i < HEIGHT / 16; i++) {
      CopWait(cp, Y(STARTY + HEIGHT + i * 4 - 1), 0xde);
      for (j = 0; j < 16; j++) {
        CopSetRGB(cp, j, *pixels++);
      }
    }
  }

  CopEnd(cp);
}

static void Init() {
  cube = NewObject3D(mesh);
  cube->translate.z = fx4i(-250);

  screen0 = NewBitmap(WIDTH, HEIGHT + 1, DEPTH);
  screen1 = NewBitmap(WIDTH, HEIGHT + 1, DEPTH);
  carry = NewBitmap(WIDTH, HEIGHT, 2);
  scratchpad = NewBitmap(WIDTH, HEIGHT, 2);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_BLITHOG;

  cp = NewCopList(80 + gradient->height * (gradient->width + 1));
  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
}

static void Kill() {
  DeleteBitmap(screen0);
  DeleteBitmap(screen1);
  DeleteBitmap(scratchpad);
  DeleteBitmap(carry);
  DeleteCopList(cp);
  DeleteObject3D(cube);
}

#define MULVERTEX(D) {                   \
  WORD t0 = (*v++) + y;                  \
  WORD t1 = (*v++) + x;                  \
  LONG t2 = (*v++) * z;                  \
  WORD t3 = (*v++);                      \
  D = normfx(t0 * t1 + t2 - x * y) + t3; \
}

static __regargs void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  WORD *src = (WORD *)object->mesh->vertex;
  WORD *dst = (WORD *)object->vertex;
  register WORD n asm("d7") = object->mesh->vertices - 1;

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
    WORD *v = (WORD *)M;
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;
    WORD xp, yp, zp;

    MULVERTEX(xp);
    MULVERTEX(yp);
    MULVERTEX(zp);

    *dst++ = div16(xp << 8, zp) + WIDTH / 2;
    *dst++ = div16(yp << 8, zp) + HEIGHT / 2;
    *dst++ = zp;
  } while (--n != -1);
}

#define MoveLong(reg, hi, lo) \
    *(ULONG *)(&custom->reg) = (((hi) << 16) | (lo))

static __regargs void DrawLine(WORD x0, WORD y0, WORD x1, WORD y1) {
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
    APTR src = scratchpad->planes[0];
    WORD start = ((y0 * WIDTH / 8) + (x0 >> 3)) & ~1;
    APTR dst = src + start;
    UWORD bltcon0 = rorw(x0 & 15, 4) | LINE_EOR;
    UWORD bltamod = derr - dmax;
    UWORD bltbmod = 2 * dmin;
    UWORD bltsize = (dmax << 6) + 66;
    APTR bltapt = (APTR)(LONG)derr;

    WaitBlitter();

    custom->bltadat = 0x8000;
    custom->bltbdat = -1;
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

static void DrawObject(Object3D *object) {
  APTR outbuf = carry->planes[0];
  APTR tmpbuf = scratchpad->planes[0];
  Point3D *point = object->vertex;
  BYTE *faceFlags = object->faceFlags;
  IndexListT **faceEdges = object->mesh->faceEdge;
  IndexListT **faces = object->mesh->face;
  IndexListT *face;

  custom->bltafwm = -1;
  custom->bltalwm = -1;

  while ((face = *faces++)) {
    IndexListT *faceEdge = *faceEdges++;

    if (*faceFlags++) {
      UWORD bltmod, bltsize;
      WORD bltstart, bltend;

      /* Estimate the size of rectangle that contains a face. */
      {
        WORD *i = face->indices;
        Point3D *p = &point[*i++];
        WORD minX = p->x;
        WORD minY = p->y;
        WORD maxX = minX; 
        WORD maxY = minY;
        WORD n = face->count - 2;

        do {
          p = &point[*i++];

          if (p->x < minX)
            minX = p->x;
          else if (p->x > maxX)
            maxX = p->x;

          if (p->y < minY)
            minY = p->y;
          else if (p->y > maxY)
            maxY = p->y;
        } while (--n != -1);

        /* Align to word boundary. */
        minX &= ~15;
        maxX += 16; /* to avoid case where a line is on right edge */
        maxX &= ~15;

        {
          WORD w = maxX - minX;
          WORD h = maxY - minY + 1;

          bltstart = (minX >> 3) + minY * WIDTH / 8;
          bltend = (maxX >> 3) + maxY * WIDTH / 8 - 2;
          bltsize = (h << 6) | (w >> 4);
          bltmod = (WIDTH / 8) - (w >> 3);
        }
      }

      /* Draw face. */
      {
        EdgeT *edges = object->mesh->edge;
        WORD m = faceEdge->count;
        WORD *i = faceEdge->indices;

        while (--m >= 0) {
          WORD *edge = (WORD *)&edges[*i++];

          WORD *p0 = (APTR)point + *edge++;
          WORD *p1 = (APTR)point + *edge++;

          WORD x0 = *p0++;
          WORD y0 = *p0++;
          WORD x1 = *p1++;
          WORD y1 = *p1++;

          if (y0 > y1) {
            swapr(x0, x1);
            swapr(y0, y1);
          }

          DrawLine(x0, y0, x1, y1);
        }
      }

      /* Fill face. */
      {
        APTR src = tmpbuf + bltend;

        WaitBlitter();

        custom->bltcon0 = (SRCA | DEST) | A_TO_D;
        custom->bltcon1 = BLITREVERSE | FILL_XOR;
        custom->bltapt = src;
        custom->bltdpt = src;
        custom->bltamod = bltmod;
        custom->bltdmod = bltmod;
        custom->bltsize = bltsize;
      }

      /* Copy filled face to outbuf. */
      {
        APTR src = tmpbuf + bltstart;
        APTR dst = outbuf + bltstart;

        WaitBlitter();

        custom->bltcon0 = (SRCA | SRCB | DEST) | A_XOR_B;
        custom->bltcon1 = 0;
        custom->bltapt = src;
        custom->bltbpt = dst;
        custom->bltdpt = dst;
        custom->bltamod = bltmod;
        custom->bltbmod = bltmod;
        custom->bltdmod = bltmod;
        custom->bltsize = bltsize;
      }

      /* Clear working area. */
      {
        APTR data = tmpbuf + bltstart;

        WaitBlitter();

        custom->bltcon0 = (DEST | A_TO_D);
        custom->bltcon1 = 0;
        custom->bltadat = 0;
        custom->bltdpt = data;
        custom->bltdmod = bltmod;
        custom->bltsize = bltsize;
      }
    }
  }
}

static __regargs void BitmapDecSaturatedFast(BitmapT *dstbm, BitmapT *srcbm) {
  APTR borrow0 = carry->planes[0];
  APTR borrow1 = carry->planes[1];
  APTR *srcbpl = srcbm->planes;
  APTR *dstbpl = dstbm->planes;
  APTR src = *srcbpl++;
  APTR dst = *dstbpl++;
  WORD n = DEPTH - 1;

  WaitBlitter();
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbdat = -1;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltapt = src;
  custom->bltdpt = borrow0;
  custom->bltcon0 = HALF_SUB_BORROW & ~SRCB;
  custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);

  WaitBlitter();
  custom->bltapt = src;
  custom->bltdpt = dst;
  custom->bltcon0 = HALF_SUB & ~SRCB;
  custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);

  while (--n >= 0) {
    src = *srcbpl++;
    dst = *dstbpl++;

    WaitBlitter();
    custom->bltapt = src;
    custom->bltbpt = borrow0;
    custom->bltdpt = borrow1;
    custom->bltcon0 = HALF_SUB_BORROW;
    custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);

    WaitBlitter();
    custom->bltapt = src;
    custom->bltbpt = borrow0;
    custom->bltdpt = dst;
    custom->bltcon0 = HALF_SUB;
    custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);

    swapr(borrow0, borrow1);
  }

  dstbpl = dstbm->planes;
  n = DEPTH;

  while (--n >= 0) {
    dst = *dstbpl++;

    WaitBlitter();
    custom->bltapt = dst;
    custom->bltbpt = borrow0;
    custom->bltdpt = dst;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_AND_NOT_B;
    custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);
  }
}

static __regargs void BitmapIncSaturatedFast(BitmapT *dstbm, BitmapT *srcbm) {
  APTR carry0 = carry->planes[0];
  APTR carry1 = carry->planes[1];
  APTR *srcbpl = srcbm->planes;
  APTR *dstbpl = dstbm->planes;
  WORD n = DEPTH;

  /* Only pixels set to one in carry[0] will be incremented. */
  
  WaitBlitter();
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  while (--n >= 0) {
    APTR src = *srcbpl++;
    APTR dst = *dstbpl++;

    WaitBlitter();
    custom->bltapt = src;
    custom->bltbpt = carry0;
    custom->bltdpt = carry1;
    custom->bltcon0 = HALF_ADDER_CARRY;
    custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);

    WaitBlitter();
    custom->bltapt = src;
    custom->bltbpt = carry0;
    custom->bltdpt = dst;
    custom->bltcon0 = HALF_ADDER;
    custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);

    swapr(carry0, carry1);
  }

  dstbpl = dstbm->planes;
  n = DEPTH;

  while (--n >= 0) {
    APTR dst = *dstbpl++;

    WaitBlitter();
    custom->bltapt = dst;
    custom->bltbpt = carry0;
    custom->bltdpt = dst;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);
  }
}

static void RenderObject3D() {
  BlitterClearSync(carry, 0);

  {
    // LONG lines = ReadLineCounter();
    cube->rotate.x = cube->rotate.y = cube->rotate.z =
      ReadFrameCounter() * 6;

    UpdateObjectTransformation(cube);
    UpdateFaceVisibility(cube);
    TransformVertices(cube);
    // Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  {
    // LONG lines = ReadLineCounter();
    DrawObject(cube);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }
}

static WORD iterCount = 0;

static void Render() {
  // LONG lines = ReadLineCounter();
  BitmapT *source = screen1;

  if (iterCount++ & 1) {
    BitmapDecSaturatedFast(screen0, screen1);
    source = screen0;
  }

  RenderObject3D();

  BitmapIncSaturatedFast(screen0, source);

  // Log("blurred3d: %ld\n", ReadLineCounter() - lines);

  WaitVBlank();
  {
    WORD n = DEPTH;

    while (--n >= 0)
      CopInsSet32(bplptr[n], screen0->planes[n]);
  }
  swapr(screen0, screen1);
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
