#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <sprite.h>
#include <system/memory.h>
#include <system/interrupt.h>
#include <common.h>

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 3

#define V_LINE_WIDTH 14
#define H_LINE_WIDTH 7
#define NO_OF_V_LINES 8
#define NO_OF_H_LINES 6
#define MAX_SPEED 4

static __code CopListT *cp;
static __code CopInsPairT *bplptr;
static __code BitmapT *screen[2];
static __code short active = 0;

static CopInsPairT *cop_lines[HEIGHT][3];

#define LINEW (WIDTH / 8 / sizeof(short))

typedef short line[LINEW];
typedef struct linedesc { short pos; short width; } linedesc;

// buffer contains all possible lines for each frame (base line + light
// intensity from 0 to 7)
static __data_chip line buffer[8][3];
static __data_chip line carry[2];
// line_sel describes which line from buffer to choose for each line on screen
static short line_sel[HEIGHT];

// {position, thickness}
static linedesc v_lines[NO_OF_V_LINES] = {
  {0, 7}, {70, 14}, {140, 9}, {166, 14}, {236, 9}, {306, 7}, {30, 9}, {290, 14},
};
// Only position, all harizontal lines have the same thickness (for now)
static short h_lines[NO_OF_H_LINES] = {0, 60, 120, 129, 189, 230};

static void CalculateFirstLine(line buf[3], linedesc vl[NO_OF_V_LINES]) {
  /*
   * Calculate first line by adding light intensity of vertical lines.
   */
  short word, offset, aux, thickness;
  short i;
  short *ptr;

  // V_LINE_n where n means line thickness in pixels

  static const u_short V_LINE_14[3] = {
    13107 << 2, // bin: 11001100110011
    3900 << 2,  // bin: 00111100111100
    192 << 2,   // bin: 00000011000000
  };

  static const u_short V_LINE_9[3] = {
    341 << 7, // bin: 101010101
    198 << 7, // bin: 011000110
    56 << 7,  // bin: 000111000
  };

  static const u_short V_LINE_7[3] = {
    85 << 9, // bin: 1010101
    54 << 9, // bin: 0110110
    8 << 9,  // bin: 0001000
  };

  const u_short *V_LINE;

  /* Set first line to 0 */
  memset(buf, 0, 3 * sizeof(line));

  /* Calculate coordinates */
  word = vl[0].pos >> 4;
  offset = vl[0].pos - (word << 4);
  thickness = vl[0].width;

  if (thickness == 14) {
    V_LINE = V_LINE_14;
  } else if (thickness == 9) {
    V_LINE = V_LINE_9;
  } else {
    V_LINE = V_LINE_7;
  }

  /* Draw first beam */
  for (i = 0; i < 3; ++i) {
    ptr = buf[i];
    ptr[word] = V_LINE[i] >> offset;
    if (offset > (16 - thickness)) {
      ptr[word + 1] = V_LINE[i] << (16 - offset);
    }
  }

  /* Add rest of the beams */
  for (i = 1; i < NO_OF_V_LINES; ++i) {
    u_short w1, w2, c1, c2 = 0;

    word = vl[i].pos >> 4;
    offset = vl[i].pos - (word << 4);
    thickness = vl[i].width;

    if (thickness == 14) {
      V_LINE = V_LINE_14;
    } else if (thickness == 9) {
      V_LINE = V_LINE_9;
    } else {
      V_LINE = V_LINE_7;
    }

    ptr = buf[0];
    w1 = V_LINE[0] >> offset;
    c1 = ptr[word] & w1;
    ptr[word] = ptr[word] ^ w1;
    if (offset > (16 - V_LINE_WIDTH)) {
      w2 = V_LINE[0] << (16 - offset);
      c2 = ptr[word + 1] & w2;
      ptr[word + 1] = ptr[word + 1] ^ w2;
    }

    ptr = buf[1];
    w1 = V_LINE[1] >> offset;

    aux = ptr[word];
    ptr[word] = (ptr[word] ^ w1) ^ c1;
    c1 = ((aux ^ w1) & c1) ^ (aux & w1);
    if (offset > (16 - V_LINE_WIDTH)) {
      w2 = V_LINE[1] << (16 - offset);
      aux = ptr[word + 1];
      ptr[word + 1] = ptr[word + 1] ^ w2 ^ c2;
      c2 = ((aux ^ w2) & c2) ^ (aux & w2);
    }

    ptr = buf[2];
    w1 = V_LINE[2] >> offset;
    aux = ptr[word];
    ptr[word] = ptr[word] ^ w1 ^ c1;
    c1 = ((aux ^ w1) & c1) ^ (aux & w1);
    if (offset > (16 - V_LINE_WIDTH)) {
      w2 = V_LINE[2] << (16 - offset);
      aux = ptr[word + 1];
      ptr[word + 1] = ptr[word + 1] ^ w2 ^ c2;
      c2 = ((aux ^ w2) & c2) ^ (aux & w2);
    }

    ptr[word] |= c1;
    ptr[word + 1] |= c2;
    ptr = buf[0];
    ptr[word] |= c1;
    ptr[word + 1] |= c2;
    ptr = buf[1];
    ptr[word] |= c1;
    ptr[word + 1] |= c2;
  }
}

