#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <sprite.h>
#include <system/memory.h>
#include <system/interrupt.h>
#include <common.h>


#include "data/ground.c"
#include "data/ground2.c"

#include "data/tree1.c"
#include "data/tree2.c"
#include "data/tree3.c"

#include "data/moonbatghost1.c"
#include "data/moonbatghost2.c"


#define WIDTH 320
#define HEIGHT 97  // 1 line for trees + 6 layers * 16 lines for ground
#define DEPTH 4
#define GROUND_HEIGHT 16


static SpriteT batspr[2][2];
static SpriteT ghostspr[2][2];
static BitmapT *screen[2];
static CopInsPairT *bplptr;
static CopInsPairT *sprptr;
static CopListT *cp;
static short active = 0;
static short spract = 0;


static __data_chip u_short treeTab[6][24] = {
    { // 1st LAYER (closest)
      0x000F, 0xFFFF, 0xFFF0, 0x0000, 0x0000,
      0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF,
      0xF000, 0x0000, 0x0000, 0x0000, 0x0000,
      0x0000, 0x00FF, 0xFFFF, 0xFFF0, 0x0000,
      0x0, 0x0, 0x0, 0x0,
    },
    { // 2nd LAYER
      0x000F, 0xFFFF, 0xFFF0, 0x0000, 0x0000,
      0x0000, 0x0000, 0x0FFF, 0xFFFF, 0xF000,
      0x0000, 0x0FFF, 0xFFFF, 0x0000, 0x0000,
      0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xF000,
      0x0, 0x0, 0x0, 0x0,
    },
    { // 3rd LAYER
      0x0000, 0x0000, 0x0FFF, 0xFFF0, 0x0000,
      0x0000, 0x00FF, 0xFFFF, 0x0000, 0x0000,
      0x0000, 0x0000, 0x0000, 0x0000, 0x000F,
      0xFFFF, 0xFF00, 0x0000, 0x000F, 0xFFFF,
      0x0, 0x0, 0x0, 0x0,
    },
    { // 4th LAYER
      0x0000, 0x0000, 0xFFFF, 0x0000, 0x0000,
      0x00FF, 0xFFF0, 0x0000, 0x0000, 0x0000,
      0x0000, 0x0000, 0x0000, 0x0FFF, 0xFF00,
      0x0000, 0x0FFF, 0xF000, 0x0000, 0x0000,
      0x0, 0x0, 0x0, 0x0,
    },
    { // 5th LAYER
      0x0000, 0x0000, 0x0FFF, 0xFFF0, 0x0000,
      0x000F, 0xFFF0, 0x0000, 0x0000, 0xFFFF,
      0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
      0x0000, 0x0000, 0x0000, 0x0FFF, 0xF000,
      0x0, 0x0, 0x0, 0x0,
    },
    { // 6th LAYER
      0x00FF, 0xFF00, 0x0000, 0x0000, 0x0000,
      0x0000, 0x0000, 0x0000, 0x0FFF, 0xFF00,
      0x0000, 0xFFFF, 0x0000, 0x0000, 0x0000,
      0x0000, 0x0000, 0x0FFF, 0xF000, 0x0000,
      0x0, 0x0, 0x0, 0x0,
    },
};

static short branchesPos[4][2] = {
  // Calculate these values from treeTab in Init?
  {-4, 43},
  {112, 163},
  {248, 299},
};

static short batPos[2] = {
  320, 56
};

static short ghostData[2] = {
  -32, 110
};

static struct layer {
  short speed;
  short word;
  short bit;
} layers[6] = {
  {  2, 19, 0 },
  {  8, 19, 0 },
  {  4, 19, 0 },
  { 10, 19, 0 },
  {  6, 19, 0 },
  { 12, 19, 0 },
};


/* MOVEMENTS */
static void SwitchSprites(void) {
  if (ghostData[0] & 0x10) {
    spract = 0;
    CopInsSetSprite(&sprptr[6], &moonbatghost1[0]);
    CopInsSetSprite(&sprptr[7], &moonbatghost1[1]);
    ghostData[1]--;
  } else {
    spract = 1;
    CopInsSetSprite(&sprptr[6], &moonbatghost2[0]);
    CopInsSetSprite(&sprptr[7], &moonbatghost2[1]);
    ghostData[1]++;
  }
}

