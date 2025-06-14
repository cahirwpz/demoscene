#include <effect.h>
#include <blitter.h>
#include <color.h>
#include <sprite.h>
#include <copper.h>
#include <pixmap.h>
#include <system/interrupt.h>
#include <system/memory.h>

#define WIDTH 80
#define HEIGHT 64
#define DEPTH 4
#define OPTIMIZED 1

static __code BitmapT *screen[2];
static __code u_short active = 0;
static __code u_short *shademap;
static __code u_short *chunky[2];
static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code CopInsPairT *sprptr;
static __code u_short *texture;

#include "data/torus-map.c"
#include "data/torus-light.c"
#include "data/texture.c"
#include "data/skull.c"

#if OPTIMIZED
#define UVMapRenderSize (WIDTH * HEIGHT * 8 + 2)
static void (*UVMapRender)(u_short *chunky asm("a0"), u_short *texture asm("a1"),
                           u_short *shades asm("a2"));

static void MakeUVMapRenderCode(void) {
  u_short *code = (void *)UVMapRender;
  u_short *data = uvmap;
  u_char *lmap = light.pixels;
  short n = WIDTH * HEIGHT;

  while (--n >= 0) {
    u_short x = *data++;
    u_short y = *lmap++;
    if (x & 1) {
      *code++ = 0x5488; /* 5488      | addq.l #2,%a0 */
    } else {
      *code++ = 0x3029;
      *code++ = x;      /* 3029 xxxx | move.w xxxx(a1),d0 */
      *code++ = 0x30f2;
      *code++ = y;      /* 30f2 00yy | move.w yy(a2,d0.w),(a0)+ */
    }
  }

  *code++ = 0x4e75; /* rts */
}
#else
static void UVMapRender(u_short *chunky, u_short *texture, u_short *shade) {
  u_short *data = uvmap;
  u_char *lmap = light_pixels;
  short n = WIDTH * HEIGHT;

  while (--n >= 0) {
    short uv = *data++;
    short l = *lmap++;

    if (uv & 1) {
      chunky++;
    } else {
      short c = *(short *)((void *)texture + uv) | l;
      *chunky++ = *(u_short *)((void *)shade + c);
    }
  }
}
#endif

#define F(a, b, c, d) (((a) << 12) | ((b) << 8) | ((c) << 4) | (d))

static u_short bluetab[16] = {
  F(0,0,0,0), F(0,0,0,3), F(0,0,3,0), F(0,0,3,3),
  F(0,3,0,0), F(0,3,0,3), F(0,3,3,0), F(0,3,3,3),
  F(3,0,0,0), F(3,0,0,3), F(3,0,3,0), F(3,0,3,3),
  F(3,3,0,0), F(3,3,0,3), F(3,3,3,0), F(3,3,3,3),
};

static u_short greentab[16] = {
  F(0,0,0,0), F(0,0,0,4), F(0,0,4,0), F(0,0,4,4),
  F(0,4,0,0), F(0,4,0,4), F(0,4,4,0), F(0,4,4,4),
  F(4,0,0,0), F(4,0,0,4), F(4,0,4,0), F(4,0,4,4),
  F(4,4,0,0), F(4,4,0,4), F(4,4,4,0), F(4,4,4,4),
};

static u_short redtab[16] = {
  F(0,0,0,0), F(0,0,0,8), F(0,0,8,0), F(0,0,8,8),
  F(0,8,0,0), F(0,8,0,8), F(0,8,8,0), F(0,8,8,8),
  F(8,0,0,0), F(8,0,0,8), F(8,0,8,0), F(8,0,8,8),
  F(8,8,0,0), F(8,8,0,8), F(8,8,8,0), F(8,8,8,8),
};

#undef F

static inline u_int PixelScramble(u_short data) {
  short ri = (data >> 8) & 15;
  short gi = (data >> 4) & 15;
  short bi = data & 15;

  /* [-- -- -- -- 11 10  9  8  7  6  5  4  3  2  1  0] */
  /* [-- -- -- -- r0 r1 r2 r3 g0 g1 g2 g3 b0 b1 b2 b3] */
  /* [11  7  3  3 10  6  2  2  9  5  1  1  8  4  0  0] */
  /* [r0 g0 b0 b0 r1 g1 b1 b1 r2 g2 b2 b2 r3 g3 b3 b3] */
  return getword(redtab, ri) + getword(greentab, gi) + getword(bluetab, bi);
}

