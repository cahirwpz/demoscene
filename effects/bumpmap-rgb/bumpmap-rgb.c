#include <effect.h>
#include <blitter.h>
#include <color.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <system/interrupt.h>
#include <system/memory.h>

#define WIDTH 80
#define HEIGHT 64
#define DEPTH 4

static BitmapT *screen[2];
static u_short active = 0;
static u_short *lightmap;
static u_short *shademap;
static u_short *chunky[2];
static CopListT *cp;
static CopInsPairT *bplptr;

#include "data/skulls_128col.c"
#include "data/skulls_map.c"
#include "data/light.c"

static u_short bluetab[16] = {
  0x0000, 0x0003, 0x0030, 0x0033, 0x0300, 0x0303, 0x0330, 0x0333,
  0x3000, 0x3003, 0x3030, 0x3033, 0x3300, 0x3303, 0x3330, 0x3333,
};

static u_short greentab[16] = {
  0x0000, 0x0004, 0x0040, 0x0044, 0x0400, 0x0404, 0x0440, 0x0444,
  0x4000, 0x4004, 0x4040, 0x4044, 0x4400, 0x4404, 0x4440, 0x4444,
};

static u_short redtab[16] = {
  0x0000, 0x0008, 0x0080, 0x0088, 0x0800, 0x0808, 0x0880, 0x0888,
  0x8000, 0x8008, 0x8080, 0x8088, 0x8800, 0x8808, 0x8880, 0x8888,
};

#define BumpMapRenderSize (skulls_width * skulls_height * 8 + 2)
typedef void (*BumpMapRenderT)(u_short *chunky asm("a0"), 
                               u_short *lightmap asm("a1"),
                               u_short *shademap asm("a2"));
static BumpMapRenderT BumpMapRender;

#define BumpMapRenderBackupSize (HEIGHT * 4)
static u_short *BumpMapRenderBackup;

static void MakeBumpMapRenderCode(void) {
  u_short *code = (void *)BumpMapRender;

  /* The image has up to 128 colors. */
  u_char *cmap = skulls.pixels;
  u_short *bmap = bumpmap;

  short n = skulls_width * skulls_height;

  while (n--) {
    /* 3029 xxxx | move.w $xxxx(a1),d0 */
    *code++ = 0x3029;
 
    /* We're going to index 16-bit values, hence multiply by word size. */
    *code++ = *bmap++ * 2;

    /* 30f2 00yy | move.w $yy(a2,d0.w),(a0)+ */
    *code++ = 0x30f2;

    /* 
     * 7-bit value of color index of shaded pixels, multiplied by word size
     * for same reasons as above
     *
     * please note that 8-bit displacement is sign extended, so colors 64..127
     * will generate negative offset, this will be handled by offsetting
     * shadowmap by 128 bytes
     */
    *code++ = (*cmap++ * 2) & 0xff;
  }

  *code++ = 0x4e75; /* rts */
}

static void *BumpMapRenderCrop(void *code, void *backup, short x, short y) {
  u_short *data = code;
  u_short *save = backup;
  short i;

  /* move to selected location */
  data += (y * skulls_width + x) * 4;
  
  /* save the pointer to rendering code */
  code = data;

  /* move to the first instruction that should be skipped */
  data += WIDTH * 4;

  for (i = 0; i < HEIGHT - 1; i++) {
    /* 6000 xxxx | bra.w xxxx */
    *save++ = *data;
    *data++ = 0x6000;
    *save++ = *data;
    *data++ = (skulls_width - WIDTH) * 8 - 2;
    /* move to the next row */
    data += (skulls_width - 1) * 4 + 2;
  }

  *save++ = *data;
  *data++ = 0x4e75; /* rts */
  *save++ = *data;
  *data++ = 0x4e75; /* rts */

  return code;
}

static void BumpMapRenderRestore(void *code, void *backup) {
  u_short *data = code;
  u_short *save = backup;
  short i;

  /* move to the first instruction that should be restored */
  data += WIDTH * 4;

  for (i = 0; i < HEIGHT; i++) {
    *data++ = *save++;
    *data++ = *save++;
    /* move to the next row */
    data += (skulls_width - 1) * 4 + 2;
  }
}

/* RGB12 pixels preprocessing for faster chunky-to-planar */
static void DataScramble(u_short *data, short n) {
  u_char *in = (u_char *)data;
  u_short *out = data;

  while (--n >= 0) {
    short ri = *in++;
    short gi = *in++;
    short bi = gi;

    /* [-- -- -- -- 11 10  9  8  7  6  5  4  3  2  1  0] */
    /* [-- -- -- -- r0 r1 r2 r3 g0 g1 g2 g3 b0 b1 b2 b3] */
    /* [11  7  3  3 10  6  2  2  9  5  1  1  8  4  0  0] */
    /* [r0 g0 b0 b0 r1 g1 b1 b1 r2 g2 b2 b2 r3 g3 b3 b3] */

    gi >>= 4;
    bi &= 15;

    *out++ = getword(redtab, ri) + getword(greentab, gi) + getword(bluetab, bi);
  }
}

#define N 128

