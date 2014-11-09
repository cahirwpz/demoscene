#include "blitter.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "print.h"
#include "file.h"

#include "startup.h"

#define WIDTH 160
#define HEIGHT 100
#define DEPTH 5

static PixmapT *chunky[2];
static PixmapT *textureHi, *textureLo;
static BitmapT *screen[2];
static UWORD *uvmap;
static UWORD active = 0;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];
static PixmapT *texture;

extern APTR UVMapRenderTemplate[5];
#define UVMapRenderSize \
  (WIDTH * HEIGHT / 2 * sizeof(UVMapRenderTemplate) + 2)
void (*UVMapRender)(UBYTE *chunky asm("a0"),
                    UBYTE *textureHi asm("a1"),
                    UBYTE *textureLo asm("a2"));

static void PixmapScramble(PixmapT *image, PixmapT *imageHi, PixmapT *imageLo)
{
  ULONG *data = image->pixels;
  ULONG *hi = imageHi->pixels;
  ULONG *lo = imageLo->pixels;
  LONG size = image->width * image->height;
  LONG n = size / 4;
  register ULONG m1 asm("d6") = 0x0c0c0c0c;
  register ULONG m2 asm("d7") = 0x03030303;

  while (--n >= 0) {
    ULONG c = *data++;
    /* [0 0 0 0 a0 a1 a2 a3] => [a2 a3 0 0 a0 a1 0 0] */
    *hi++ = (c & m1) | ((c & m2) << 6);
    /* [0 0 0 0 a0 a1 a2 a3] => [ 0 0 a2 a3 0 0 a0 a1] */
    *lo++ = ((c & m1) >> 2) | ((c & m2) << 4);
  }

  /* Extra halves for cheap texture motion. */
  memcpy(((APTR)imageHi->pixels) + size, (APTR)imageHi->pixels, size);
  memcpy(((APTR)imageLo->pixels) + size, (APTR)imageLo->pixels, size);
}

static void Load() {
  screen[0] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH);
  screen[1] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH);

  texture = LoadTGA("data/texture-16-1.tga", PM_CMAP, MEMF_PUBLIC);
  uvmap = ReadFile("data/uvmap.bin", MEMF_PUBLIC);
}

static void UnLoad() {
  MemFreeAuto(uvmap);
  DeletePalette(texture->palette);
  DeletePixmap(texture);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static struct {
  WORD phase;
  WORD active;
} c2p = { 5, 0 };

static void ChunkyToPlanar() {
  BitmapT *dst = screen[c2p.active];
  PixmapT *src = chunky[c2p.active];

  switch (c2p.phase) {
    case 0:
      /* Initialize chunky to planar. */
      custom->bltamod = 2;
      custom->bltbmod = 2;
      custom->bltdmod = 0;
      custom->bltcdat = 0xf0f0;
      custom->bltafwm = -1;
      custom->bltalwm = -1;

      /* Swap 4x2, pass 1. */
      custom->bltapt = src->pixels;
      custom->bltbpt = src->pixels + 2;
      custom->bltdpt = dst->planes[0];

      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom->bltcon1 = 4 << BSHIFTSHIFT;
      custom->bltsize = 1;
      break;

    case 1:
      custom->bltsize = 1 | (976 << 6);
      break;

    case 2:
      /* Swap 4x2, pass 2. */
      // custom->bltapt = src->pixels + WIDTH * HEIGHT / 2;
      // custom->bltbpt = src->pixels + WIDTH * HEIGHT / 2 + 2;
      custom->bltdpt = dst->planes[2] + WIDTH * HEIGHT / 4;

      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (4 << ASHIFTSHIFT);
      custom->bltcon1 = BLITREVERSE;
      custom->bltsize = 1;
      break;

    case 3:
      custom->bltsize = 1 | (977 << 6);
      break;

    case 4:
      CopInsSet32(bplptr[0], dst->planes[0]);
      CopInsSet32(bplptr[1], dst->planes[0]);
      CopInsSet32(bplptr[2], dst->planes[2]);
      CopInsSet32(bplptr[3], dst->planes[2]);
      CopInsSet32(bplptr[4], dst->planes[4]);
      break;

    default:
      return;
  }

  c2p.phase++;
}

static void BlitterInterrupt() {
  if (custom->intreqr & INTF_BLIT) {
    ChunkyToPlanar();
  }
}

static void MakeCopperList(CopListT *cp) {
  WORD i;

  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(28), WIDTH * 2, HEIGHT * 2);
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH);
  CopLoadColor(cp, 0, 15, 0);
  CopLoadPal(cp, texture->palette, 16);
  for (i = 0; i < HEIGHT * 2; i++) {
    CopWaitMask(cp, Y(i + 28), 0, 0xff, 0);
    CopMove16(cp, bplcon1, (i & 1) ? 0x0021 : 0x0010);
    CopMove16(cp, bpl1mod, (i & 1) ? -40 : 0);
    CopMove16(cp, bpl2mod, (i & 1) ? -40 : 0);
  }
  CopEnd(cp);
}

