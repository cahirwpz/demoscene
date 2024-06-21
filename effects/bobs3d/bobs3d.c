#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "3d.h"
#include "fx.h"

#define WIDTH  256
#define HEIGHT 256
#define DEPTH 3

#define TZ (-256)

static Object3D *cube;
static CopListT *cp;
static BitmapT *screen[2];
static CopInsPairT *bplptr;
static int active = 0;

#include "data/flares32.c"
#include "data/pilka.c"

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(80);
  CopWait(cp, Y(-1), 0);
  bplptr = CopSetupBitplanes(cp, screen[1], DEPTH);
  return CopListFinish(cp);
}

static void Init(void) {
  cube = NewObject3D(&pilka);
  cube->translate.z = fx4i(TZ);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR|BM_INTERLEAVED);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR|BM_INTERLEAVED);

  SetupPlayfield(MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  LoadColors(bobs_colors, 0);

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DeleteCopList(cp);
  DisableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
  DeleteObject3D(cube);
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
  void *_objdat = object->objdat;
  short *group = object->vertexGroups;

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
    short i;

    while ((i = *group++)) {
      short *v = (short *)M;
      short *pt = (short *)POINT(i);

      short x = *pt++;
      short y = *pt++;
      short z = *pt++;
      short zp;

      int xy = x * y;
      int xp, yp;

      MULVERTEX1(xp, m0);
      MULVERTEX1(yp, m1);
      MULVERTEX2(zp);

      *pt++ = div16(xp, zp) + WIDTH / 2;  /* div(xp * 256, zp) */
      *pt++ = div16(yp, zp) + HEIGHT / 2; /* div(yp * 256, zp) */
      *pt++ = zp;
    }
  } while (*group);
}

#define BOBW 32
#define BOBH 32

#define OPTIMIZED 1

#if OPTIMIZED
#define SRCROW (512 / 8)
#define DSTROW (256 / 8)
#else
#define SRCROW src->bytesPerRow
#define DSTROW dst->bytesPerRow
#endif

void BlitterOrArea(BitmapT *dst asm("a0"), u_short x asm("d0"), u_short y asm("d1"),
                   const BitmapT *src asm("a1"), u_short sx asm("d2"))
{
  u_short dxo = x & 15;
  u_short width = dxo + BOBW;
  u_short wo = width & 15;
  u_short bytesPerRow = ((width + 15) & ~15) >> 3;
  u_short srcmod = SRCROW - bytesPerRow;
  u_short dstmod = DSTROW - bytesPerRow;
  u_short bltafwm = FirstWordMask[dxo];
  u_short bltalwm = LastWordMask[wo];
  u_short bltshift = rorw(dxo, 4);

  u_int src_start = (sx & ~15) >> 3;
  u_int dst_start = ((x & ~15) >> 3) + y * DSTROW * DEPTH;
  u_short bltsize = (BOBH * DEPTH << 6) | (bytesPerRow >> 1);

  void *srcbpt = src->planes[0] + src_start;
  void *dstbpt = dst->planes[0] + dst_start;

  bool fast = (dxo == 0);

  WaitBlitter();

  if (fast) {
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltcon1 = 0;
    custom->bltafwm = -1;
    custom->bltalwm = -1;
    custom->bltamod = srcmod;
    custom->bltbmod = dstmod;
    custom->bltdmod = dstmod;

    custom->bltapt = srcbpt;
    custom->bltbpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  } else {
    /* AB + C */
    custom->bltcon0 = (SRCB | SRCC | DEST) | (ABC | ABNC | ANBC | NABC | NANBC);
    custom->bltcon1 = bltshift;
    custom->bltadat = -1;
    custom->bltafwm = bltafwm;
    custom->bltalwm = bltalwm;
    custom->bltbmod = srcmod;
    custom->bltcmod = dstmod;
    custom->bltdmod = dstmod;

    custom->bltbpt = srcbpt;
    custom->bltcpt = dstbpt;
    custom->bltdpt = dstbpt;
    custom->bltsize = bltsize;
  }
}

static void DrawObject(Object3D *object, BitmapT *dst) {
  void *_objdat = object->objdat;
  short *group = object->vertexGroups;

#if 0
  short minZ = 32767, maxZ = -32768;
#endif

  do {
    short v;

    while ((v = *group++)) {
      short *data = (short *)VERTEX(v);
      short x = *data++;
      short y = *data++;
      short z = *data++;

      z >>= 4;
      z -= TZ;
      z += 128 - 32;
      z = z + z + z - 32;
      z &= ~31;

#if 0
      if (z < minZ)
        minZ = z;
      if (z > maxZ)
        maxZ = z;
#endif

      BlitterOrArea(dst, x - 16, y - 16, &bobs, z);
    }
  } while (*group);
}

static void BitmapClearI(BitmapT *bm) {
  WaitBlitter();

  custom->bltadat = 0;
  custom->bltcon0 = DEST | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltdmod = 0;
  custom->bltdpt = bm->planes[0];
  custom->bltsize = ((bm->height * bm->depth) << 6) | (bm->bytesPerRow >> 1);
}

PROFILE(TransformObject);
PROFILE(DrawObject);

static void Render(void) {
  BitmapClearI(screen[active]);

  ProfilerStart(TransformObject);
  {
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 12;

    UpdateObjectTransformation(cube);
    TransformVertices(cube);
  }
  ProfilerStop(TransformObject);

  WaitBlitter();

  ProfilerStart(DrawObject);
  {
    DrawObject(cube, screen[active]);
  }
  ProfilerStop(DrawObject);

  TaskWaitVBlank();

  CopUpdateBitplanes(bplptr, screen[active], DEPTH);
  active ^= 1;
}

EFFECT(Bobs3D, NULL, NULL, Init, Kill, Render, NULL);
