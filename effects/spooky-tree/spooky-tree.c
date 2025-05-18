#include "effect.h"
#include "copper.h"
#include "pixmap.h"
#include "gfx.h"

#include "data/tree-pal.c"
#include "data/tree-data.c"

#define HEIGHT 256

static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code CopInsT *colorLine[HEIGHT];

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(100 + HEIGHT * (tree_cols_width + 3));

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

static void Init(void) {
  SetupPlayfield(MODE_LORES, tree_depth,
                 X(0), Y(0), tree_width, HEIGHT);

  cp = MakeCopperList();
  CopListActivate(cp);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
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

EFFECT(SpookyTree, NULL, NULL, Init, Kill, Render, NULL);
