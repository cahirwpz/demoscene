#include <effect.h>
#include <copper.h>
#include <palette.h>
#include <pixmap.h>
#include <gfx.h>
#include <sprite.h>
#include <system/memory.h>

#include "data/tree-pal.c"
#include "data/tree-data.c"
#include "data/ghost64x_01.c"
#include "data/ghost64x_02.c"
#include "data/ghost64x_03.c"
#include "data/ghost32x_01.c"
#include "data/ghost32x_02.c"
#include "data/ghost32x_03.c"

static SpriteT *bigGhost[3][4] = {
  { ghost1_0, ghost1_1, ghost1_2, ghost1_3 },
  { ghost2_0, ghost2_1, ghost2_2, ghost2_3 },
  { ghost3_0, ghost3_1, ghost3_2, ghost3_3 },
};

static SpriteT *smallGhost[3][2] = {
  { smallGhost1_0, smallGhost1_1 },
  { smallGhost2_0, smallGhost2_1 },
  { smallGhost3_0, smallGhost3_1 },
};

static __code short spriteFrame[4] = {0, 1, 2, 1 };

#define WIDTH 320
#define HEIGHT 256

#include "softsprite.h"

static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code CopInsPairT *sprptr;
static __code SprChanT sprchan[2][8];
static __code CopInsT *colorLine[HEIGHT];
static __code short active = 1;

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(100 + HEIGHT * (tree_cols_width + 3));

  sprptr = CopSetupSprites(cp);
  CopWait(cp, Y(-1), HP(0));
  bplptr = CopSetupBitplanes(cp, &tree, tree_depth);

  {
    u_short *data = tree_cols_pixels;
    short i;

    for (i = 0; i < HEIGHT; i++) {
      u_short c;

      /* Start exchanging palette colors at the end of line */
      CopWait(cp, Y(i - 1), X(tree_width - 4));
      /* Set border to black */
      CopMove16(cp, color[0], 0);

      c = *data++;
      colorLine[i] = CopMove16(cp, color[1], *data++);
      CopMove16(cp, color[2], *data++);
      CopMove16(cp, color[3], *data++);
      CopMove16(cp, color[4], *data++);
      CopMove16(cp, color[5], *data++);
      CopMove16(cp, color[6], *data++);
      CopMove16(cp, color[7], *data++);

      /* Wait to set color #0 where image starts */
      CopWait(cp, Y(i), X(-4));
      CopMove16(cp, color[0], c);
    }

    /* Make sure color #0 does not spread to other lines */
    CopWait(cp, Y(i - 1), X(tree_width - 4));
    CopMove16(cp, color[0], 0);
  }

  return CopListFinish(cp);
}

static void AppendSpriteDitherA(SprDataT *dst, SprDataT *src, short height) {
  height >>= 1;

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltapt = src++;
  custom->bltamod = 4;
  custom->bltbdat = 0x5555;
  custom->bltdpt = dst++;
  custom->bltdmod = 4;
  custom->bltcon0 = (SRCA | DEST) | A_AND_B;
  custom->bltcon1 = 0;
  custom->bltsize = (height << 6) | 2;
  WaitBlitter();

  custom->bltapt = src;
  custom->bltbdat = 0xAAAA;
  custom->bltdpt = dst;
  custom->bltcon0 = (SRCA | DEST) | A_AND_B;
  custom->bltsize = (height << 6) | 2;
  WaitBlitter();
}

static void AppendSpriteDitherB(SprDataT *dst, SprDataT *src, short height) {
  height >>= 1;

  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltapt = src++;
  custom->bltamod = 4;
  custom->bltbdat = 0xAAAA;
  custom->bltdpt = dst++;
  custom->bltdmod = 4;
  custom->bltcon0 = (SRCA | DEST) | A_AND_B;
  custom->bltcon1 = 0;
  custom->bltsize = (height << 6) | 2;
  WaitBlitter();

  custom->bltapt = src;
  custom->bltbdat = 0x5555;
  custom->bltdpt = dst;
  custom->bltcon0 = (SRCA | DEST) | A_AND_B;
  custom->bltsize = (height << 6) | 2;
  WaitBlitter();
}

static SOFTSPRITEARRAY(sprites, 24);

