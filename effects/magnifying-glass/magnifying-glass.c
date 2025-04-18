#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <sprite.h>
#include <color.h>
#include <system/memory.h>

#define S_WIDTH 320
#define S_HEIGHT 256
#define S_DEPTH 4

#define WIDTH 64
#define HEIGHT 64

static __code PixmapT *segment_p;
static __code BitmapT *segment_bp;
static __code u_char *texture_hi;
static __code u_char *texture_lo;
static __code SprDataT *sprdat;
static __code SpriteT sprite[8];
static __code CopListT *cp;

#include "data/logo-gtn.c"
#include "data/ball.c"
#include "data/ball-anim.c"

#define UVMapRenderSize (WIDTH * HEIGHT / 2 * 10 + 2)
static __code void (*UVMapRender)(u_char *chunky asm("a0"),
                                  u_char *textureHi asm("a1"),
                                  u_char *textureLo asm("a2"));

static void ChunkyToPlanar(PixmapT *input, BitmapT *output);

static void CropPixmapBlitter(
  const PixmapT *input, u_short x0, u_short y0,
  short width, short height, u_char* thi, u_char *tlo);

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

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(80);
  CopInsPairT *sprptr = CopSetupSprites(cp);
  short i;
  CopSetupBitplanes(cp, &logo_bp, S_DEPTH);
  for (i = 0; i < 8; i++)
    CopInsSetSprite(&sprptr[i], &sprite[i]);
  return CopListFinish(cp);
}

static void Load(void) {
  UVMapRender = MemAlloc(UVMapRenderSize, MEMF_PUBLIC);
  MakeUVMapRenderCode();
}

static void UnLoad(void) {
  MemFree(UVMapRender);
}

static void Init(void) {
  // segment_bp and segment_p are bitmap and pixmap for the magnified segment
  segment_bp = NewBitmap(WIDTH, HEIGHT, S_DEPTH, BM_CLEAR);
  segment_p = NewPixmap(WIDTH, HEIGHT, PM_CMAP4, MEMF_CHIP);

  texture_hi = MemAlloc(WIDTH*HEIGHT, MEMF_CHIP);
  texture_lo = MemAlloc(WIDTH*HEIGHT, MEMF_CHIP);

  sprdat = MemAlloc(SprDataSize(64, 2) * 8 * 2, MEMF_CHIP | MEMF_CLEAR);

  {
    SprDataT *dat = sprdat;
    short j;

    for (j = 0; j < 8; j++) {
      MakeSprite(&dat, 64, j & 1, &sprite[j]);
      EndSprite(&dat);
    }
  }

  SetupPlayfield(MODE_LORES, S_DEPTH, X(0), Y(0), S_WIDTH, S_HEIGHT);
  LoadColors(logo_pal_colors, 0);
  FadeBlack(logo_pal_colors, 16, 0, 8);
  LoadColors(logo_pal_colors, 16);

  cp = MakeCopperList();
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_SPRITE | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  CopperStop();
  BlitterStop();

  DeleteCopList(cp);
  MemFree(sprdat);

  MemFree(texture_hi);
  MemFree(texture_lo);
  DeletePixmap(segment_p);
  DeleteBitmap(segment_bp);
}

#define C2P_MASK0 0x00FF
#define C2P_MASK1 0x0F0F
#define C2P_MASK2 0x3333
#define C2P_MASK3 0x5555
// BLTSIZE register value
#define BLTSIZE_VAL(w, h) (h << 6 | (w))
// Blit area given these WIDTH, HEIGHT, MODULO settings
#define BLTAREA(w, h, mod) (h == 0 ? ((2*w + mod) * 1024) : ((2*w + mod) * h))
// Calculate necesary height for the area
// The macro will never return more than 1024h
#define _BLITHEIGHT(w, area, mod) ((area) / (2*w + mod))
#define BLITHEIGHT(w, area, mod) (_BLITHEIGHT(w, area, mod) >= 1024 ? 0 : _BLITHEIGHT(w, area, mod))