static void MoveTrees(u_int* arg) {
  u_int a, d;
  short i;

  d = (arg[11] & 1) << 31;
  for (i = 0; i < 12; i++) {
    a = *arg;
    *arg++ = d | (a >> 1);
    d = a & 1;
    asm volatile("ror.l #1,%0"
               : "=d" (d)
               : "0" (d));
  }
}

static void MoveBranches(short bp[4][2]) {
  short i, j;

  SpriteUpdatePos(&tree1[0],   X(bp[0][0]), Y(-16));
  SpriteUpdatePos(&tree1[1],   X(bp[0][1]), Y(-16));

  SpriteUpdatePos(&tree2[0],   X(bp[1][0]), Y(-16));
  SpriteUpdatePos(&tree2[1],   X(bp[1][1]), Y(-16));

  SpriteUpdatePos(&tree3[0],   X(bp[2][0]), Y(-16));
  SpriteUpdatePos(&tree3[1],   X(bp[2][1]), Y(-16));

  for (i = 0; i <= 2; ++i) {
    for (j = 0; j <= 1; ++j) {
      if (bp[i][j] > 320) {
        bp[i][j] = -63;
      }
      bp[i][j]++;
    }
  }
}

static void MoveBat(SpriteT spr[2]) {
  SpriteUpdatePos(&spr[0], X(batPos[0]), Y(batPos[1]));
  SpriteUpdatePos(&spr[1], X(batPos[0]+16), Y(batPos[1]));

  batPos[0] -= 1;
  if (batPos[0] < -64) {
    batPos[0] = 360;
  }
}

static void MoveGhost(SpriteT spr[2]) {
  SpriteUpdatePos(&spr[0], X(ghostData[0]),    Y(ghostData[1]));
  SpriteUpdatePos(&spr[1], X(ghostData[0]+16), Y(ghostData[1]));

  ghostData[0] += 2;
  if (ghostData[0] > 320) {
    ghostData[0] = -64;
    ghostData[1] = 112;
  }
}

static void MoveForest(struct layer* l, short s) {
  if (l->speed == 0) {
    l->speed = s + 1;
    MoveTrees((u_int*)treeTab[s]);

    l->bit += 1;
    if (l->bit >= 16) {
      l->bit = 0;
      l->word -= 1;
    }
    if (l->word < 0) {
      l->word = 19;
    }
  } else {
    l->speed -= 1;
  }
}

/* BLITTER */
static void VerticalFill(void** planes) {
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltcdat = -1;
  custom->bltafwm = -1;
  custom->bltalwm = -1;
  custom->bltcon1 = 0;

  /* BITPLANE 0 */
  custom->bltapt = planes[0];
  custom->bltbpt = planes[0] + WIDTH/8;
  custom->bltdpt = planes[0] + WIDTH/8;

  custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | NABC | ANBC);
  custom->bltsize = ((HEIGHT - GROUND_HEIGHT*2 - 1) << 6) | 20;
  WaitBlitter();

  /* BITPLANE 1 */
  custom->bltapt = planes[1];
  custom->bltbpt = planes[1] + WIDTH/8;
  custom->bltdpt = planes[1] + WIDTH/8;

  custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | NABC | ANBC);
  custom->bltsize = ((HEIGHT - 1) << 6) | 20;
  WaitBlitter();

  /* BITPLANE 2 */
  custom->bltapt = planes[2];
  custom->bltbpt = planes[2] + WIDTH/8;
  custom->bltdpt = planes[2] + WIDTH/8;

  custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | NABC | ANBC);
  custom->bltsize = ((HEIGHT - GROUND_HEIGHT*4 - 1) << 6) | 20;
  WaitBlitter();

  /* BITPLANE 3 */
  custom->bltapt = planes[3];
  custom->bltbpt = planes[3] + WIDTH/8;
  custom->bltdpt = planes[3] + WIDTH/8;

  custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | NABC | ANBC);
  custom->bltsize = ((HEIGHT - GROUND_HEIGHT - 1) << 6) | 20;
  WaitBlitter();
}

