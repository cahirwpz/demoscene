#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <sprite.h>
#include <system/memory.h>

#define S_WIDTH 320
#define S_HEIGHT 256
#define S_DEPTH 4

#define WIDTH 64
#define HEIGHT 64

static PixmapT *textureHi, *textureLo;
static PixmapT *chunky;
static BitmapT *bitmap;
static SprDataT *sprdat;
static SpriteT sprite[2][8];

#include "data/dragon-bg.c"
#include "data/texture-15.c"
#include "data/ball.c"

static short active = 0;
static CopListT *cp[2];

#define UVMapRenderSize (WIDTH * HEIGHT / 2 * 10 + 2)
void (*UVMapRender)(u_char *chunky asm("a0"),
                    u_char *textureHi asm("a1"),
                    u_char *textureLo asm("a2"));

static void PixmapToTexture(const PixmapT *image,
                            PixmapT *imageHi, PixmapT *imageLo)
{
  u_char *data = image->pixels;
  int size = image->width * image->height;
  /* Extra halves for cheap texture motion. */
  u_short *hi0 = imageHi->pixels;
  u_short *hi1 = imageHi->pixels + size;
  u_short *lo0 = imageLo->pixels;
  u_short *lo1 = imageLo->pixels + size;
  short n = size / 2;

  while (--n >= 0) {
    u_char a = *data++;
    u_short b = ((a << 8) | (a << 1)) & 0xAAAA;
    /* [a0 b0 a1 b1 a2 b2 a3 b3] => [a0 -- a2 -- a1 -- a3 --] */
    *hi0++ = b;
    *hi1++ = b;
    /* [a0 b0 a1 b1 a2 b2 a3 b3] => [-- b0 -- b2 -- b1 -- b3] */
    *lo0++ = b >> 1;
    *lo1++ = b >> 1;
  }
}

static void MakeUVMapRenderCode(void) {
  u_short *code = (void *)UVMapRender;
  u_short *data = uvmap;
  short n = WIDTH * HEIGHT / 2;
  short uv;

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

static CopListT *MakeCopperList(int active) {
  CopListT *cp = NewCopList(80);
  CopInsPairT *sprptr = CopSetupSprites(cp);
  short i;

  CopSetupBitplanes(cp, &background, S_DEPTH);
  for (i = 0; i < 8; i++)
    CopInsSetSprite(&sprptr[i], &sprite[active][i]);
  return CopListFinish(cp);
}

static void Init(void) {
  bitmap = NewBitmap(WIDTH, HEIGHT, S_DEPTH, BM_CLEAR);
  chunky = NewPixmap(WIDTH, HEIGHT, PM_CMAP4, MEMF_CHIP);

  UVMapRender = MemAlloc(UVMapRenderSize, MEMF_PUBLIC);
  MakeUVMapRenderCode();

  textureHi = NewPixmap(texture.width, texture.height * 2,
                        PM_CMAP8, MEMF_PUBLIC);
  textureLo = NewPixmap(texture.width, texture.height * 2,
                        PM_CMAP8, MEMF_PUBLIC);

  PixmapScramble_4_1(&texture);
  PixmapToTexture(&texture, textureHi, textureLo);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  sprdat = MemAlloc(SprDataSize(64, 2) * 8 * 2, MEMF_CHIP|MEMF_CLEAR);

  {
    SprDataT *dat = sprdat;
    short i, j;

    for (i = 0; i < 2; i++)
      for (j = 0; j < 8; j++) {
        MakeSprite(&dat, 64, j & 1, &sprite[i][j]);
        EndSprite(&dat);
      }
  }

  SetupPlayfield(MODE_LORES, S_DEPTH, X(0), Y(0), S_WIDTH, S_HEIGHT);
  LoadColors(background_colors, 0);
  LoadColors(texture_colors, 16);

  cp[0] = MakeCopperList(0);
  cp[1] = MakeCopperList(1);
  CopListActivate(cp[0]);

  EnableDMA(DMAF_RASTER | DMAF_SPRITE);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);

  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
  DeletePixmap(textureHi);
  DeletePixmap(textureLo);
  MemFree(UVMapRender);
  MemFree(sprdat);

  DeletePixmap(chunky);
  DeleteBitmap(bitmap);
}

#define BLTSIZE (WIDTH * HEIGHT / 2)

#if (BLTSIZE / 4) > 1024
#error "blit size too big!"
#endif

static void ChunkyToPlanar(PixmapT *input, BitmapT *output) {
  void *planes = output->planes[0];
  void *chunky = input->pixels;

  /* Swap 8x4, pass 1. */
  {
    WaitBlitter();

    /* (a & 0xFF00) | ((b >> 8) & ~0xFF00) */
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
    custom->bltcon1 = BSHIFT(8);
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
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(8);
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
    custom->bltcon1 = BSHIFT(4);
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
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(4);
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
    custom->bltcon1 = BSHIFT(2);

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
    custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(2);
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

static void BitmapToSprite(BitmapT *input, SpriteT sprite[8]) {
  void *planes = input->planes[0];
  short bltsize = (input->height << 6) | 1;
  short i = 0;

  WaitBlitter();

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltcon0 = (SRCA | DEST) | A_TO_D;
  custom->bltcon1 = 0;
  custom->bltamod = 6;
  custom->bltdmod = 2;

  for (i = 0; i < 4; i++) {
    SprDataT *sprdat0 = (sprite++)->sprdat;
    SprDataT *sprdat1 = (sprite++)->sprdat;

    WaitBlitter();
    custom->bltapt = planes + i * 2;
    custom->bltdpt = &sprdat0->data[0][0];
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltdpt = &sprdat0->data[0][1];
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltdpt = &sprdat1->data[0][0];
    custom->bltsize = bltsize;

    WaitBlitter();
    custom->bltdpt = &sprdat1->data[0][1];
    custom->bltsize = bltsize;
  }
}

static void PositionSprite(SpriteT sprite[8], short xo, short yo) {
  short x = X((S_WIDTH - WIDTH) / 2) + xo;
  short y = Y((S_HEIGHT - HEIGHT) / 2) + yo;
  short n = 4;

  while (--n >= 0) {
    SpriteT *spr0 = sprite++;
    SpriteT *spr1 = sprite++;

    SpriteUpdatePos(spr0, x, y);
    SpriteUpdatePos(spr1, x, y);

    x += 16;
  }
}

PROFILE(UVMapRender);

static void Render(void) {
  short xo = normfx(SIN(frameCount * 16) * 128);
  short yo = normfx(COS(frameCount * 16) * 100);
  short offset = ((64 - xo) + (64 - yo) * 128) & 16383;
  u_char *txtHi = textureHi->pixels + offset;
  u_char *txtLo = textureLo->pixels + offset;

  ProfilerStart(UVMapRender);
  {
    (*UVMapRender)(chunky->pixels, txtHi, txtLo);
    ChunkyToPlanar(chunky, bitmap);
    BitmapToSprite(bitmap, sprite[active]);
    PositionSprite(sprite[active], xo / 2, yo / 2);
    CopListActivate(cp[active]);
  }
  ProfilerStop(UVMapRender);

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Ball, NULL, NULL, Init, Kill, Render, NULL);
