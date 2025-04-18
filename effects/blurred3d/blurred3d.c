#include <effect.h>
#include <3d.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <system/memory.h>

#define WIDTH  176
#define HEIGHT 176
#define DEPTH  4

#define STARTX ((320 - WIDTH) / 2)
#define STARTY ((256 - HEIGHT * 5 / 4) / 2)

static Object3D *cube;
static CopListT *cp;
static CopInsPairT *bplptr;
static BitmapT *screen[2];
static BitmapT *scratchpad;
static BitmapT *carry;
static int active = 0;

#include "data/blurred3d-pal.c"
#include "data/szescian.c"

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(80 + gradient.height * (gradient.width + 1));
  bplptr = CopSetupBitplanes(cp, screen[0], DEPTH);

  {
    short *pixels = gradient.pixels;
    short i, j;

    for (i = 0; i < HEIGHT / 8; i++) {
      CopWait(cp, Y(STARTY + i * 8 - 1), 0xde);
      for (j = 0; j < 16; j++) {
        CopSetColor(cp, j, *pixels++);
      }
    }

    CopWait(cp, Y(STARTY + HEIGHT - 1), 0xde);
    CopMove16(cp, bpl1mod, - WIDTH * 5 / 8);
    CopMove16(cp, bpl2mod, - WIDTH * 5 / 8);

    for (i = 0; i < HEIGHT / 16; i++) {
      CopWait(cp, Y(STARTY + HEIGHT + i * 4 - 1), 0xde);
      for (j = 0; j < 16; j++) {
        CopSetColor(cp, j, *pixels++);
      }
    }
  }

  return CopListFinish(cp);
}

static void Init(void) {
  cube = NewObject3D(&szescian);
  cube->translate.z = fx4i(-250);

  screen[0] = NewBitmap(WIDTH, HEIGHT + 1, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH, HEIGHT + 1, DEPTH, BM_CLEAR);
  carry = NewBitmap(WIDTH, HEIGHT, 2, 0);
  scratchpad = NewBitmap(WIDTH, HEIGHT, 2, 0);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  SetupPlayfield(MODE_LORES, DEPTH, X(STARTX), Y(STARTY), WIDTH, HEIGHT * 5 / 4);

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  DisableDMA(DMAF_RASTER|DMAF_BLITTER);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteBitmap(scratchpad);
  DeleteBitmap(carry);
  DeleteCopList(cp);
  DeleteObject3D(cube);
}

#define MULVERTEX(D) {                          \
  short t0 = (*v++) + y;                        \
  short t1 = (*v++) + x;                        \
  int t2 = (*v++) * z;                          \
  short t3 = (*v++);                            \
  D = normfx(t0 * t1 + t2 - x * y) + t3;        \
}

static void TransformVertices(Object3D *object) {
  Matrix3D *M = &object->objectToWorld;
  void *_objdat = object->objdat;
  short *group = object->vertexGroups;

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
    short i;

    while ((i = *group++)) {
      short *pt = (short *)POINT(i);
      short *v = (short *)M;
      short x, y, z;
      int xp, yp, zp;

      x = *pt++;
      y = *pt++;
      z = *pt++;

      MULVERTEX(xp);
      MULVERTEX(yp);
      MULVERTEX(zp);

      *pt++ = div16(xp << 8, zp) + WIDTH / 2;
      *pt++ = div16(yp << 8, zp) + HEIGHT / 2;
      *pt++ = zp;
    }
  } while (*group);
}

