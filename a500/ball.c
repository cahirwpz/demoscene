#include "startup.h"
#include "bltop.h"
#include "coplist.h"
#include "memory.h"
#include "tga.h"
#include "file.h"
#include "sprite.h"
#include "ilbm.h"
#include "fx.h"

#define S_WIDTH 320
#define S_HEIGHT 256
#define S_DEPTH 4

#define WIDTH 64
#define HEIGHT 64

static PixmapT *textureHi, *textureLo;
static PixmapT *chunky;
static BitmapT *bitmap;
static SpriteT *sprite[2][4];
static SpriteT *nullspr;
static CopInsT *sprptr[8];

static BitmapT *background;
static UWORD *uvmap;
static UWORD active = 0;
static CopListT *cp;
static PixmapT *texture;

#define UVMapRenderSize (WIDTH * HEIGHT / 2 * 10 + 2)
void (*UVMapRender)(UBYTE *chunky asm("a0"),
                    UBYTE *textureHi asm("a1"),
                    UBYTE *textureLo asm("a2"));

static __regargs void PixmapToTexture(PixmapT *image, PixmapT *imageHi, PixmapT *imageLo)
{
  UBYTE *data = image->pixels;
  LONG size = image->width * image->height;
  /* Extra halves for cheap texture motion. */
  UWORD *hi0 = imageHi->pixels;
  UWORD *hi1 = imageHi->pixels + size;
  UWORD *lo0 = imageLo->pixels;
  UWORD *lo1 = imageLo->pixels + size;
  WORD n = size / 2;

  while (--n >= 0) {
    UBYTE a = *data++;
    UWORD b = ((a << 8) | (a << 1)) & 0xAAAA;
    /* [a0 b0 a1 b1 a2 b2 a3 b3] => [a0 -- a2 -- a1 -- a3 --] */
    *hi0++ = b;
    *hi1++ = b;
    /* [a0 b0 a1 b1 a2 b2 a3 b3] => [-- b0 -- b2 -- b1 -- b3] */
    *lo0++ = b >> 1;
    *lo1++ = b >> 1;
  }
}

static void MakeUVMapRenderCode() {
  UWORD *code = (APTR)UVMapRender;
  UWORD *data = uvmap;
  WORD n = WIDTH * HEIGHT / 2;
  WORD uv;

  while (n--) {
    if ((uv = *data++) >= 0) {
      *code++ = 0x1029;  /* 1029 xxxx | move.b xxxx(a1),d0 */
      *code++ = uv;
    } else {
      *code++ = 0x7000;  /* 7000      | moveq  #0,d0 */
    }
    if ((uv = *data++) >= 0) {
      *code++ = 0x802a;  /* 802a yyyy | or.b   yyyy(a2),d0 */
      *code++ = uv;
    }
    *code++ = 0x10c0;    /* 10c0      | move.b d0,(a0)+    */
  }

  *code++ = 0x4e75; /* rts */
}

static void Load() {
  background = LoadILBM("data/dragon-bg.ilbm");
  texture = LoadTGA("data/texture-15.tga", PM_CMAP4, MEMF_PUBLIC);
  uvmap = ReadFile("data/ball.bin", MEMF_PUBLIC);
}

static void UnLoad() {
  MemFreeAuto(uvmap);

  DeletePalette(background->palette);
  DeleteBitmap(background);
  DeletePalette(texture->palette);
  DeletePixmap(texture);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopMakeDispWin(cp, X(0), Y(0), S_WIDTH, S_HEIGHT);
  CopMakePlayfield(cp, NULL, background, S_DEPTH);
  CopLoadPal(cp, background->palette, 0);
  CopLoadPal(cp, texture->palette, 16);
  CopMakeSprites(cp, sprptr, nullspr);
  CopEnd(cp);

  {
    WORD i;

    for (i = 0; i < 4; i++) {
      SpriteT *spr = sprite[active][i];
      CopInsSet32(sprptr[i * 2], spr->data);
      CopInsSet32(sprptr[i * 2 + 1], spr->attached->data);
    }
  }
}