static void Load(void) {
  int lightSize = light_width * light_height;

  BumpMapRender = MemAlloc(BumpMapRenderSize, MEMF_PUBLIC);
  MakeBumpMapRenderCode();

  lightmap = MemAlloc(lightSize * sizeof(u_short) * 2, MEMF_PUBLIC);
  {
    /* The light is 128x128 and full 8-bit grayscale */
    u_char *src = light;
    u_short *dst0 = lightmap;
    u_short *dst1 = lightmap + lightSize;
    short n = lightSize;

    /*
     * lightmap: 16-bit values of light intensity from 0 to 31
     *           in following format {000, l4...l0, 00000000}
     */
    while (--n >= 0) {
      short v = ((*src++) << 5) & 0x1f00;
      *dst0++ = v;
      *dst1++ = v;
    }
  }

  shademap = MemAlloc(32 * N * sizeof(u_short), MEMF_PUBLIC);
  {
    u_short *dst0 = &shademap[0 * N];
    u_short *dst1 = &shademap[16 * N];
    short i, j;

    /*
     * shademap: each row is transition from black to color to white
     *           and consists of 32 pixels in RGB12 format,
     *           it must be indexed by pair (c: color, l: light)
     *           in following format {000, l4...l0, c6...c0, 0}
     */

    for (i = 0; i < 16; i++) {
      const u_short *cp = skulls_colors;

      for (j = 0; j < N; j++) {
        u_short c = (j < skulls_colors_count) ? *cp++ : 0xf00;

        /* handle 8-bit displacement in the speed code */
        int o = (j < N / 2) ? (j + N / 2) : (j - N / 2);

        dst0[o] = ColorTransition(0x000, c, i);
        dst1[o] = ColorTransition(c, 0xfff, i);
      }

      dst0 += N;
      dst1 += N;
    }
  }

  DataScramble(shademap, N * 32);
}

static void UnLoad(void) {
  MemFree(BumpMapRender);
  MemFree(shademap);
  MemFree(lightmap);
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

  ClearIRQ(INTF_BLIT);

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
}

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(1200);
  short i;

  bplptr = CopSetupBitplanes(cp, screen[active], DEPTH);
  CopLoadColor(cp, 0, 15, 0);
  for (i = 0; i < HEIGHT * 4; i++) {
    CopWaitSafe(cp, Y(i), 0);
    /* Line quadrupling. */
    CopMove16(cp, bpl1mod, ((i & 3) != 3) ? -40 : 0);
    CopMove16(cp, bpl2mod, ((i & 3) != 3) ? -40 : 0);
    /* Alternating shift by one for bitplane data. */
    CopMove16(cp, bplcon1, (i & 1) ? 0x0022 : 0x0000);
  }
  return CopListFinish(cp);
}

static void Init(void) {
  screen[0] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH, BM_HAM);
  screen[1] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH, BM_HAM);

  chunky[0] = MemAlloc((WIDTH * 4) * HEIGHT, MEMF_CHIP);
  chunky[1] = MemAlloc((WIDTH * 4) * HEIGHT, MEMF_CHIP);

  BumpMapRenderBackup = MemAlloc(BumpMapRenderBackupSize, MEMF_PUBLIC);

  EnableDMA(DMAF_BLITTER);

  BitmapClear(screen[0]);
  BitmapClear(screen[1]);

  SetupPlayfield(MODE_HAM, 7, X(0), Y(0), WIDTH * 4 + 2, HEIGHT * 4);

  custom->bpldat[4] = 0x7777; // rgbb: 0111
  custom->bpldat[5] = 0xcccc; // rgbb: 1100

  cp = MakeCopperList();
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);

  SetIntVector(INTB_BLIT, (IntHandlerT)ChunkyToPlanar, NULL);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER);

  DisableINT(INTF_BLIT);
  ResetIntVector(INTB_BLIT);

  DeleteCopList(cp);

  MemFree(BumpMapRenderBackup);

  MemFree(chunky[0]);
  MemFree(chunky[1]);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

PROFILE(BumpMapRender);

static void Render(void) {
  ProfilerStart(BumpMapRender);
  {
    short xc = normfx(SIN(frameCount * 16) * WIDTH / 2) + WIDTH / 2;
    short yc = normfx(COS(frameCount * 16) * HEIGHT / 2) + HEIGHT / 2;
    short xo = 24 - xc - normfx(SIN(frameCount * 16) * HEIGHT / 2);
    short yo = 32 - yc - normfx(COS(frameCount * 16) * HEIGHT / 2);
    BumpMapRenderT code = BumpMapRenderCrop(BumpMapRender, BumpMapRenderBackup,
                                            xc, yc);
    (*code)(chunky[active], &lightmap[(yo * 128 + xo) & 16383],
            &shademap[N / 2]);
    BumpMapRenderRestore(code, BumpMapRenderBackup);
  }
  ProfilerStop(BumpMapRender);

  c2p.phase = 0;
  c2p.chunky = chunky[active];
  c2p.bpl = screen[active]->planes;
  ChunkyToPlanar();
  active ^= 1;
}

EFFECT(BumpMapRGB, Load, UnLoad, Init, Kill, Render, NULL);