static void DrawForest(void **planes, u_short trees[6][24]) {
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltcmod = 0;
  custom->bltdmod = 0;

  custom->bltafwm = -1;
  custom->bltalwm = -1;

  { /* PLAYFIELD 1 */
    /* PLAYFIELD 1 BITPLANE 0 */

    custom->bltapt = trees[3];
    custom->bltbpt = trees[4];
    custom->bltcpt = trees[5];
    custom->bltdpt = planes[0];

    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (NABNC | NABC | ANBNC | ANBC | ABNC | ABC);
    custom->bltcon1 = 0;
    custom->bltsize = (1 << 6) | 20;

    WaitBlitter();

    /* PLAYFIELD 1 BITPLANE 1 */
    custom->bltapt = trees[3];
    custom->bltbpt = trees[4];
    custom->bltcpt = trees[5];
    custom->bltdpt = planes[2];

    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (NANBC | NABNC | NABC);
    custom->bltcon1 = 0;
    custom->bltsize = (1 << 6) | 20;

    WaitBlitter();
  }

  { /* PLAYFIELD 2 */
    /* PLAYFIELD 2 BITPLANE 0 */
    custom->bltapt = trees[0];
    custom->bltbpt = trees[1];
    custom->bltcpt = trees[2];
    custom->bltdpt = planes[1];

    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (NABNC | NABC | ANBNC | ANBC | ABNC | ABC);
    custom->bltcon1 = 0;
    custom->bltsize = (1 << 6) | 20;

    WaitBlitter();

    /* PLAYFIELD 2 BITPLANE 1 */
    custom->bltapt = trees[0];
    custom->bltbpt = trees[1];
    custom->bltcpt = trees[2];
    custom->bltdpt = planes[3];

    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (NANBC | NABNC | NABC);
    custom->bltcon1 = 0;
    custom->bltsize = (1 << 6) | 20;

    WaitBlitter();
  }
}

static void DrawGround(void **planes, struct layer lr[6]) {
  custom->bltamod = 40;
  custom->bltbmod = -40;
  custom->bltcmod = -40;
  custom->bltdmod = 0;

  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltcdat = -1;
  custom->bltbdat = -1;

  custom->bltcon1 = 0;

  { /* 6th LAYER */
    custom->bltapt = _ground_bpl + lr[5].word;
    custom->bltbpt = planes[0];
    custom->bltdpt = planes[2] + 40;

    custom->bltcon0 = (SRCA | SRCB | DEST) | (ANBC | ANBNC) | ASHIFT(lr[5].bit);
    custom->bltsize = (GROUND_HEIGHT << 6) | 20;
    WaitBlitter();
  }

  { /* 5th LAYER */
    custom->bltapt = _ground_bpl + lr[4].word;
    custom->bltdpt = planes[0] + 40 + 16*40;

    custom->bltcon0 = (SRCA | DEST) | A_TO_D | ASHIFT(lr[4].bit);
    custom->bltsize = (GROUND_HEIGHT << 6) | 20;
    WaitBlitter();
  }

  { /* 4th LAYER */
    custom->bltapt = _ground2_bpl + lr[3].word;
    custom->bltbpt = planes[0];
    custom->bltcpt = planes[2];
    custom->bltdpt = planes[2] + 40 + 32*40;;

    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (NABC | NANBC | NANBNC) | ASHIFT(lr[3].bit);
    custom->bltsize = (GROUND_HEIGHT << 6) | 20;
    WaitBlitter();
  }

  { /* 3rd LAYER */
    custom->bltapt = _ground_bpl + lr[2].word;
    custom->bltbpt = planes[1];
    custom->bltdpt = planes[3] + 40 + 48*40;

    custom->bltcon0 = (SRCA | SRCB | DEST) | (ANBC | ANBNC) | ASHIFT(lr[2].bit);
    custom->bltsize = (GROUND_HEIGHT << 6) | 20;
    WaitBlitter();
  }

  { /* 2nd LAYER */
    custom->bltapt = _ground2_bpl + lr[1].word;
    custom->bltdpt = planes[1] + 40 + 64*40;

    custom->bltcon0 = (SRCA | DEST) | A_TO_D | ASHIFT(lr[1].bit);
    custom->bltsize = (GROUND_HEIGHT << 6) | 20;
    WaitBlitter();
  }

  { /* 1st LAYER */
    custom->bltapt = _ground_bpl + lr[0].word;
    custom->bltbpt = planes[1];
    custom->bltcpt = planes[3];
    custom->bltdpt = planes[3] + 40 + 80*40;

    custom->bltcon0 = (SRCA | SRCB | SRCC | DEST) | (NABC | NANBC | NANBNC) | ASHIFT(lr[0].bit);
    custom->bltsize = (GROUND_HEIGHT << 6) | 20;
    WaitBlitter();
  }
}