/* C2P pass 1 is AC + BNC, pass 2 is ANC + BC */
#define C2P_LF_PASS1 (NABNC | ANBC | ABNC | ABC)
#define C2P_LF_PASS2 (NABC | ANBNC | ABNC | ABC)

/*
 * Chunky to Planar conversion using blitter
 */
static void ChunkyToPlanar(PixmapT *input, BitmapT *output) {
  char *planes = output->planes[0];
  char *chunky = input->pixels;

  u_short planesz  = output->bplSize;
  u_short planarsz = planesz * 4;
  u_short blith    = 0;
  u_short blitarea = 0;
  u_short i = 0;

  // This is implemented as in prototypes/c2p/c2p_1x1_4bp_blitter_backforth.py
  // rem: modulos and pointers are in bytes

  //TODO: Specialcase last non-1024h blit
  //TODO: Parametrize the blit sizes for screen size other than full screen

  while (blitarea < planarsz) {
    // Swap 8x4, pass 1
    {
      blith = BLITHEIGHT(2, planarsz - blitarea, 4);

      WaitBlitter();

      /* ((a >> 8) & 0x00FF) | (b & ~0x00FF) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS1 | ASHIFT(8);
      custom->bltcon1 = 0;
      custom->bltafwm = -1;
      custom->bltalwm = -1;
      custom->bltamod = 4;
      custom->bltbmod = 4;
      custom->bltdmod = 4;
      custom->bltcdat = C2P_MASK0;

      custom->bltapt = chunky + 4 + blitarea;
      custom->bltbpt = chunky     + blitarea;
      custom->bltdpt = planes     + blitarea;
      custom->bltsize = BLTSIZE_VAL(2, blith);
    }

    // Swap 8x4, pass 2
    {
      WaitBlitter();

      /* ((a << 8) & ~0x00FF) | (b & 0x00FF) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS2 | ASHIFT(8);
      custom->bltcon1 = BLITREVERSE;

      custom->bltapt = chunky - 6 + BLTAREA(2, blith, 4) + blitarea;
      custom->bltbpt = chunky - 2 + BLTAREA(2, blith, 4) + blitarea;
      custom->bltdpt = planes - 2 + BLTAREA(2, blith, 4) + blitarea;
      custom->bltsize = BLTSIZE_VAL(2, blith);
    }
    blitarea += BLTAREA(2, blith, 4);
  }

  blitarea = 0;
  while (blitarea < planarsz) {
     blith = BLITHEIGHT(1, planarsz - blitarea, 2);

    // Swap 4x2, pass 1
    {
      WaitBlitter();

      /* ((a >> 4) & 0x0F0F) | (b & ~0x0F0F) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS1 | ASHIFT(4);
      custom->bltcon1 = 0;
      custom->bltafwm = -1;
      custom->bltalwm = -1;
      custom->bltamod = 2;
      custom->bltbmod = 2;
      custom->bltdmod = 2;
      custom->bltcdat = C2P_MASK1;

      custom->bltapt = planes + 2 + blitarea;
      custom->bltbpt = planes     + blitarea;
      custom->bltdpt = chunky     + blitarea;
      custom->bltsize = BLTSIZE_VAL(1, blith);
    }

    // Swap 4x2, pass 2
    {
      WaitBlitter();

      /* ((a << 4) & ~0x0F0F) | (b & 0x0F0F) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS2 | ASHIFT(4);
      custom->bltcon1 = BLITREVERSE;

      custom->bltapt = planes + BLTAREA(1, blith, 2) - 4 + blitarea;
      custom->bltbpt = planes + BLTAREA(1, blith, 2) - 2 + blitarea;
      custom->bltdpt = chunky + BLTAREA(1, blith, 2) - 2 + blitarea;
      custom->bltsize = BLTSIZE_VAL(1, blith);
    }

    blitarea += BLTAREA(1, blith, 2);
  }

  blitarea = 0;
  while (blitarea < planarsz) {
    blith = BLITHEIGHT(2, planarsz - blitarea, 4);

    // Swap 2x2, pass 1
    {
      WaitBlitter();

      /* ((a >> 2) & 0x3333) | (b & ~0x3333) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS1 | ASHIFT(2);
      custom->bltcon1 = 0;
      custom->bltafwm = -1;
      custom->bltalwm = -1;
      custom->bltamod = 4;
      custom->bltbmod = 4;
      custom->bltdmod = 4;
      custom->bltcdat = C2P_MASK2;

      custom->bltapt = chunky + 4 + blitarea;
      custom->bltbpt = chunky     + blitarea;
      custom->bltdpt = planes     + blitarea;
      custom->bltsize = BLTSIZE_VAL(2, blith);
    }

    // Swap 2x2, pass 2
    {
      WaitBlitter();

      /* ((a << 2) & ~0x3333) | (b & 0x3333) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS2 | ASHIFT(2);
      custom->bltcon1 = BLITREVERSE;

      custom->bltapt = chunky - 6 + BLTAREA(2, blith, 4) + blitarea;
      custom->bltbpt = chunky - 2 + BLTAREA(2, blith, 4) + blitarea;
      custom->bltdpt = planes - 2 + BLTAREA(2, blith, 4) + blitarea;
      custom->bltsize = BLTSIZE_VAL(2, blith);
    }

    blitarea += BLTAREA(2, blith, 4);
  }

  // Copy to bitplanes - last swap
  // Numbers in quotes refer to bitplane numbers on test card.

  blitarea = 0;
  while (blitarea < planarsz){
    blith = BLITHEIGHT(1, planarsz - blitarea, 6);

    //Copy to bitplane 0, "4"
    {
      WaitBlitter();

      /* ((a >> 1) & 0x5555) | (b & ~0x5555) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS1 | ASHIFT(1);
      custom->bltcon1 = 0;
      custom->bltafwm = -1;
      custom->bltalwm = -1;
      custom->bltamod = 6;
      custom->bltbmod = 6;
      custom->bltdmod = 0;
      custom->bltcdat = C2P_MASK3;

      custom->bltapt = planes + 2 + blitarea;
      custom->bltbpt = planes     + blitarea;
      custom->bltdpt = chunky + 3*planesz + i*BLTAREA(1, blith, 0);
      custom->bltsize = BLTSIZE_VAL(1, blith);
    }

    //Copy to bitplane 1, "3"
    {
      WaitBlitter();

      /* ((a << 1) & ~0x5555) | (b & 0x5555) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS2 | ASHIFT(1);
      //custom->bltcon0 = DEST | A_TO_D;
      custom->bltcon1 = BLITREVERSE;

      custom->bltapt = planes + BLTAREA(1, blith, 6) - 8 + blitarea;
      custom->bltbpt = planes + BLTAREA(1, blith, 6) - 6 + blitarea;
      custom->bltdpt = chunky + 2*planesz - 2 + (i+1)*BLTAREA(1, blith, 0);

      custom->bltsize = BLTSIZE_VAL(1, blith);
    }

    // Copy to bitplane 2, "2"
    {
      WaitBlitter();

      /* ((a >> 1) & 0x5555) | (b & ~0x5555) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS1 | ASHIFT(1);
      custom->bltcon1 = 0;

      custom->bltapt = planes + 6 + blitarea;
      custom->bltbpt = planes + 4 + blitarea;
      custom->bltdpt = chunky + planesz + i*BLTAREA(1, blith, 0);
      custom->bltsize = BLTSIZE_VAL(1, blith);
    }

    // Copy to bitplane 3, "1"
    {
      WaitBlitter();

      /* ((a << 1) & ~0x5555) | (b & 0x5555) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | C2P_LF_PASS2 | ASHIFT(1);
      custom->bltcon1 = BLITREVERSE;

      custom->bltapt = planes + BLTAREA(1, blith, 6) - 4 + blitarea;
      custom->bltbpt = planes + BLTAREA(1, blith, 6) - 2 + blitarea;
      custom->bltdpt = chunky - 2 + (i+1)*BLTAREA(1, blith, 0);
      custom->bltsize = BLTSIZE_VAL(1, blith);
    }

    blitarea += BLTAREA(1, blith, 6);
    i++;
  }

  /* Because there is an even number of steps here, c2p finishes with planar
   * data in chunky. These last 4 blits move it to planar, where it can be
   * copied to screen.
   * This is where it's getting complicated. We need to copy an arbitrary number
   * of bytes from chunky to planes.
   * The blith algorhitm is simple - modulo is zero
   * To copy the arbitrary number of bytes we should use optimal blitter
   * settings, but for now W = 32 seems to work for most cases...
   */
  blith = BLITHEIGHT(32, planesz, 0);

  // Copy bpl0 "1"
  {
    WaitBlitter();
    custom->bltcon0 = (SRCA | DEST) | A_TO_D;
    custom->bltcon1 = 0;
    custom->bltamod = 0;
    custom->bltdmod = 0;
    custom->bltapt = chunky;
    custom->bltdpt = planes;
    custom->bltsize = BLTSIZE_VAL(32, blith);
  }

  // Copy bpl1 "2"
  {
    WaitBlitter();
    custom->bltapt = chunky + planesz;
    custom->bltdpt = planes + planesz;
    custom->bltsize = BLTSIZE_VAL(32, blith);
  }

  // Copy bpl2 "3"
  {
    WaitBlitter();
    custom->bltapt = chunky + 2*planesz;
    custom->bltdpt = planes + 2*planesz;
    custom->bltsize = BLTSIZE_VAL(32, blith);
  }

  // Copy bpl3 "4"
  {
    WaitBlitter();
    custom->bltapt = chunky + 3*planesz;
    custom->bltdpt = planes + 3*planesz;
    custom->bltsize = BLTSIZE_VAL(32, blith);
  }
}

static void PositionSprite(SpriteT sprite[8], short xo, short yo) {
  short x = X(xo);
  short y = Y(yo);
  short n = 4;

  while (--n >= 0) {
    SpriteT *spr0 = sprite++;
    SpriteT *spr1 = sprite++;

    SpriteUpdatePos(spr0, x, y);
    SpriteUpdatePos(spr1, x, y);

    x += 16;
  }
}

#define POS4BPP(pixels, iw, x0, y0) ((pixels) + (y0)*(iw)/2 + (x0)/2)
static void CropPixmapBlitter(const PixmapT *input, u_short x0, u_short y0,
			      short width, short height, u_char* thi,	u_char *tlo)
{

  /* The blitter operates on words, therefore we have 4 different shift cases
   * depending how the edge of cropped pixmap is aligned to word boundaries.
   * That is, if x0 % 4 equals...
   * 0: no shift,        1: left shift by 4
   * 2: left shift by 8, 3: left shift by 12
   */

  short shiftamt = (x0 & 0x3) * 4;
  short lwm = 0x000f;
  u_char *chunky = input->pixels;

  {
    WaitBlitter();
    /* (A << 4) & 0xF0F0  */
    custom->bltcon0 = (SRCA | DEST) | ANBC | ABC | ASHIFT(shiftamt);
    custom->bltcon1 = BLITREVERSE;
    custom->bltafwm = -1;
    custom->bltalwm = lwm;
    custom->bltamod = (input->width - width) / 2;
    custom->bltdmod = 0;
    custom->bltcdat = 0xf0f0;

    custom->bltapt = POS4BPP(chunky, input->width, x0+width, y0+width -1) - 2;
    custom->bltdpt = tlo + (width*height/2) - 2; //ok
    custom->bltsize = BLTSIZE_VAL(width/4, height);
  }
  {
    WaitBlitter();
    /* (A << 4) & 0xF0F0  */
    custom->bltcon0 = (SRCA | DEST) | ANBC | ABC | ASHIFT(shiftamt);
    custom->bltcon1 = BLITREVERSE;
    custom->bltcdat = 0x0f0f;

    custom->bltapt = POS4BPP(chunky, input->width, x0+width, y0+width -1) - 2;
    custom->bltdpt = thi + (width*height/2) - 2; //ok
    custom->bltsize = BLTSIZE_VAL(width/4, height);
  }
}

static void PlanarToSprite(const BitmapT *planar, SpriteT *sprites){
  /*
   * Copy out planar format into sprites
   * This function takes care of interlacing SPRxDATA and SPRxDATB registers
   * inside a SprDataT structure
   */
  short i;

  for (i = 0; i < 4; i++) {
    // Sprite 0, plane 0
    void *sprdat =  sprites[i*2].sprdat->data;
    {
      WaitBlitter();

      custom->bltcon0 = SRCA | DEST | A_TO_D;
      custom->bltafwm = -1;
      custom->bltalwm = -1;
      custom->bltamod = 6;
      custom->bltdmod = 2;

      custom->bltapt = planar->planes[0] + 2*i;
      custom->bltdpt = sprdat;
      custom->bltsize = BLTSIZE_VAL(1, HEIGHT);
    }
    //Sprite 0, plane 1
    {
      WaitBlitter();

      custom->bltcon0 = SRCA | DEST | A_TO_D;
      custom->bltafwm = -1;
      custom->bltalwm = -1;
      custom->bltamod = 6;
      custom->bltdmod = 2;
      custom->bltadat = 0xFFFF;
      custom->bltapt = planar->planes[1] + 2*i;
      custom->bltdpt = sprdat + 2;
      custom->bltsize = BLTSIZE_VAL(1, HEIGHT);
    }

    sprdat = sprites[i*2 + 1].sprdat->data;

    // Sprite 1, plane 2
    {
      WaitBlitter();

      custom->bltcon0 = SRCA | DEST | A_TO_D;
      custom->bltafwm = -1;
      custom->bltalwm = -1;
      custom->bltamod = 6;
      custom->bltdmod = 2;

      custom->bltapt = planar->planes[2] + 2*i;
      custom->bltdpt = sprdat;
      custom->bltsize = BLTSIZE_VAL(1, HEIGHT);
    }

    // Sprite 1, plane 3
    {
      WaitBlitter();

      custom->bltcon0 = SRCA | DEST | A_TO_D;
      custom->bltafwm = -1;
      custom->bltalwm = -1;
      custom->bltamod = 6;
      custom->bltdmod = 2;

      custom->bltapt = planar->planes[3] + 2*i;
      custom->bltdpt = sprdat + 2;
      custom->bltsize = BLTSIZE_VAL(1, HEIGHT);
    }
  }
}

PROFILE(UVMapRender);
PROFILE(ChunkyToPlanar);
PROFILE(CropPixmapBlitter);
PROFILE(PlanarToSprite);

static void Render(void) {
  short xo, yo;

  {
    short *frame;
    short pos = frameCount % ball_anim_frames;
    frame = ball_anim[pos];
    xo = S_WIDTH - *frame++ - WIDTH;
    yo = *frame++;
  }

  {
    ProfilerStart(CropPixmapBlitter);
    CropPixmapBlitter(&logo, xo, yo, WIDTH, HEIGHT, texture_hi, texture_lo);
    ProfilerStop(CropPixmapBlitter);

    ProfilerStart(UVMapRender);
    UVMapRender(segment_p->pixels, texture_lo, texture_hi);
    ProfilerStop(UVMapRender);

    ProfilerStart(ChunkyToPlanar);
    ChunkyToPlanar(segment_p, segment_bp);
    ProfilerStop(ChunkyToPlanar);

    ProfilerStart(PlanarToSprite);
    PlanarToSprite(segment_bp, sprite);
    ProfilerStop(PlanarToSprite);

    PositionSprite(sprite, xo, yo);
  }

  TaskWaitVBlank();
}

EFFECT(MagnifyingGlass, Load, UnLoad, Init, Kill, Render, NULL);