static void DrawLine(short x0, short y0, short x1, short y1) {
  short dmax = x1 - x0;
  short dmin = y1 - y0;
  short derr;
  u_short bltcon1 = LINEMODE | ONEDOT;

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
    void *src = scratchpad->planes[0];
    short start = ((y0 * WIDTH / 8) + (x0 >> 3)) & ~1;
    void *dst = src + start;
    u_short bltcon0 = rorw(x0 & 15, 4) | BC0F_LINE_EOR;
    u_short bltamod = derr - dmax;
    u_short bltbmod = 2 * dmin;
    u_short bltsize = (dmax << 6) + 66;
    void *bltapt = (void *)(int)derr;

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

static void DrawObject(Object3D *object, CustomPtrT custom_ asm("a6")) {
  void *outbuf = carry->planes[0];
  void *tmpbuf = scratchpad->planes[0];

  void *_objdat = object->objdat;
  short *group = object->faceGroups;

  custom_->bltafwm = -1;
  custom_->bltalwm = -1;

  do {
    short f;

    while ((f = *group++)) {
      if (FACE(f)->flags >= 0) {
        short minX = 32767;
        short minY = 32767;
        short maxX = -32768;
        short maxY = -32768;

        {
          register short *index asm("a3") = (short *)(FACE(f)->indices);
          short n = FACE(f)->count - 1;

          do {
            /* Calculate area. */
            {
              short i = *index++; /* vertex */
              short x = VERTEX(i)->x;
              short y = VERTEX(i)->y;

              if (x < minX)
                minX = x;
              if (x > maxX)
                maxX = x;

              if (y < minY)
                minY = y;
              if (y > maxY)
                maxY = y;
            }

            /* Draw edge. */
            {
              short j = *index++; /* edge */
              short x0, y0, x1, y1, i;

              i = EDGE(j)->point[0];
              x0 = VERTEX(i)->x;
              y0 = VERTEX(i)->y;

              i = EDGE(j)->point[1];
              x1 = VERTEX(i)->x;
              y1 = VERTEX(i)->y;

              if (y0 > y1) {
                swapr(x0, x1);
                swapr(y0, y1);
              }

              DrawLine(x0, y0, x1, y1);
            }
          } while (--n != -1);
        }

        /* Estimate the size of rectangle that contains a face. */
        {
          u_short bltmod, bltsize;
          short bltstart, bltend;

          /* Align to word boundary. */
          minX &= ~15;
          maxX += 16; /* to avoid case where a line is on right edge */
          maxX &= ~15;

          {
            short w = maxX - minX;
            short h = maxY - minY + 1;

            bltstart = (minX >> 3) + minY * WIDTH / 8;
            bltend = (maxX >> 3) + maxY * WIDTH / 8 - 2;
            bltsize = (h << 6) | (w >> 4);
            bltmod = (WIDTH / 8) - (w >> 3);
          }

          /* Fill face. */
          {
            void *src = tmpbuf + bltend;

            _WaitBlitter(custom_);

            custom_->bltcon0 = (SRCA | DEST) | A_TO_D;
            custom_->bltcon1 = BLITREVERSE | FILL_XOR;
            custom_->bltapt = src;
            custom_->bltdpt = src;
            custom_->bltamod = bltmod;
            custom_->bltdmod = bltmod;
            custom_->bltsize = bltsize;
          }

          /* Copy filled face to outbuf. */
          {
            void *src = tmpbuf + bltstart;
            void *dst = outbuf + bltstart;

            _WaitBlitter(custom_);

            custom_->bltcon0 = (SRCA | SRCB | DEST) | A_XOR_B;
            custom_->bltcon1 = 0;
            custom_->bltapt = src;
            custom_->bltbpt = dst;
            custom_->bltdpt = dst;
            custom_->bltamod = bltmod;
            custom_->bltbmod = bltmod;
            custom_->bltdmod = bltmod;
            custom_->bltsize = bltsize;
          }

          /* Clear working area. */
          {
            void *data = tmpbuf + bltstart;

            _WaitBlitter(custom_);

            custom_->bltcon0 = (DEST | A_TO_D);
            custom_->bltcon1 = 0;
            custom_->bltadat = 0;
            custom_->bltdpt = data;
            custom_->bltdmod = bltmod;
            custom_->bltsize = bltsize;
          }
        }
      }
    }
  } while (*group);
}

static void BitmapDecSaturatedFast(BitmapT *dstbm, BitmapT *srcbm) {
  void *borrow0 = carry->planes[0];
  void *borrow1 = carry->planes[1];
  void **srcbpl = srcbm->planes;
  void **dstbpl = dstbm->planes;
  void *src = *srcbpl++;
  void *dst = *dstbpl++;
  short n = DEPTH - 1;

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

static void BitmapIncSaturatedFast(BitmapT *dstbm, BitmapT *srcbm) {
  void *carry0 = carry->planes[0];
  void *carry1 = carry->planes[1];
  void **srcbpl = srcbm->planes;
  void **dstbpl = dstbm->planes;
  short n = DEPTH;

  /* Only pixels set to one in carry[0] will be incremented. */
  
  WaitBlitter();
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  while (--n >= 0) {
    void *src = *srcbpl++;
    void *dst = *dstbpl++;

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
    void *dst = *dstbpl++;

    WaitBlitter();
    custom->bltapt = dst;
    custom->bltbpt = carry0;
    custom->bltdpt = dst;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltsize = (HEIGHT << 6) | (WIDTH >> 4);
  }
}

PROFILE(UpdateGeometry);
PROFILE(DrawObject);

static void RenderObject3D(void) {
  BlitterClear(carry, 0);

  ProfilerStart(UpdateGeometry);
  {
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 6;

    UpdateObjectTransformation(cube);
    UpdateFaceVisibility(cube);
    TransformVertices(cube);
  }
  ProfilerStop(UpdateGeometry);

  ProfilerStart(DrawObject);
  {
    DrawObject(cube, custom);
  }
  ProfilerStop(DrawObject);
}

static short iterCount = 0;

static void Render(void) {
  BitmapT *source = screen[active ^ 1];

  if (iterCount++ & 1) {
    BitmapDecSaturatedFast(screen[active], screen[active ^ 1]);
    source = screen[active];
  }

  RenderObject3D();

  BitmapIncSaturatedFast(screen[active], source);

  {
    short n = DEPTH;

    while (--n >= 0)
      CopInsSet32(&bplptr[n], screen[active]->planes[n]);
  }

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Blurred3D, NULL, NULL, Init, Kill, Render, NULL);
