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
#define DEPTH  4

static PaletteT *palette;
static Mesh3D *mesh;
static Object3D *cube;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static BitmapT *screen0, *screen1;
static BitmapT *buffer;

static void Load() {
  mesh = LoadMesh3D("codi.3d", SPFlt(384));
  // mesh = LoadMesh3D("cube.3d", SPFlt(50));
  CalculateFaceNormals(mesh);
  palette = LoadPalette("flatshade-pal.ilbm");
}

static void UnLoad() {
  DeletePalette(palette);
  DeleteMesh3D(mesh);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  CopSetupBitplanes(cp, bplptr, screen0, DEPTH);
  CopLoadPal(cp, palette, 0);
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
  EnableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG);
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

static void DrawObject(Object3D *object, volatile struct Custom* const custom asm("a6")) {
  IndexListT **faces = object->mesh->face;
  SortItemT *item = object->visibleFace;
  BYTE *faceFlags = object->faceFlags;
  WORD n = object->visibleFaces;
  APTR point = object->vertex;
  APTR temp = buffer->planes[0];

  custom->bltafwm = -1;
  custom->bltalwm = -1;

  for (; --n >= 0; item++) {
    WORD index = item->index;
    IndexListT *face = faces[index];

    WORD minX, minY, maxX, maxY;

    /* Draw edges and calculate bounding box. */
    {
      WORD *i = face->indices;
      register WORD m asm("d7") = face->count - 1;
      WORD *ptr = (WORD *)(point + (WORD)(i[m] << 3));
      WORD xs = *ptr++;
      WORD ys = *ptr++;
      WORD xe, ye;

      minX = xs;
      minY = ys;
      maxX = xs;
      maxY = ys;

      do {
        ptr = (WORD *)(point + (WORD)(*i++ << 3));
        xe = *ptr++;
        ye = *ptr++;

        /* Estimate the size of rectangle that contains a face. */
        if (xe < minX)
          minX = xe;
        else if (xe > maxX)
          maxX = xe;
        if (ye < minY)
          minY = ye;
        else if (ye > maxY)
          maxY = ye;

        /* Draw an edge. */
        {
          WORD x0, y0, dx, dy, derr;
          UWORD bltcon1;

          if (ys < ye) {
            x0 = xs; y0 = ys;
            dx = xe - xs;
            dy = ye - ys;
          } else {
            x0 = xe; y0 = ye;
            dx = xs - xe;
            dy = ys - ye;
          }

          if (dx < 0) {
            dx = -dx;
            if (dx >= dy) {
              bltcon1 = AUL | SUD | LINEMODE | ONEDOT;
            } else {
              bltcon1 = SUL | LINEMODE | ONEDOT;
              swapr(dx, dy);
            }
          } else {
            if (dx >= dy) {
              bltcon1 = SUD | LINEMODE | ONEDOT;
            } else {
              bltcon1 = LINEMODE | ONEDOT;
              swapr(dx, dy);
            }
          }

          derr = dy + dy - dx;
          if (derr < 0)
            bltcon1 |= SIGNFLAG;

          {
            WORD start = ((y0 << 5) + (x0 >> 3)) & ~1;
            APTR dst = temp + start;
            UWORD bltcon0 = rorw(x0 & 15, 4) | BC0F_LINE_EOR;
            UWORD bltamod = derr - dx;
            UWORD bltbmod = dy + dy;
            UWORD bltsize = (dx << 6) + 66;

            WaitBlitter();

            MoveLong(bltbdat, 0xffff, 0x8000);

            custom->bltcon0 = bltcon0;
            custom->bltcon1 = bltcon1;
            custom->bltcpt = dst;
            custom->bltapt = (APTR)(LONG)derr;
            custom->bltdpt = temp;
            custom->bltcmod = WIDTH / 8;
            custom->bltbmod = bltbmod;
            custom->bltamod = bltamod;
            custom->bltdmod = WIDTH / 8;
            custom->bltsize = bltsize;
          }
        }

        xs = xe; ys = ye;
      } while (--m != -1);
    }

    {
      WORD bltstart, bltend;
      UWORD bltmod, bltsize;

      /* Align to word boundary. */
      minX = (minX & ~15) >> 3;
      /* to avoid case where a line is on right edge */
      maxX = ((maxX + 16) & ~15) >> 3;

      {
        WORD w = maxX - minX;
        WORD h = maxY - minY + 1;

        bltstart = minX + minY * (WIDTH / 8);
        bltend = maxX + maxY * (WIDTH / 8) - 2;
        bltsize = (h << 6) | (w >> 1);
        bltmod = (WIDTH / 8) - w;
      }

      /* Fill face. */
      {
        APTR src = temp + bltend;

        WaitBlitter();

        custom->bltcon0 = (SRCA | DEST) | A_TO_D;
        custom->bltcon1 = BLITREVERSE | FILL_XOR;
        custom->bltapt = src;
        custom->bltdpt = src;
        custom->bltamod = bltmod;
        custom->bltbmod = bltmod;
        custom->bltdmod = bltmod;
        custom->bltsize = bltsize;
      }

      /* Copy filled face to screen. */
      {
        APTR *screen = &screen0->planes[DEPTH];
        APTR src = temp + bltstart;
        BYTE mask = 1 << (DEPTH - 1);
        BYTE color = faceFlags[index];
        WORD n = DEPTH;

        while (--n >= 0) {
          APTR dst = *(--screen) + bltstart;
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
          custom->bltsize = bltsize;

          mask >>= 1;
        }
      }

      /* Clear working area. */
      {
        APTR data = temp + bltstart;

        WaitBlitter();

        custom->bltcon0 = (DEST | A_TO_D);
        custom->bltadat = 0;
        custom->bltdpt = data;
        custom->bltsize = bltsize;
      }
    }
  }
}

static void Render() {
  BitmapClear(screen0);

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
    DrawObject(cube, custom);
    // Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  CopUpdateBitplanes(bplptr, screen0, DEPTH);
  TaskWait(VBlankEvent);
  swapr(screen0, screen1);
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