static void ClearBitplanes(void **planes, struct layer lr[6]) {
  /* const */
  custom->bltdmod = 0;
  custom->bltcon0 = (DEST) | 0x0;
  custom->bltcon1 = 0;

  /* per blit */
  custom->bltdpt = planes[0] + 40;
  custom->bltsize = (GROUND_HEIGHT << 6) | 20;
  WaitBlitter();

  /* per blit */
  custom->bltdpt = planes[2] + 40 + 16*40;
  custom->bltsize = (GROUND_HEIGHT << 6) | 20;
  MoveForest(&lr[5], 5);
  WaitBlitter();

  /* per blit */
  custom->bltdpt = planes[1] + 40;
  custom->bltsize = ((GROUND_HEIGHT*4) << 6) | 20;
  MoveForest(&lr[4], 4);
  MoveForest(&lr[3], 3);
  MoveForest(&lr[2], 2);
  WaitBlitter();

  /* per blit */
  custom->bltdpt = planes[3] + 40;
  custom->bltsize = ((GROUND_HEIGHT*5) << 6) | 20;
  MoveForest(&lr[1], 1);
  MoveForest(&lr[0], 0);
  if (layers[0].speed == 0) {
    SwitchSprites();
    MoveGhost(ghostspr[spract]);
    MoveBat(batspr[spract]);
    MoveBranches(branchesPos);
  }
  WaitBlitter();
}

/* SETUP */
static void SetupColors(void) {
  unsigned short colors[7] = {
    0x111, // 1st layer BLUE
    0x223,
    0x334,
    0x445,
    0x556,
    0x667,
    0x778, // background
  };
  // unsigned short colors[7] = {
  //   0x111, // 1st layer GREEN
  //   0x232,
  //   0x343,
  //   0x454,
  //   0x565,
  //   0x676,
  //   0x787, // background
  // };
  // unsigned short colors[7] = {
  //   0x111, // 1st layer RED
  //   0x322,
  //   0x433,
  //   0x544,
  //   0x655,
  //   0x766,
  //   0x877, // background
  // };

  // TREES
  // PLAYFIELD 1
  SetColor(8,  colors[6]); // BACKGROUND
  SetColor(9,  colors[0]); // 1st LAYER
  SetColor(10, colors[2]); // 3rd LAYER
  SetColor(11, colors[1]); // 2nd LAYER
  SetColor(12, 0xF0F); // UNUSED
  SetColor(13, 0xF0F); // UNUSED
  SetColor(14, 0xF0F); // UNUSED
  SetColor(15, 0xF0F); // UNUSED
  // PLAYFLIELD 2
  SetColor(0,  colors[6]); // BACKGROUND
  SetColor(1,  colors[3]); // 4th LAYER
  SetColor(2,  colors[5]); // 6th LAYER
  SetColor(3,  colors[4]); // 5th LAYER
  SetColor(4,  0xF0F); // UNUSED
  SetColor(5,  0xF0F); // UNUSED
  SetColor(6,  0xF0F); // UNUSED
  SetColor(7,  0xF0F); // UNUSED

  // SPRITES
  // SPRITES 0-1  (tree 1)
  SetColor(17, 0x111);
  SetColor(18, 0x031);
  SetColor(19, 0x999);
  // SPRITES 2-3  (tree 2)
  SetColor(21, 0x111);
  SetColor(22, 0x920);
  SetColor(23, 0x031);
  // SPRITES 4-5  (tree 3)
  SetColor(25, 0x111);
  SetColor(26, 0x920);
  SetColor(27, 0x031);
  // SPRITES 6-7  (moon and ghost)
  SetColor(29, 0x444);
  SetColor(30, 0x222);
  SetColor(31, 0xBBB);
}