static void Load(void) {
  {
    u_char *src = light_pixels;
    u_char *dst = light_pixels;
    short n = WIDTH * HEIGHT;

    while (--n >= 0)
      *dst++ = (*src++ >> 2) & 0x3E;
  }

  shademap = MemAlloc(32 * sizeof(u_short) * texture_colors_count, MEMF_PUBLIC);
  {
    u_short *cp = texture_colors;
    u_short *dst = shademap;
    short n = texture_colors_count;
    short i;

    while (--n >= 0) {
      u_short c = *cp++;
      for (i = 0; i < 16; i++) {
        *dst++ = PixelScramble(ColorTransition(0, c, i));
      }
      for (i = 0; i < 16; i++) {
        *dst++ = PixelScramble(ColorTransition(c, 0xfff, i));
      }
    }
  }

  texture = MemAlloc(128 * 128 * 2 * sizeof(u_short), MEMF_PUBLIC);
  {
    u_char *src = texture_raw_pixels;
    u_short *dst = texture;
    short n = 128 * 128;

    while (--n >= 0)
      *dst++ = *src++ << 6;

    /* Extra half for cheap texture motion. */
    memcpy((void *)texture + 32768, texture, 32768);
  }
}

static struct {
  short phase;
  void **bpl;
  void *chunky;
} c2p = { 256, NULL, NULL };

#define BPLSIZE ((WIDTH * 4) * HEIGHT / 8) /* 2560 bytes */
#define BLTSIZE ((WIDTH * 4) * HEIGHT / 2) /* 10240 bytes */

static void ChunkyToPlanar(void) {
  void *src = c2p.chunky;
  void *dst = c2p.chunky + BLTSIZE;
  void **bpl = c2p.bpl;

  switch (c2p.phase) {
    case 0:
      /* Initialize chunky to planar. */
      custom->bltamod = 4;
      custom->bltbmod = 4;
      custom->bltdmod = 4;
      custom->bltcdat = 0x00FF;
      custom->bltafwm = -1;
      custom->bltalwm = -1;

      /* Swap 8x4, pass 1. */
      custom->bltapt = src + 4;
      custom->bltbpt = src;
      custom->bltdpt = dst;

      /* ((a >> 8) & 0x00FF) | (b & ~0x00FF) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ANBC | ABNC | NABNC) | ASHIFT(8);
      custom->bltcon1 = 0;
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 1:
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 2:
      /* Swap 8x4, pass 2. */
      custom->bltapt = src + BLTSIZE - 6;
      custom->bltbpt = src + BLTSIZE - 2;
      custom->bltdpt = dst + BLTSIZE - 2;

      /* ((a << 8) & ~0x00FF) | (b & 0x00FF) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABNC | ANBNC | ABC | NABC) | ASHIFT(8);
      custom->bltcon1 = BLITREVERSE;
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 3:
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 4:
      custom->bltamod = 6;
      custom->bltbmod = 6;
      custom->bltdmod = 0;
      custom->bltcdat = 0x0F0F;

      custom->bltapt = dst + 2;
      custom->bltbpt = dst;
      custom->bltdpt = bpl[0];

      /* ((a >> 4) & 0x0F0F) | (b & ~0x0F0F) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ANBC | ABNC | NABNC) | ASHIFT(4);
      custom->bltcon1 = 0;
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 5:
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 6:
      custom->bltapt = dst + 6;
      custom->bltbpt = dst + 4;
      custom->bltdpt = bpl[2];
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 7:
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 8:
      custom->bltapt = dst + BLTSIZE - 8;
      custom->bltbpt = dst + BLTSIZE - 6;
      custom->bltdpt = bpl[1] + BPLSIZE - 2;

      /* ((a << 8) & ~0x0F0F) | (b & 0x0F0F) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABNC | ANBNC | ABC | NABC) | ASHIFT(4);
      custom->bltcon1 = BLITREVERSE;
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 9:
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 10:
      custom->bltapt = dst + BLTSIZE - 4;
      custom->bltbpt = dst + BLTSIZE - 2;
      custom->bltdpt = bpl[3] + BPLSIZE - 2;
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 11:
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 12:
      CopInsSet32(&bplptr[0], bpl[3]);
      CopInsSet32(&bplptr[1], bpl[2]);
      CopInsSet32(&bplptr[2], bpl[1]);
      CopInsSet32(&bplptr[3], bpl[0]);
      break;

    default:
      break;
  }

  c2p.phase++;

  ClearIRQ(INTF_BLIT);
}

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(1200);
  short i;

  bplptr = CopSetupBitplanes(cp, screen[active], DEPTH + (IsAGA() ? 2 : 0));
  sprptr = CopSetupSprites(cp);
  CopLoadColor(cp, 0, 15, 0);
  for (i = 0; i < HEIGHT * 4; i++) {
    CopWaitSafe(cp, Y(i), HP(0));
    /* Line quadrupling. */
    CopMove16(cp, bpl1mod, ((i & 3) != 3) ? -40 : 0);
    CopMove16(cp, bpl2mod, ((i & 3) != 3) ? -40 : 0);
    /* Alternating shift by one for bitplane data. */
    CopMove16(cp, bplcon1, (i & 1) ? 0x0022 : 0x0000);
  }
  return CopListFinish(cp);
}