static void CalculateBuffer(line buf[8][3]) {
  /*
   * Add all possible light intensity (from 0 to (2^DEPTH)-1 to base line with
   * vertical lines)
   */

  static const short lines_bltadat[8][3] = {
    {0x0000, 0x0000, 0x0000}, {0xFFFF, 0x0000, 0x0000},
    {0x0000, 0xFFFF, 0x0000}, {0xFFFF, 0xFFFF, 0x0000},
    {0x0000, 0x0000, 0xFFFF}, {0xFFFF, 0x0000, 0xFFFF},
    {0x0000, 0xFFFF, 0xFFFF}, {0xFFFF, 0xFFFF, 0xFFFF},
  };

  line *base_line = buf[0];

  short i;

  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltcmod = 0;
  custom->bltdmod = 0;

  custom->bltadat = -1;
  custom->bltbdat = -1;
  custom->bltcdat = -1;
  custom->bltddat = -1;

  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltcon1 = 0;

  for (i = 1; i < 8; ++i) {
    line *dest_line = buf[i];

    /* BITPLANE 0 */
    /* CARRY */
    custom->bltadat = lines_bltadat[i][0];
    custom->bltbpt = base_line[0];
    custom->bltdpt = carry[0];

    custom->bltcon0 = (SRCB | DEST) | A_AND_B; // HALF_ADDER_CARRY

    custom->bltsize = (1 << 6) | LINEW;
    WaitBlitter();
    /* SUM */
    custom->bltadat = lines_bltadat[i][0];
    custom->bltbpt = base_line[0];
    custom->bltdpt = dest_line[0];

    custom->bltcon0 = (SRCB | DEST) | A_XOR_B; // HALF_ADDER

    custom->bltsize = (1 << 6) | LINEW;
    WaitBlitter();

    /* BITPLANE 1 */
    /* CARRY */
    custom->bltadat = lines_bltadat[i][1];
    custom->bltbpt = base_line[1];
    custom->bltcpt = carry[0];
    custom->bltdpt = carry[1];

    custom->bltcon0 =
      (SRCB | SRCC | DEST) | (NABC | ANBC | ABNC | ABC); // FULL ADDER CARRY

    custom->bltsize = (1 << 6) | LINEW;
    WaitBlitter();
    /* SUM */
    custom->bltadat = lines_bltadat[i][1];
    custom->bltbpt = base_line[1];
    custom->bltcpt = carry[0];
    custom->bltdpt = dest_line[1];

    custom->bltcon0 =
      (SRCB | SRCC | DEST) | (NANBC | NABNC | ANBNC | ABC); // FULL ADDER

    custom->bltsize = (1 << 6) | LINEW;
    WaitBlitter();

    /* BITPLANE 2 */
    /* CARRY */
    custom->bltadat = lines_bltadat[i][2];
    custom->bltbpt = base_line[2];
    custom->bltcpt = carry[1];
    custom->bltdpt = carry[0];

    custom->bltcon0 =
      (SRCB | SRCC | DEST) | (NABC | ANBC | ABNC | ABC); // FULL ADDER CARRY

    custom->bltsize = (1 << 6) | LINEW;
    WaitBlitter();
    /* SUM */
    custom->bltadat = lines_bltadat[i][2];
    custom->bltbpt = base_line[2];
    custom->bltcpt = &carry[1];
    custom->bltdpt = dest_line[2];

    custom->bltcon0 = (SRCB | SRCC | DEST) | (A_OR_B | A_OR_C); // FULL_ADDER;

    custom->bltsize = (1 << 6) | LINEW;
    WaitBlitter();

    /* PROPAGATE CARRY */
    custom->bltamod = 0;

    custom->bltapt = carry[0];
    custom->bltbpt = dest_line[0];
    custom->bltdpt = dest_line[0];

    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;

    custom->bltsize = (1 << 6) | LINEW;
    WaitBlitter();

    custom->bltapt = carry[0];
    custom->bltbpt = dest_line[1];
    custom->bltdpt = dest_line[1];

    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;

    custom->bltsize = (1 << 6) | LINEW;
    WaitBlitter();
  }
}

static void HorizontalLines(short ls[HEIGHT], short hl[NO_OF_H_LINES]) {
  /*
   * Fill line_sel with correct buffer line index
   */
  short i;

  memset(ls, 0, sizeof(short) * HEIGHT);

  for (i = 0; i < NO_OF_H_LINES; ++i) {
    short *pos = &ls[*hl++];

    pos[0] += 1;
    pos[1] += 2;
    pos[2] += 3;
    pos[3] += 4;
    pos[4] += 5;
    pos[5] += 4;
    pos[6] += 3;
    pos[7] += 2;
    pos[8] += 1;
  }

  for (i = 0; i < HEIGHT; ++i) {
    if (*(ls++) > 7) {
      *(ls - 1) = 7;
    }
  }
}