static void SetupSprites(void) {
  SprDataT *dat;
  sprptr = CopSetupSprites(cp);

  /* Moon, bat, ghost */
  dat = &moonbatghost10_sprdat;
  MakeSprite(&dat, 32, false, &moonbatghost1[0]);
  dat = &moonbatghost11_sprdat;
  MakeSprite(&dat, 32, false, &moonbatghost1[1]);
  dat = &moonbatghost20_sprdat;
  MakeSprite(&dat, 32, false, &moonbatghost2[0]);
  dat = &moonbatghost21_sprdat;
  MakeSprite(&dat, 32, false, &moonbatghost2[1]);

  /* Bat, active == 0 */
  dat = (SprDataT*)moonbatghost1[0].sprdat->data[32];
  MakeSprite(&dat, 19, false, &batspr[0][0]);
  dat = (SprDataT*)moonbatghost1[1].sprdat->data[32];
  MakeSprite(&dat, 19, false, &batspr[0][1]);
  /* Bat, active == 1 */
  dat = (SprDataT*)moonbatghost2[0].sprdat->data[32];
  MakeSprite(&dat, 19, false, &batspr[1][0]);
  dat = (SprDataT*)moonbatghost2[1].sprdat->data[32];
  MakeSprite(&dat, 19, false, &batspr[1][1]);

  /* Ghost, active == 0 */
  dat = (SprDataT*)moonbatghost1[0].sprdat->data[52];
  MakeSprite(&dat, 46, false, &ghostspr[0][0]);
  dat = (SprDataT*)moonbatghost1[1].sprdat->data[52];
  MakeSprite(&dat, 46, false, &ghostspr[0][1]);
  /* Ghost, active == 1 */
  dat = (SprDataT*)moonbatghost2[0].sprdat->data[52];
  MakeSprite(&dat, 46, false, &ghostspr[1][0]);
  dat = (SprDataT*)moonbatghost2[1].sprdat->data[52];
  MakeSprite(&dat, 46, false, &ghostspr[1][1]);

  CopInsSetSprite(&sprptr[6], &moonbatghost1[0]);
  CopInsSetSprite(&sprptr[7], &moonbatghost1[1]);

  SpriteUpdatePos(&moonbatghost1[0], X(16), Y(16));
  SpriteUpdatePos(&moonbatghost1[1], X(32), Y(16));
  SpriteUpdatePos(&moonbatghost2[0], X(16), Y(16));
  SpriteUpdatePos(&moonbatghost2[1], X(32), Y(16));

  /* Branches */
  CopInsSetSprite(&sprptr[0], &tree1[0]);
  CopInsSetSprite(&sprptr[1], &tree1[1]);

  CopInsSetSprite(&sprptr[2], &tree2[0]);
  CopInsSetSprite(&sprptr[3], &tree2[1]);

  CopInsSetSprite(&sprptr[4], &tree3[0]);
  CopInsSetSprite(&sprptr[5], &tree3[1]);

  SpriteUpdatePos(&tree1[0],   X(branchesPos[0][0]), Y(-16));
  SpriteUpdatePos(&tree1[1],   X(branchesPos[0][1]), Y(-16));

  SpriteUpdatePos(&tree2[0],   X(branchesPos[1][0]), Y(-16));
  SpriteUpdatePos(&tree2[1],   X(branchesPos[1][1]), Y(-16));

  SpriteUpdatePos(&tree3[0],   X(branchesPos[2][0]), Y(-16));
  SpriteUpdatePos(&tree3[1],   X(branchesPos[2][1]), Y(-16));
}

