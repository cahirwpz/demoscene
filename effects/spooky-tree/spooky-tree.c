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

#define softsprites_count 16
static __code SoftSpriteT softsprites[16] = {
  /* small ghost top-left */
  {
    .x = 32 + 16 * 0,
    .y = 32,
    .spr = smallGhost1_0,
  },
  {
    .x = 32 + 16 * 1,
    .y = 32,
    .spr = smallGhost1_1,
  },
  /* small ghost top-right */
  {
    .x = 256 + 16 * 0,
    .y = 32,
    .spr = smallGhost1_0,
  },
  {
    .x = 256 + 16 * 1,
    .y = 32,
    .spr = smallGhost1_1,
  },
  /* small ghost bottom-left */
  {
    .x = 32 + 16 * 0,
    .y = 192,
    .spr = smallGhost3_0,
  },
  {
    .x = 32 + 16 * 1,
    .y = 192,
    .spr = smallGhost3_1,
  },
  /* small ghost bottom-right */
  {
    .x = 256 + 16 * 0,
    .y = 192,
    .spr = smallGhost3_0,
  },
  {
    .x = 256 + 16 * 1,
    .y = 192,
    .spr = smallGhost3_1,
  },
  /* small ghost center-left */
  {
    .x = 32 + 16 * 0,
    .y = (256 - smallGhost1_height) / 2,
    .spr = smallGhost2_0,
  },
  {
    .x = 32 + 16 * 1,
    .y = (256 - smallGhost1_height) / 2,
    .spr = smallGhost2_1,
  },
  /* small ghost center-right */
  {
    .x = 256 + 16 * 0,
    .y = (256 - smallGhost1_height) / 2,
    .spr = smallGhost2_0,
  },
  {
    .x = 256 + 16 * 1,
    .y = (256 - smallGhost1_height) / 2,
    .spr = smallGhost2_1,
  },
  /* big ghost */
  {
    .x = (320 - 64) / 2 + 16 * 0,
    .y = (256 - ghost1_height) / 2,
    .spr = ghost1_0,
  },
  {
    .x = (320 - 64) / 2 + 16 * 1,
    .y = (256 - ghost1_height) / 2,
    .spr = ghost1_1,
  },
  {
    .x = (320 - 64) / 2 + 16 * 2,
    .y = (256 - ghost1_height) / 2,
    .spr = ghost1_2,
  },
  {
    .x = (320 - 64) / 2 + 16 * 3,
    .y = (256 - ghost1_height) / 2,
    .spr = ghost1_3,
  },
};

static void InitSprites(void) {
  short i;

  for (i = 0; i < 8; i++) {
    InitSprChan(&sprchan[0][i], HEIGHT + 10);
    InitSprChan(&sprchan[1][i], HEIGHT + 10);
  }

  RenderSprites(sprchan[0], softsprites, softsprites_count);
  RenderSprites(sprchan[1], softsprites, softsprites_count);

  for (i = 0; i < 8; i++) {
    CopInsSetSprite(&sprptr[i], sprchan[0][i].spr);
  }
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, tree_depth,
                 X(0), Y(0), tree_width, HEIGHT);

  LoadColors(ghost_colors, 16);
  LoadColors(ghost_colors, 20);
  LoadColors(smallGhost_colors, 24);
  LoadColors(smallGhost_colors, 28);

  cp = MakeCopperList();
  CopListActivate(cp);

  InitSprites();

  EnableDMA(DMAF_RASTER | DMAF_SPRITE);
}

static void Kill(void) {
  ResetSprites();
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

static void Render(void) {
  short line = mod16(frameCount, tree_height - HEIGHT);
  short i;

  for (i = 0; i < tree_depth; i++) {
    CopInsSet32(&bplptr[i], tree.planes[i] + tree_bytesPerRow * line);
  }

  UpdateColorLines(line);

  TaskWaitVBlank();
}

static void VBlank(void) {
  static short active = 0;
  // short j = (ReadFrameCounter() >> 3) & 3;
  short i;

  for (i = 0; i < 8; i++) {
    CopInsSetSprite(&sprptr[i], sprchan[active][i].spr);
  }

  active ^= 1;
}

EFFECT(SpookyTree, NULL, NULL, Init, Kill, Render, VBlank);
