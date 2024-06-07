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
static BitmapT *buffer;
static int active = 0;

#include "data/flatshade-pal.c"
#include "data/pilka.c"

static Mesh3D *mesh = &pilka;

static void Init(void) {
  cube = NewObject3D(mesh);
  cube->translate.z = fx4i(-250);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  buffer = NewBitmap(WIDTH, HEIGHT, 1, 0);

  SetupPlayfield(MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  LoadColors(flatshade_colors, 0);

  cp = NewCopList(80);
  bplptr = CopSetupBitplanes(cp, screen[0], DEPTH);
  CopListFinish(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(buffer);
  DeleteCopList(cp);
  DeleteObject3D(cube);
}

#define MULVERTEX1(D, E) {               \
  short t0 = (*v++) + y;                  \
  short t1 = (*v++) + x;                  \
  int t2 = (*v++) * z;                  \
  v++;                                   \
  D = ((t0 * t1 + t2 - x * y) >> 4) + E; \
}

#define MULVERTEX2(D) {                  \
  short t0 = (*v++) + y;                  \
  short t1 = (*v++) + x;                  \
  int t2 = (*v++) * z;                  \
  short t3 = (*v++);                      \
  D = normfx(t0 * t1 + t2 - x * y) + t3; \
}

static void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  short *mx = (short *)M;
  short *src = (short *)object->point;
  short *dst = (short *)object->vertex;
  register short n asm("d7") = object->vertices - 1;

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
    if (((Point3D *)dst)->flags) {
      short x = *src++;
      short y = *src++;
      short z = *src++;
      short *v = mx;
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

static void DrawObject(Object3D *object, CustomPtrT custom_ asm("a6")) {
  short **vertexIndexList = object->faceVertexIndexList;
  SortItemT *item = object->visibleFace;
  short n = object->visibleFaces;
  void *vertex = object->vertex;
  void *temp = buffer->planes[0];

  custom_->bltafwm = -1;
  custom_->bltalwm = -1;

  for (; --n >= 0; item++) {
    short faceIndex = item->index;
    short *vertexIndex = vertexIndexList[faceIndex];
    char color = vertexIndex[FV_FLAGS];

    short minX, minY, maxX, maxY;

    /* Draw edges and calculate bounding box. */
    {
      register short m asm("d7") = vertexIndex[FV_COUNT] - 1;
      short *ptr = (short *)(vertex + (short)(vertexIndex[m] << 3));
      short xs = *ptr++;
      short ys = *ptr++;
      short xe, ye;

      minX = xs;
      minY = ys;
      maxX = xs;
      maxY = ys;

      do {
        ptr = (short *)(vertex + (short)(*vertexIndex++ << 3));
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
          short x0, y0, dx, dy, derr;
          u_short bltcon1;

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
            short start = ((y0 << 5) + (x0 >> 3)) & ~1;
            void *dst = temp + start;
            u_short bltcon0 = rorw(x0 & 15, 4) | BC0F_LINE_EOR;
            u_short bltamod = derr - dx;
            u_short bltbmod = dy + dy;
            u_short bltsize = (dx << 6) + 66;

            WaitBlitter();

            custom_->bltbdat = 0xffff;
            custom_->bltadat = 0x8000;
            custom_->bltcon0 = bltcon0;
            custom_->bltcon1 = bltcon1;
            custom_->bltcpt = dst;
            custom_->bltapt = (void *)(int)derr;
            custom_->bltdpt = temp;
            custom_->bltcmod = WIDTH / 8;
            custom_->bltbmod = bltbmod;
            custom_->bltamod = bltamod;
            custom_->bltdmod = WIDTH / 8;
            custom_->bltsize = bltsize;
          }
        }

        xs = xe; ys = ye;
      } while (--m != -1);
    }

    {
      short bltstart, bltend;
      u_short bltmod, bltsize;

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

      /* Fill face. */
      {
        void *src = temp + bltend;

        WaitBlitter();

        custom_->bltcon0 = (SRCA | DEST) | A_TO_D;
        custom_->bltcon1 = BLITREVERSE | FILL_XOR;
        custom_->bltapt = src;
        custom_->bltdpt = src;
        custom_->bltamod = bltmod;
        custom_->bltbmod = bltmod;
        custom_->bltdmod = bltmod;
        custom_->bltsize = bltsize;
      }

      /* Copy filled face to screen. */
      {
        void **dstbpl = &screen[active]->planes[DEPTH];
        void *src = temp + bltstart;
        char mask = 1 << (DEPTH - 1);
        short n = DEPTH;

        while (--n >= 0) {
          void *dst = *(--dstbpl) + bltstart;
          u_short bltcon0;

          if (color & mask)
            bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
           else
            bltcon0 = (SRCA | SRCB | DEST) | (NABC | NABNC);

          WaitBlitter();

          custom_->bltcon0 = bltcon0;
          custom_->bltcon1 = 0;
          custom_->bltapt = src;
          custom_->bltbpt = dst;
          custom_->bltdpt = dst;
          custom_->bltsize = bltsize;

          mask >>= 1;
        }
      }

      /* Clear working area. */
      {
        void *data = temp + bltstart;

        WaitBlitter();

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
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 8;
    UpdateObjectTransformation(cube);
    UpdateFaceVisibility(cube);
    UpdateVertexVisibility(cube);
    TransformVertices(cube);
    SortFaces(cube);
  }
  ProfilerStop(Transform); // Average: 163

  ProfilerStart(Draw);
  {
    DrawObject(cube, custom);
  }
  ProfilerStop(Draw); // Average: 671

  CopUpdateBitplanes(bplptr, screen[active], DEPTH);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(FlatShade, NULL, NULL, Init, Kill, Render, NULL);