static void Init() {
  bitmap = NewBitmap(WIDTH, HEIGHT, S_DEPTH);
  chunky = NewPixmap(WIDTH, HEIGHT, PM_GRAY4, MEMF_CHIP);

  UVMapRender = MemAlloc(UVMapRenderSize, MEMF_PUBLIC);
  MakeUVMapRenderCode();

  textureHi = NewPixmap(texture->width, texture->height * 2,
                        PM_CMAP, MEMF_PUBLIC);
  textureLo = NewPixmap(texture->width, texture->height * 2,
                        PM_CMAP, MEMF_PUBLIC);

  PixmapScramble_4_1(texture);
  PixmapToTexture(texture, textureHi, textureLo);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER | DMAF_BLITHOG;

  nullspr = NewSprite(0, FALSE);
  {
    WORD i, j;

    for (i = 0; i < 2; i++)
      for (j = 0; j < 4; j++)
        sprite[i][j] = NewSprite(64, TRUE);
  }

  cp = NewCopList(80);
  MakeCopperList(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER | DMAF_SPRITE;
  custom->intena = INTF_SETCLR | INTF_BLIT;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE;
  custom->intena = INTF_BLIT;

  DeleteCopList(cp);
  DeletePixmap(textureHi);
  DeletePixmap(textureLo);
  MemFree(UVMapRender, UVMapRenderSize);

  DeleteSprite(nullspr);
  {
    WORD i, j;

    for (i = 0; i < 2; i++)
      for (j = 0; j < 4; j++)
        DeleteSprite(sprite[i][j]);
  }

  DeletePixmap(chunky);
  DeleteBitmap(bitmap);
}

#define BLTSIZE (WIDTH * HEIGHT / 2)

#if (BLTSIZE / 4) > 1024
#error "blit size too big!"
#endif

static __regargs void ChunkyToPlanar(PixmapT *input, BitmapT *output) {
  APTR planes = output->planes[0];
  APTR chunky = input->pixels;

  /* Swap 8x4, pass 1. */
  {
    WaitBlitter();

    /* (a & 0xFF00) | ((b >> 8) & ~0xFF00) */
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
    custom->bltcon1 = 8 << BSHIFTSHIFT;
    custom->bltafwm = -1;
    custom->bltalwm = -1;
    custom->bltamod = 4;
    custom->bltbmod = 4;
    custom->bltdmod = 4;
    custom->bltcdat = 0xFF00;

    custom->bltapt = chunky;
    custom->bltbpt = chunky + 4;
    custom->bltdpt = planes;
    custom->bltsize = 2 | ((BLTSIZE / 8) << 6);
  }

  /* Swap 8x4, pass 2. */
  {
    WaitBlitter();

    /* ((a << 8) & 0xFF00) | (b & ~0xFF00) */
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (8 << ASHIFTSHIFT);
    custom->bltcon1 = BLITREVERSE;

    custom->bltapt = chunky + BLTSIZE - 6;
    custom->bltbpt = chunky + BLTSIZE - 2;
    custom->bltdpt = planes + BLTSIZE - 2;
    custom->bltsize = 2 | ((BLTSIZE / 8) << 6);
  }

  /* Swap 4x2, pass 1. */
  {
    WaitBlitter();

    /* (a & 0xF0F0) | ((b >> 4) & ~0xF0F0) */
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
    custom->bltcon1 = 4 << BSHIFTSHIFT;
    custom->bltamod = 2;
    custom->bltbmod = 2;
    custom->bltdmod = 2;
    custom->bltcdat = 0xF0F0;

    custom->bltapt = planes;
    custom->bltbpt = planes + 2;
    custom->bltdpt = chunky;
    custom->bltsize = 1 | ((BLTSIZE / 4) << 6);
  }

  /* Swap 4x2, pass 2. */
  {
    WaitBlitter();

    /* ((a << 4) & 0xF0F0) | (b & ~0xF0F0) */
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (4 << ASHIFTSHIFT);
    custom->bltcon1 = BLITREVERSE;

    custom->bltapt = planes + BLTSIZE - 4;
    custom->bltbpt = planes + BLTSIZE - 2;
    custom->bltdpt = chunky + BLTSIZE - 2;
    custom->bltsize = 1 | ((BLTSIZE / 4) << 6);
  }

  /* Swap 2x1, pass 1 & 2. */
  {
    WaitBlitter();

    /* (a & 0xCCCC) | ((b >> 2) & ~0xCCCC) */
    custom->bltamod = 6;
    custom->bltbmod = 6;
    custom->bltdmod = 0;
    custom->bltcdat = 0xCCCC;
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
    custom->bltcon1 = 2 << BSHIFTSHIFT;

    custom->bltapt = chunky;
    custom->bltbpt = chunky + 4;
    custom->bltdpt = planes + BLTSIZE * 3 / 4;
    custom->bltsize = 1 | ((BLTSIZE / 8) << 6);

    WaitBlitter();
    custom->bltapt = chunky + 2;
    custom->bltbpt = chunky + 6;
    custom->bltdpt = planes + BLTSIZE * 2 / 4;
    custom->bltsize = 1 | ((BLTSIZE / 8) << 6);
  }

  /* Swap 2x1, pass 3 & 4. */
  {
    WaitBlitter();

    /* ((a << 2) & 0xCCCC) | (b & ~0xCCCC) */
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (2 << ASHIFTSHIFT);
    custom->bltcon1 = BLITREVERSE;

    custom->bltapt = chunky + BLTSIZE - 8;
    custom->bltbpt = chunky + BLTSIZE - 4;
    custom->bltdpt = planes + BLTSIZE * 2 / 4 - 2;
    custom->bltsize = 1 | ((BLTSIZE / 8) << 6);

    WaitBlitter();
    custom->bltapt = chunky + BLTSIZE - 6;
    custom->bltbpt = chunky + BLTSIZE - 2;
    custom->bltdpt = planes + BLTSIZE * 1 / 4 - 2;
    custom->bltsize = 1 | ((BLTSIZE / 8) << 6);
  }
}

static __regargs void BitmapToSprite(BitmapT *input, SpriteT **sprite) {
  APTR planes = input->planes[0];
  WORD bltsize = (input->height << 6) | 1;
  WORD i = 0;

  WaitBlitter();

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltamod = 6;
  custom->bltdmod = 2;

  for (i = 0; i < 4; i++) {
    SpriteT *spr = *sprite++;

    WaitBlitter();
    custom->bltapt = planes + i * 2;
    custom->bltdpt = &spr->data[2];
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltdpt = &spr->data[3];
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltdpt = &spr->attached->data[2];
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltdpt = &spr->attached->data[3];
    custom->bltsize = bltsize;
  }
}

static __regargs void PositionSprite(SpriteT **sprite, WORD xo, WORD yo) {
  WORD x = X((S_WIDTH - WIDTH) / 2) + xo;
  WORD y = Y((S_HEIGHT - HEIGHT) / 2) + yo;
  CopInsT **ptr = sprptr;
  WORD n = 4;

  while (--n >= 0) {
    SpriteT *spr = *sprite++;

    UpdateSpritePos(spr, x, y);

    CopInsSet32(*ptr++, spr->data);
    CopInsSet32(*ptr++, spr->attached->data);

    x += 16;
  }
}

static void Render() {
  WORD xo = normfx(SIN(frameCount * 16) * 128);
  WORD yo = normfx(COS(frameCount * 16) * 100);
  WORD offset = ((64 - xo) + (64 - yo) * 128) & 16383;

  {
    UBYTE *txtHi = textureHi->pixels + offset;
    UBYTE *txtLo = textureLo->pixels + offset;

    // LONG lines = ReadLineCounter();
    (*UVMapRender)(chunky->pixels, txtHi, txtLo);
    ChunkyToPlanar(chunky, bitmap);
    BitmapToSprite(bitmap, sprite[active]);
    PositionSprite(sprite[active], xo / 2, yo / 2);
    // Log("uvmap: %ld\n", ReadLineCounter() - lines);
  }

  WaitVBlank();
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