static void MakeUVMapRenderCode() {
  UWORD *code = (APTR)UVMapRender;
  UWORD *tmpl = (APTR)UVMapRenderTemplate;
  UWORD *data = uvmap;
  WORD n = WIDTH * HEIGHT / 2;

  /* UVMap is pre-scrambled. */
  while (n--) {
    *code++ = tmpl[0];
    *code++ = *data++;
    *code++ = tmpl[2];
    *code++ = *data++;
    *code++ = tmpl[4];
  }

  *code++ = 0x4e75; /* return from subroutine instruction */
}

static void Init() {
  static PixmapT recycled[2];

  chunky[0] = &recycled[0];
  chunky[1] = &recycled[1];

  InitSharedPixmap(chunky[0], WIDTH, HEIGHT, PM_GRAY4, screen[0]->planes[1]);
  InitSharedPixmap(chunky[1], WIDTH, HEIGHT, PM_GRAY4, screen[1]->planes[1]);

  UVMapRender = MemAlloc(UVMapRenderSize, MEMF_PUBLIC);
  textureHi = NewPixmap(texture->width, texture->height * 2,
                        PM_CMAP, MEMF_PUBLIC);
  textureLo = NewPixmap(texture->width, texture->height * 2,
                        PM_CMAP, MEMF_PUBLIC);

  MakeUVMapRenderCode();
  PixmapScramble(texture, textureHi, textureLo);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;

  ITER(i, 0, 4, BlitterClearSync(screen[0], i));
  ITER(i, 0, 4, BlitterClearSync(screen[1], i));

  memset(screen[0]->planes[4], 0x55, WIDTH * HEIGHT * 4 / 8);
  memset(screen[1]->planes[4], 0x55, WIDTH * HEIGHT * 4 / 8);

  cp = NewCopList(1024);

  MakeCopperList(cp);
  CopListActivate(cp);
  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;
  custom->intena = INTF_SETCLR | INTF_BLIT;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER;
  custom->intena = INTF_BLIT;

  DeleteCopList(cp);
  DeletePixmap(textureHi);
  DeletePixmap(textureLo);
  MemFree(UVMapRender, UVMapRenderSize);
}

static void Render() {
  UBYTE *txtHi = textureHi->pixels + (frameCount & 16383);
  UBYTE *txtLo = textureLo->pixels + (frameCount & 16383);

  {
    // LONG lines = ReadLineCounter();
    (*UVMapRender)(chunky[active]->pixels, txtHi, txtLo);
    // Log("uvmap: %ld\n", ReadLineCounter() - lines);
  }

  c2p.phase = 0;
  c2p.active = active;
  ChunkyToPlanar();
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, BlitterInterrupt };
