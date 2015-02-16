#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "print.h"
#include "file.h"

#define WIDTH 160
#define HEIGHT 100
#define DEPTH 4

static PixmapT *textureHi, *textureLo;
static BitmapT *screen[2];
static UWORD *uvmap;
static UWORD active = 0;
static CopListT *cp;
static CopInsT *bplptr[DEPTH + 1];
static PixmapT *texture;

#define UVMapRenderSize (WIDTH * HEIGHT / 2 * 10 + 2)
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
    /* [0 0 0 0 a0 a1 a2 a3] => [a0 a1 0 0 a2 a3 0 0] */
    *hi++ = ((c & m1) << 4) | ((c & m2) << 2);
    /* [0 0 0 0 b0 b1 b2 b3] => [ 0 0 b0 b1 0 0 b2 b3] */
    *lo++ = ((c & m1) << 2) | (c & m2);
  }

  /* Extra halves for cheap texture motion. */
  memcpy(((APTR)imageHi->pixels) + size, (APTR)imageHi->pixels, size);
  memcpy(((APTR)imageLo->pixels) + size, (APTR)imageLo->pixels, size);
}

static void MakeUVMapRenderCode() {
  UWORD *code = (APTR)UVMapRender;
  UWORD *data = uvmap;
  WORD n = WIDTH * HEIGHT / 2;

  /* UVMap is pre-scrambled. */
  while (n--) {
    *code++ = 0x1029;  /* 1029 xxxx | move.b xxxx(a1),d0 */
    *code++ = *data++;
    *code++ = 0x802a;  /* 802a yyyy | or.b   yyyy(a2),d0 */
    *code++ = *data++;
    *code++ = 0x10c0;  /* 10c0      | move.b d0,(a0)+    */
  }

  *code++ = 0x4e75; /* rts */
}

static void Load() {
  screen[0] = NewBitmap(WIDTH * 2, HEIGHT, DEPTH + 1); /* add mask bpl */
  screen[1] = NewBitmap(WIDTH * 2, HEIGHT, DEPTH);

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
  APTR *bpl;
} c2p = { 256, NULL };

#define BPLSIZE ((WIDTH * 2) * HEIGHT / 8) /* 4000 bytes */
#define BLTSIZE ((WIDTH / 2) * HEIGHT)     /* 8000 bytes */

static void ChunkyToPlanar() {
  register APTR *bpl asm("a0") = c2p.bpl;

  switch (c2p.phase) {
    case 0:
      /* Initialize chunky to planar. */
      custom->bltamod = 2;
      custom->bltbmod = 2;
      custom->bltdmod = 0;
      custom->bltcdat = 0xF0F0;
      custom->bltafwm = -1;
      custom->bltalwm = -1;

      /* Swap 4x2, pass 1, high-bits. */
      custom->bltapt = bpl[0];
      custom->bltbpt = bpl[0] + 2;
      custom->bltdpt = bpl[3];

      /* (a & 0xF0F0) | ((b >> 4) & ~0xF0F0) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom->bltcon1 = 4 << BSHIFTSHIFT;
      custom->bltsize = 1 | ((BLTSIZE / 8) << 6);
      break;

    case 1:
      custom->bltsize = 1 | ((BLTSIZE / 8) << 6); /* overall size: BLTSIZE / 2 bytes */
      break;

    case 2:
      /* Swap 4x2, pass 2, low-bits. */
      custom->bltapt = bpl[2] - 4;
      custom->bltbpt = bpl[2] - 2;
      custom->bltdpt = bpl[3] - 2;

      /* ((a << 4) & 0xF0F0) | (b & ~0xF0F0) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (4 << ASHIFTSHIFT);
      custom->bltcon1 = BLITREVERSE;
      custom->bltsize = 1 | ((BLTSIZE / 8) << 6);
      break;

    case 3:
      custom->bltsize = 1 | ((BLTSIZE / 8) << 6); /* overall size: BLTSIZE / 2 bytes */
      break;

    case 4:
      CopInsSet32(bplptr[0], bpl[2]);
      CopInsSet32(bplptr[1], bpl[2]);
      CopInsSet32(bplptr[2], bpl[3]);
      CopInsSet32(bplptr[3], bpl[3]);
      break;

    default:
      return;
  }

  c2p.phase++;
}

static void BlitterInterrupt() {
  if (custom->intreqr & INTF_BLIT)
    ChunkyToPlanar();
}

static void MakeCopperList(CopListT *cp) {
  WORD i;

  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(28), WIDTH * 2, HEIGHT * 2);
  CopMakePlayfield(cp, bplptr, screen[active], DEPTH + 1);
  CopLoadColor(cp, 0, 15, 0);
  CopLoadPal(cp, texture->palette, 16);
  for (i = 0; i < HEIGHT * 2; i++) {
    CopWait(cp, Y(i + 28), 0);
    /* Line doubling. */
    CopMove16(cp, bpl1mod, (i & 1) ? 0 : -40);
    CopMove16(cp, bpl2mod, (i & 1) ? 0 : -40);
    /* Alternating shift by one for bitplane data. */
    CopMove16(cp, bplcon1, (i & 1) ? 0x0010 : 0x0021);
  }
  CopEnd(cp);
}

static void Init() {
  UVMapRender = MemAlloc(UVMapRenderSize, MEMF_PUBLIC);
  MakeUVMapRenderCode();

  textureHi = NewPixmap(texture->width, texture->height * 2,
                        PM_CMAP, MEMF_PUBLIC);
  textureLo = NewPixmap(texture->width, texture->height * 2,
                        PM_CMAP, MEMF_PUBLIC);
  PixmapScramble(texture, textureHi, textureLo);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;

  BitmapClear(screen[0], DEPTH);
  BitmapClear(screen[1], DEPTH);

  memset(screen[0]->planes[4], 0x55, screen[0]->bplSize);

  cp = NewCopList(900);
  MakeCopperList(cp);
  CopListActivate(cp);
  CopInsSet32(bplptr[4], screen[0]->planes[4]);

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
  WORD offset = frameCount & 16383;

  /* screen's bitplane #0 & #1 are be used as a chunky buffer */
  {
    UBYTE *txtHi = textureHi->pixels + offset;
    UBYTE *txtLo = textureLo->pixels + offset;

    // LONG lines = ReadLineCounter();
    (*UVMapRender)(screen[active]->planes[0], txtHi, txtLo);
    // Log("uvmap: %ld\n", ReadLineCounter() - lines);
  }

  c2p.phase = 0;
  c2p.bpl = screen[active]->planes;
  ChunkyToPlanar();
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, BlitterInterrupt };
