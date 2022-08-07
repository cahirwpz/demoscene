/*
 * Turmite specification table
 * indexed by turmite state and board color
 * encodes: new color, direction change, new color
 *
 * direction change is:
 * +1 = RIGHT
 * -1 = LEFT
 * +2 = UTURN
 *  0 = NO-TURN
 */

final int SOUTH = 0;
final int WEST = 1;
final int NORTH = 2;
final int EAST = 3;

/* 2D Turing machine */
class Turmite {
  protected int dir, state;
  protected int nstates;
  protected color[] palette;
  protected int[][][] rules;
  protected int init_x, init_y, init_state, init_dir;

  int x, y;

  Turmite(int[][][] rules, color[] palette) {
    this.rules = rules;
    this.palette = palette;
    nstates = rules.length;
    randomize();
  }

  void randomize() {
    init_dir = randomInt(0, 3);
    init_state = randomInt(0, nstates - 1);
  }

  void reset() {
    dir = init_dir;
    state = init_state;
    x = init_x;
    y = init_y;
  }

  void position(int x, int y) {
    init_x = x;
    init_y = y;
    randomize();
    reset();
  }

  int transition(int col) {
    int[] change = rules[state][col];

    col = change[0];
    dir += change[1];
    state = change[2];

    dir &= 3;

    if (dir == SOUTH)
      y++; 
    else if (dir == WEST) 
      x--;
    else if (dir == NORTH)
      y--;
    else if (dir == EAST)
      x++;

    return col;
  }
  
  void dump() {
    println("{");
    for (int i = 0; i < rules.length; i++) {
      println("  {");
      for (int j = 0; j < rules[i].length; j++) {
        int[] row = rules[i][j];
        println(String.format("    {%d, %d, %d},",
                               row[0], row[1], row[2]));
      }
      println("  },");
    }
    println("}");
  }

  void dumpJSON(String filename) {
    JSONArray dump = new JSONArray();
    for (int i = 0; i < rules.length; i++) {
      JSONArray outer = new JSONArray();
      for (int j = 0; j < rules[i].length; j++) {
        JSONArray inner = new JSONArray();
        int[] row = rules[i][j];
        inner.append(row[0]);
        inner.append(row[1]);
        inner.append(row[2]);
        outer.append(inner);
      }
      dump.append(outer);
    }
    JSONArray prevdump = loadJSONArray(filename);
    prevdump.append(dump);
    saveJSONArray(prevdump, filename);
  }
}

Turmite Irregular(color[] pal) {
  int[][][] rules = {
    {
      { 1, +1, 0 }, 
      { 1, +1, 1 }
    }, {
      { 0, 0, 0 }, 
      { 0, 0, 1 }
    }
  };
  return new Turmite(rules, pal);
}

Turmite SpiralGrowth(color[] pal) {
  int[][][] rules = { 
    {
      { 1, 0, 1 }, 
      { 1, -1, 0 }, 
    }, {
      { 1, +1, 1 }, 
      { 0, 0, 0 }, 
    }
  };
  return new Turmite(rules, pal);
}

Turmite ChaoticGrowth(color[] pal) {
  int[][][] rules = {
    {
      { 1, +1, 1 }, 
      { 1, -1, 1 }, 
    }, {
      { 1, +1, 1 }, 
      { 0, +1, 0 }, 
    }
  };
  return new Turmite(rules, pal);
}

Turmite ExpandingFrame(color[] pal) {
  int[][][] rules = {
    {
      { 1, -1, 0 }, 
      { 1, +1, 1 }, 
    }, {
      { 0, +1, 0 }, 
      { 0, -1, 1 }, 
    }
  };
  return new Turmite(rules, pal);
}

Turmite FibonacciSpiral(color[] pal) {
  int[][][] rules = {
    {
      {1, -1, 1}, 
      {1, -1, 1}
    }, {
      {1, +1, 1}, 
      {0, 0, 0}
    }
  };
  return new Turmite(rules, pal);
}

Turmite SnowFlake(color[] pal) {
  int[][][] rules = {
    {
      {1, -1, 1}, 
      {1, +1, 0}
    }, {
      {1, +2, 1}, 
      {1, +2, 2}
    }, {
      {0, +2, 0}, 
      {0, +2, 0}
    } 
  };
  return new Turmite(rules, pal);
}

Turmite RandomTurmite(int nstates, color[] pal) {
  int ncolors = pal.length;
  int[][][] rules = new int[nstates][ncolors][3];
  for (int i = 0; i < nstates; i++) {
    for (int j = 0; j < pal.length; j++) {
      rules[i][j][0] = randomInt(0, ncolors - 1);
      rules[i][j][1] = randomInt(-1, 2);
      rules[i][j][2] = randomInt(0, nstates - 1);
    }
  }
  return new Turmite(rules, pal);
}