static void Move(short hl[NO_OF_H_LINES], linedesc vl[NO_OF_V_LINES]) {
  /*
   * Change position of each line.
   */
  short i = 0;

  static short mh[NO_OF_H_LINES] = {
    1, 2, 1, -1, -2, -1,
  };

  static short mv[NO_OF_V_LINES] = {
    1, 2, 1, -3, -2, -1, 1, -3
  };

  if (frameCount % 2 == 0) {
    return;
  }

  hl[0] += mh[0];
  hl[1] += mh[1];
  hl[2] += mh[2];
  hl[3] += mh[3];
  hl[4] += mh[4];
  hl[5] += mh[5];

  for (i = 0; i < NO_OF_H_LINES; ++i) {
    if (hl[i] > HEIGHT - 7) {
      hl[i] = HEIGHT - 7;
      mh[i] *= -1;
    }
    if (hl[i] < 0) {
      hl[i] = 0;
      mh[i] *= -1;
    }
  }

  vl[0].pos += mv[0];
  vl[1].pos += mv[1];
  vl[2].pos += mv[2];
  vl[3].pos += mv[3];
  vl[4].pos += mv[4];
  vl[5].pos += mv[5];
  vl[6].pos += mv[6];
  vl[7].pos += mv[7];

  for (i = 0; i < NO_OF_V_LINES; ++i) {
    if (vl[i].pos > WIDTH - vl[i].width) {
      vl[i].pos = WIDTH - vl[i].width;
      mv[i] *= -1;
    }
    if (vl[i].pos < 0) {
      vl[i].pos = 0;
      mv[i] *= -1;
    }
  }
}

static void UpdateCopperLines(line buf[8][3], short *ls,
                              CopInsPairT *cl[HEIGHT][3]) {
  /*
   * Update copper list so correct lines from buffer are displayed.
   * Note that most of the lines will still be "base line", so there is no need
   * to update them.
   */
  short i, j;

  for (i = 0; i < NO_OF_H_LINES; ++i) {
    short pos = h_lines[i] - MAX_SPEED;
    short *L;
    CopInsPairT **clp;

    if (pos < 0)
      continue;

    clp = &(cl[pos][0]);
    L = &(ls[pos]);

    for (j = 0; j < H_LINE_WIDTH + 2 * MAX_SPEED; ++j) {
      line *line = buf[*(L++)];

      if (pos > 255)
        break;

      CopInsSet32(*clp++, line[0]);
      CopInsSet32(*clp++, line[1]);
      CopInsSet32(*clp++, line[2]);

      ++pos;
    }
  }
}

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(4096);
  short i;

  bplptr = CopSetupBitplanes(cp, screen[active], DEPTH);

  for (i = 0; i < HEIGHT; i++) {
    line *line = buffer[line_sel[i]];
    CopWaitSafe(cp, Y(i), X(-DIWHP));
    cop_lines[i][0] = CopMove32(cp, bplpt[0], line[0]);
    cop_lines[i][1] = CopMove32(cp, bplpt[1], line[1]);
    cop_lines[i][2] = CopMove32(cp, bplpt[2], line[2]);
  }

  return CopListFinish(cp);
}

static void Init(void) {
  short i;
  for (i = 0; i < HEIGHT; ++i) {
    line_sel[i] = 0;
  }
  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG | DMAF_RASTER);

  screen[0] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH, HEIGHT, DEPTH, BM_CLEAR);
  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);

  SetColor(0, 0x000);
  SetColor(1, 0x313);
  SetColor(2, 0x535);
  SetColor(3, 0x757);
  SetColor(4, 0x979);
  SetColor(5, 0xC9C);
  SetColor(6, 0xFBF);
  SetColor(7, 0xFFF);

  cp = MakeCopperList();
  CopListActivate(cp);
}

static void Kill(void) {
  DisableDMA(DMAF_BLITTER | DMAF_BLITHOG | DMAF_RASTER);

  DeleteCopList(cp);
  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

PROFILE(Darkroom);

static void Render(void) {
  ProfilerStart(Darkroom);
  {
    CalculateFirstLine(buffer[0], v_lines);            // 1860c [26r]
    CalculateBuffer(buffer);                        // 1806c [55r]
    HorizontalLines(line_sel, h_lines);             // 344c  [29r]
    Move(h_lines, v_lines);                         // 996c  [3r]
    UpdateCopperLines(buffer, line_sel, cop_lines); // 592c  [50r]
  }
  ProfilerStop(Darkroom); // Total: 162r

  ITER(i, 0, DEPTH - 1, CopInsSet32(&bplptr[i], screen[active]->planes[i]));

  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(Darkroom, NULL, NULL, Init, Kill, Render, NULL);
