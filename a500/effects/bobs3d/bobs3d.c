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
#define DEPTH 3

#define TZ (-256)

static Mesh3D *mesh;
static Object3D *cube;
static CopListT *cp;
static BitmapT *screen0, *screen1;
static BitmapT *bobs;
static CopInsT *bplptr[DEPTH];

static void Load() {
  bobs = LoadILBMCustom("flares32.ilbm", 
                        BM_DISPLAYABLE | BM_INTERLEAVED | BM_LOAD_PALETTE);
  mesh = LoadMesh3D("pilka.3d", SPFlt(50));
}

static void UnLoad() {
  DeletePalette(bobs->palette);
  DeleteBitmap(bobs);
  DeleteMesh3D(mesh);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(32), Y(0), WIDTH, HEIGHT);
  CopWait(cp, Y(-1), 0);
  CopSetupBitplanes(cp, bplptr, screen1, DEPTH);
  CopLoadPal(cp, bobs->palette, 0);
  CopEnd(cp);
}

static void Init() {
  cube = NewObject3D(mesh);
  cube->translate.z = fx4i(TZ);

  screen0 = NewBitmapCustom(WIDTH, HEIGHT, DEPTH,
                            BM_DISPLAYABLE | BM_INTERLEAVED);
  screen1 = NewBitmapCustom(WIDTH, HEIGHT, DEPTH,
                            BM_DISPLAYABLE | BM_INTERLEAVED);

  cp = NewCopList(80);
  MakeCopperList(cp);
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill() {
  DeleteCopList(cp);
  DisableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
  DeleteBitmap(screen0);
  DeleteBitmap(screen1);
  DeleteObject3D(cube);
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
  WORD *src = (WORD *)object->mesh->vertex;
  WORD *dst = (WORD *)object->vertex;
  register WORD n asm("d7") = object->mesh->vertices;

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
    WORD x = *src++;
    WORD y = *src++;
    WORD z = *src++;
    LONG xy = x * y;
    LONG xp, yp;
    WORD zp;
    WORD *v = (WORD *)M;

    MULVERTEX1(xp, m0);
    MULVERTEX1(yp, m1);
    MULVERTEX2(zp);

    *dst++ = div16(xp, zp) + WIDTH / 2;  /* div(xp * 256, zp) */
    *dst++ = div16(yp, zp) + HEIGHT / 2; /* div(yp * 256, zp) */
    *dst++ = zp;

    src++;
    dst++;
  } while (--n > 0);
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

void BlitterOrArea(BitmapT *dst asm("a0"), UWORD x asm("d0"), UWORD y asm("d1"),
                   BitmapT *src asm("a1"), UWORD sx asm("d2"))
{
  UWORD dxo = x & 15;
  UWORD width = dxo + BOBW;
  UWORD wo = width & 15;
  UWORD bytesPerRow = ((width + 15) & ~15) >> 3;
  UWORD srcmod = SRCROW - bytesPerRow;
  UWORD dstmod = DSTROW - bytesPerRow;
  UWORD bltafwm = FirstWordMask[dxo];
  UWORD bltalwm = LastWordMask[wo];
  UWORD bltshift = rorw(dxo, 4);

  ULONG src_start = (sx & ~15) >> 3;
  ULONG dst_start = ((x & ~15) >> 3) + y * DSTROW * DEPTH;
  UWORD bltsize = (BOBH * DEPTH << 6) | (bytesPerRow >> 1);

  APTR srcbpt = src->planes[0] + src_start;
  APTR dstbpt = dst->planes[0] + dst_start;

  BOOL fast = (dxo == 0);

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

static __regargs void DrawObject(Object3D *object, BitmapT *dst) {
  WORD *data = (WORD *)object->vertex;
  register WORD n asm("d7") = object->mesh->vertices;

#if 0
  WORD minZ = 32767, maxZ = -32768;
#endif

  do {
    WORD x = *data++;
    WORD y = *data++;
    WORD z = *data++;

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

    BlitterOrArea(dst, x - 16, y - 16, bobs, z);

    data++;
  } while (--n > 0);
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

static void Render() {
  BitmapClearI(screen0);

  {
    LONG lines = ReadLineCounter();
    cube->rotate.x = cube->rotate.y = cube->rotate.z = frameCount * 12;

    UpdateObjectTransformation(cube);
    TransformVertices(cube);
    Log("transform: %ld\n", ReadLineCounter() - lines);
  }

  WaitBlitter();

  {
    LONG lines = ReadLineCounter();
    DrawObject(cube, screen0);
    Log("draw: %ld\n", ReadLineCounter() - lines);
  }

  TaskWait(VBlankEvent);

  CopUpdateBitplanes(bplptr, screen0, DEPTH);
  swapr(screen0, screen1);
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