static void InitSpriteArray(SoftSpriteArrayT *sprites, short i) {
  SoftSpriteArrayReset(sprites);

  sprites->append = (frameCount & 1) ? AppendSpriteDitherA : AppendSpriteDitherB;

  /* small ghost top-left */
  AddSprite(sprites, smallGhost[i][0], X(32 + 16 * 0), Y(32));
  AddSprite(sprites, smallGhost[i][1], X(32 + 16 * 1), Y(32));
  /* small ghost top-mid-left */
  AddSprite(sprites, smallGhost[i][0], X(112 + 16 * 0), Y(32));
  AddSprite(sprites, smallGhost[i][1], X(112 + 16 * 1), Y(32));
  /* small ghost top-mid-right */
  AddSprite(sprites, smallGhost[i][0], X(176 + 16 * 0), Y(32));
  AddSprite(sprites, smallGhost[i][1], X(176 + 16 * 1), Y(32));
  /* small ghost top-right */
  AddSprite(sprites, smallGhost[i][0], X(256 + 16 * 0), Y(32));
  AddSprite(sprites, smallGhost[i][1], X(256 + 16 * 1), Y(32));
  /* small ghost bottom-left */
  AddSprite(sprites, smallGhost[i][0], X(32 + 16 * 0), Y(192));
  AddSprite(sprites, smallGhost[i][1], X(32 + 16 * 1), Y(192));
  /* small ghost bottom-mid-left */
  AddSprite(sprites, smallGhost[i][0], X(112 + 16 * 0), Y(192));
  AddSprite(sprites, smallGhost[i][1], X(112 + 16 * 1), Y(192));
  /* small ghost bottom-mid-right */
  AddSprite(sprites, smallGhost[i][0], X(176 + 16 * 0), Y(192));
  AddSprite(sprites, smallGhost[i][1], X(176 + 16 * 1), Y(192));
  /* small ghost bottom-right */
  AddSprite(sprites, smallGhost[i][0], X(256 + 16 * 0), Y(192));
  AddSprite(sprites, smallGhost[i][1], X(256 + 16 * 1), Y(192));
  /* small ghost center-left */
  AddSprite(sprites, smallGhost[i][0], X(32 + 16 * 0), Y((256 - smallGhost1_height) / 2));
  AddSprite(sprites, smallGhost[i][1], X(32 + 16 * 1), Y((256 - smallGhost1_height) / 2));
  /* small ghost center-right */
  AddSprite(sprites, smallGhost[i][0], X(256 + 16 * 0), Y((256 - smallGhost1_height) / 2));
  AddSprite(sprites, smallGhost[i][1], X(256 + 16 * 1), Y((256 - smallGhost1_height) / 2));
  /* big ghost */
  AddSprite(sprites, bigGhost[i][0], X(128 + 16 * 0), Y((256 - ghost1_height) / 2));
  AddSprite(sprites, bigGhost[i][1], X(128 + 16 * 1), Y((256 - ghost1_height) / 2));
  AddSprite(sprites, bigGhost[i][2], X(128 + 16 * 2), Y((256 - ghost1_height) / 2));
  AddSprite(sprites, bigGhost[i][3], X(128 + 16 * 3), Y((256 - ghost1_height) / 2));
}

static DispWinT diw = {X(0), Y(0), X(WIDTH), Y(HEIGHT) };

static void Init(void) {
  SetupPlayfield(MODE_LORES, tree_depth,
                 X(0), Y(0), tree_width, HEIGHT);

  LoadColors(ghost_colors, 16);
  LoadColors(ghost_colors, 20);
  LoadColors(smallGhost_colors, 24);
  LoadColors(smallGhost_colors, 28);

  InitSpriteArray(&sprites, 0);

  cp = MakeCopperList();
  CopListActivate(cp);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);

  {
    short i;

    for (i = 0; i < 8; i++) {
      InitSprChan(&sprchan[0][i], HEIGHT + 10);
      InitSprChan(&sprchan[1][i], HEIGHT + 10);
    }

    for (i = 0; i < 8; i++) {
      RenderSprites(sprchan[0], &sprites, &diw);
      CopInsSetSprite(&sprptr[i], sprchan[0][i].spr);
    }
  }

  EnableDMA(DMAF_RASTER | DMAF_SPRITE);
}

static void Kill(void) {
  ResetSprites();
  CopperStop();
  BlitterStop();

  DeleteCopList(cp);
}

static void UpdateColorLines(short line) {
  u_short *data = tree_cols_pixels + tree_cols_width * line;
  short i;

  for (i = 0; i < HEIGHT; i++) {
    CopInsT *ins = colorLine[i];
    u_short c = *data++;

    CopInsSet16(ins++, *data++);
    CopInsSet16(ins++, *data++);
    CopInsSet16(ins++, *data++);
    CopInsSet16(ins++, *data++);
    CopInsSet16(ins++, *data++);
    CopInsSet16(ins++, *data++);
    CopInsSet16(ins++, *data++);
    CopInsSet16(&ins[1], c);
  }
}

PROFILE(Sprites);
PROFILE(ColorLines);

static void Render(void) {
  short line = mod16(frameCount, tree_height - HEIGHT);
  short i;

  for (i = 0; i < tree_depth; i++) {
    CopInsSet32(&bplptr[i], tree.planes[i] + tree_bytesPerRow * line);
  }

  ProfilerStart(Sprites);
  {
    InitSpriteArray(&sprites, spriteFrame[(frameCount >> 3) & 3]);
    RenderSprites(sprchan[active], &sprites, &diw);
  }
  ProfilerStop(Sprites);

  ProfilerStart(ColorLines);
  {
    UpdateColorLines(line);
  }
  ProfilerStop(ColorLines);

  active ^= 1;
  TaskWaitVBlank();
}

static void VBlank(void) {
  short i;

  for (i = 0; i < 8; i++) {
    CopInsSetSprite(&sprptr[i], sprchan[active][i].spr);
  }
}

EFFECT(SpookyTree, NULL, NULL, Init, Kill, Render, VBlank);