static void Init(void) {
  screen[0] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH + (IsAGA() ? 2 : 0), 0);
  screen[1] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH + (IsAGA() ? 2 : 0), 0);

  chunky[0] = MemAlloc((WIDTH * 4) * HEIGHT, MEMF_CHIP|MEMF_CLEAR);
  chunky[1] = MemAlloc((WIDTH * 4) * HEIGHT, MEMF_CHIP|MEMF_CLEAR);

#if OPTIMIZED
  UVMapRender = MemAlloc(UVMapRenderSize, MEMF_PUBLIC);
  MakeUVMapRenderCode();
#endif

  EnableDMA(DMAF_BLITTER);

  BitmapClear(screen[0]);
  BitmapClear(screen[1]);

  SetupPlayfield(MODE_HAM, IsAGA() ? 6 : 7, X(0), Y(0), WIDTH * 4 + 2, HEIGHT * 4);

  if (IsAGA()) {
    memset(screen[0]->planes[4], 0x77, WIDTH * 4 * HEIGHT / 8);
    memset(screen[1]->planes[4], 0x77, WIDTH * 4 * HEIGHT / 8);
    memset(screen[0]->planes[5], 0xcc, WIDTH * 4 * HEIGHT / 8);
    memset(screen[1]->planes[5], 0xcc, WIDTH * 4 * HEIGHT / 8);
  } else {
    custom->bpldat[4] = 0x7777; // rgbb: 0111
    custom->bpldat[5] = 0xcccc; // rgbb: 1100
  }

  LoadColors(sprite_colors, 16);

  cp = MakeCopperList();
  CopListActivate(cp);

  {
    short i;
    for (i = 0; i < 8; i++) {
      CopInsSetSprite(&sprptr[i], skull[i]);
      SpriteUpdatePos(skull[i], X(16 * (i >> 1) + 256), Y(0));
    }
  }

  EnableDMA(DMAF_RASTER|DMAF_SPRITE);

  SetIntVector(INTB_BLIT, (IntHandlerT)ChunkyToPlanar, NULL);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  ResetSprites();
  CopperStop();

  DisableINT(INTF_BLIT);
  ResetIntVector(INTB_BLIT);

  DeleteCopList(cp);
#if OPTIMIZED
  MemFree(UVMapRender);
#endif

  MemFree(chunky[0]);
  MemFree(chunky[1]);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

PROFILE(UVLight);

static void Render(void) {
  ProfilerStart(UVLight);
#if OPTIMIZED
  (*UVMapRender)(chunky[active], &texture[frameCount & 16383], shademap);
#else
  UVMapRender(chunky[active], &texture[frameCount & 16383], shademap);
#endif
  ProfilerStop(UVLight);

  c2p.phase = 0;
  c2p.chunky = chunky[active];
  c2p.bpl = screen[active]->planes;
  ChunkyToPlanar();
  active ^= 1;
}

EFFECT(UVLight, Load, NULL, Init, Kill, Render, NULL);
