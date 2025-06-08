#include <effect.h>
#include <copper.h>
#include <palette.h>
#include <pixmap.h>
#include <gfx.h>
#include <sprite.h>
#include <system/memory.h>

#define _SYSTEM
#include "system/cia.h"

#include "data/tree-pal.c"
#include "data/tree-data.c"
#include "data/ghost64x_01.c"
//#include "data/ghost64x_02.c"
//#include "data/ghost64x_03.c"
#include "data/ghost32x_01.c"
#include "data/ghost32x_02.c"
#include "data/ghost32x_03.c"

#define WIDTH 320
#define HEIGHT 256

#include "softsprite.h"

static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code CopInsPairT *sprptr;
static __code SprChanT sprchan[2][8];
static __code CopInsT *colorLine[HEIGHT];
static __code short active = 0;

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

#if 0
SpriteDither(sprA, 0x55555555);
SpriteDither(sprB, 0xAAAAAAAA);

static void SpriteDither(SpriteT *spr, u_int mask) {
  u_int *data = (u_int *)&spr->data[0];
  short n = SpriteHeight(spr);

  while (n-- > 0) {
    *data++ &= mask;
    mask = ~mask;
  }
}
#endif

static SOFTSPRITEARRAY(sprites, 24);

static void InitSpriteArray(SoftSpriteArrayT *sprites) {
  /* small ghost top-left */
  AddSprite(sprites, smallGhost1_0, X(32 + 16 * 0), Y(32));
  AddSprite(sprites, smallGhost1_1, X(32 + 16 * 1), Y(32));
  /* small ghost top-mid-left */
  AddSprite(sprites, smallGhost1_0, X(112 + 16 * 0), Y(32));
  AddSprite(sprites, smallGhost1_1, X(112 + 16 * 1), Y(32));
  /* small ghost top-mid-right */
  AddSprite(sprites, smallGhost1_0, X(176 + 16 * 0), Y(32));
  AddSprite(sprites, smallGhost1_1, X(176 + 16 * 1), Y(32));
  /* small ghost top-right */
  AddSprite(sprites, smallGhost1_0, X(256 + 16 * 0), Y(32));
  AddSprite(sprites, smallGhost1_1, X(256 + 16 * 1), Y(32));
  /* small ghost bottom-left */
  AddSprite(sprites, smallGhost3_0, X(32 + 16 * 0), Y(192));
  AddSprite(sprites, smallGhost3_1, X(32 + 16 * 1), Y(192));
  /* small ghost bottom-mid-left */
  AddSprite(sprites, smallGhost3_0, X(112 + 16 * 0), Y(192));
  AddSprite(sprites, smallGhost3_1, X(112 + 16 * 1), Y(192));
  /* small ghost bottom-mid-right */
  AddSprite(sprites, smallGhost3_0, X(176 + 16 * 0), Y(192));
  AddSprite(sprites, smallGhost3_1, X(176 + 16 * 1), Y(192));
  /* small ghost bottom-right */
  AddSprite(sprites, smallGhost3_0, X(256 + 16 * 0), Y(192));
  AddSprite(sprites, smallGhost3_1, X(256 + 16 * 1), Y(192));
  /* small ghost center-left */
  AddSprite(sprites, smallGhost2_0, X(32 + 16 * 0), Y((256 - smallGhost1_height) / 2));
  AddSprite(sprites, smallGhost2_1, X(32 + 16 * 1), Y((256 - smallGhost1_height) / 2));
  /* small ghost center-right */
  AddSprite(sprites, smallGhost2_0, X(256 + 16 * 0), Y((256 - smallGhost1_height) / 2));
  AddSprite(sprites, smallGhost2_1, X(256 + 16 * 1), Y((256 - smallGhost1_height) / 2));
  /* big ghost */
  AddSprite(sprites, ghost1_0, X(128 + 16 * 0), Y((256 - ghost1_height) / 2));
  AddSprite(sprites, ghost1_1, X(128 + 16 * 1), Y((256 - ghost1_height) / 2));
  AddSprite(sprites, ghost1_2, X(128 + 16 * 2), Y((256 - ghost1_height) / 2));
  AddSprite(sprites, ghost1_3, X(128 + 16 * 3), Y((256 - ghost1_height) / 2));
}

static DispWinT diw = {X(0), Y(0), X(WIDTH), Y(HEIGHT) };

static void Init(void) {
  SetupPlayfield(MODE_LORES, tree_depth,
                 X(0), Y(0), tree_width, HEIGHT);

  LoadColors(ghost_colors, 16);
  LoadColors(ghost_colors, 20);
  LoadColors(smallGhost_colors, 24);
  LoadColors(smallGhost_colors, 28);

  InitSpriteArray(&sprites);

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

PROFILE(SpookyTree);

static void Render(void) {
  short line = mod16(frameCount, tree_height - HEIGHT);
  short i;

  for (i = 0; i < tree_depth; i++) {
    CopInsSet32(&bplptr[i], tree.planes[i] + tree_bytesPerRow * line);
  }

  ProfilerStart(SpookyTree);
  {
    RenderSprites(sprchan[active], &sprites, &diw);
    active ^= 1;
  }
  ProfilerStop(SpookyTree);

  UpdateColorLines(line);

  TaskWaitVBlank();
}

static void VBlank(void) {
  // short j = (ReadFrameCounter() >> 3) & 3;
  short i;

  for (i = 0; i < 8; i++) {
    CopInsSetSprite(&sprptr[i], sprchan[active][i].spr);
  }
}

EFFECT(SpookyTree, NULL, NULL, Init, Kill, Render, VBlank);