static CopListT *MakeCopperList(void) {
  short i;
  static unsigned short backgroundGradient[12] = {
    0x222, 0x222, 0x223, 0x223, // BLUE
    0x233, 0x334, 0x334, 0x435,
    0x445, 0x446, 0x456, 0x557
  };
  // static unsigned short backgroundGradient[12] = {
  //   0x221, 0x222, 0x222, 0x332, // GREEN
  //   0x343, 0x343, 0x453, 0x454,
  //   0x454, 0x464, 0x565, 0x575
  // };
  // static unsigned short backgroundGradient[12] = {
  //   0x221, 0x222, 0x222, 0x333, // RED
  //   0x333, 0x433, 0x433, 0x534,
  //   0x544, 0x644, 0x655, 0x755
  // };
  static short groundLevel[6] = {
    160,  // 6th LAYER (changing these values may brake copper list)
    144,  // 5th LAYER
    122,  // 4th LAYER
    94,   // 3rd LAYER
    64,   // 2nd LAYER
    30,   // 1st LAYER (min. 16)
  };

  cp = NewCopList(128);
  bplptr = CopSetupBitplanes(cp, screen[active], DEPTH);

  // Sprites
  SetupSprites();

  // Moon behind trees
  CopWait(cp, 0, 0);
  CopMove16(cp, bplcon2, BPLCON2_PF2PRI | BPLCON2_PF1P0 | BPLCON2_PF1P1 | BPLCON2_PF2P0 | BPLCON2_PF2P1);

  // Moon colors
  CopSetColor(cp, 29, 0x444);
  CopSetColor(cp, 30, 0x777);
  CopSetColor(cp, 31, 0xBBB);

  // Duplicate lines
  CopWaitSafe(cp, Y(0), 0);
  CopMove16(cp, bpl1mod, -40);
  CopMove16(cp, bpl2mod, -40);

  // Background gradient
  for (i = 0; i < 12; ++i) {
    CopWait(cp, Y(i * 8), 0);
    CopSetColor(cp, 0, backgroundGradient[i]);
    // Bat Colors
    if (i * 8 == 48) {
      CopSetColor(cp, 29, 0x222);
      CopSetColor(cp, 30, 0x777);
      CopSetColor(cp, 31, 0x700);
    }
  }

  // Ghost colors
  CopSetColor(cp, 29, 0x666);
  CopSetColor(cp, 30, 0x999);
  CopSetColor(cp, 31, 0xDDD);

  // Ghost between playfields
  CopMove16(cp, bplcon2, BPLCON2_PF2PRI | BPLCON2_PF2P0 | BPLCON2_PF2P1 | BPLCON2_PF1P2);

  // Duplicate lines
  for (i = 0; i < 6; ++i) {
    CopWaitSafe(cp, Y(256 - groundLevel[i]), 0);
    CopMove16(cp, bpl1mod, 0);
    CopMove16(cp, bpl2mod, 0);
    CopWaitSafe(cp, Y(256 + GROUND_HEIGHT - groundLevel[i]), 0);
    CopMove16(cp, bpl1mod, -40);
    CopMove16(cp, bpl2mod, -40);
  }

  CopListFinish(cp);

  return cp;
}

static void Init(void) {
  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG | DMAF_RASTER | DMAF_SPRITE);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  SetupPlayfield(MODE_DUALPF, DEPTH, X(0), Y(0), WIDTH, 256);

  SetupColors();

  cp = MakeCopperList();
  CopListActivate(cp);
}

static void Kill(void) {
  DisableDMA(DMAF_BLITTER | DMAF_BLITHOG | DMAF_RASTER | DMAF_SPRITE);

  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

PROFILE(Forest);

static void Render(void) {
  ProfilerStart(Forest);
  {
    ClearBitplanes(screen[active]->planes, layers);
    DrawForest(screen[active]->planes, treeTab);
    DrawGround(screen[active]->planes, layers);
    VerticalFill(screen[active]->planes);

    ITER(i, 0, DEPTH - 1, CopInsSet32(&bplptr[i], screen[active]->planes[i]));
  }
  ProfilerStop(Forest);

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Forest, NULL, NULL, Init, Kill, Render, NULL);
