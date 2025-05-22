#include "effect.h"
#include "copper.h"
#include "palette.h"
#include "pixmap.h"
#include "gfx.h"
#include "sprite.h"

#define _SYSTEM
#include "system/cia.h"

#include "data/tree-pal.c"
#include "data/tree-data.c"
#include "data/ghost64x_01.c"
#include "data/ghost64x_02.c"
#include "data/ghost64x_03.c"

#define HEIGHT 256

static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code CopInsPairT *sprptr;
static __code CopInsT *colorLine[HEIGHT];
static __code SpriteT ghost1Alt[4], ghost2Alt[4], ghost3Alt[4];

static __code SpriteT *ghost[4] = {
  ghost1,
  ghost2,
  ghost3,
  ghost2,
};

static __code SpriteT *ghostAlt[4] = {
  ghost1Alt,
  ghost2Alt,
  ghost3Alt,
  ghost2Alt,
};

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

static void SpriteDither(SpriteT *spr, u_int mask) {
  u_int *data = (u_int *)&spr->sprdat->data[0];
  short n = spr->height;

  while (n-- > 0) {
    *data++ &= mask;
    mask = ~mask;
  }
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, tree_depth,
                 X(0), Y(0), tree_width, HEIGHT);

  LoadColors(ghost_colors, 16);
  LoadColors(ghost_colors, 20);

  cp = MakeCopperList();
  CopListActivate(cp);

  {
    short i, j;

    for (j = 0; j < 3; j++) {
      for (i = 0; i < 4; i++) {
        SpriteUpdatePos(&ghost[j][i],
                        X((320 - 64) / 2 + 16 * i),
                        Y((256 - ghost1_height) / 2));
        CopySprite(&ghostAlt[j][i], &ghost[j][i]);
        SpriteDither(&ghost[j][i], 0x55555555);
        SpriteDither(&ghostAlt[j][i], 0xAAAAAAAA);
      }
    }

    for (i = 0; i < 4; i++) {
      CopInsSetSprite(&sprptr[i], &ghost[0][i]);
    }
  }

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
  short j = (ReadFrameCounter() >> 3) & 3;
  short i;

  if (active) {
    for (i = 0; i < 4; i++) {
      CopInsSetSprite(&sprptr[i], &ghost[j][i]);
    }
  } else {
    for (i = 0; i < 4; i++) {
      CopInsSetSprite(&sprptr[i], &ghostAlt[j][i]);
    }
  }

  active ^= 1;
}

EFFECT(SpookyTree, NULL, NULL, Init, Kill, Render, VBlank);
