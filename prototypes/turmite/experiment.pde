interface Experiment {
  void setup();
  void keyPressed();
  void mousePressed(int x, int y);

  String status();
}

class DualPlay implements Experiment {
  void setup() {
    color[] pal1 = { #000000, #ff0000 };  
    Turmite t1 = ChaoticGrowth(pal1);
    t1.position(board.w / 4, board.h / 4);
    t1.reset();
    turmites.add(t1);

    color[] pal2 = { #000000, #00ff00 };
    Turmite t2 = ChaoticGrowth(pal2);
    t2.position(board.w * 3 / 4, board.h * 3 / 4);
    t2.reset();                                                      
    turmites.add(t2);

    board.tileClass = BasicTile.class;
    board.fadeaway = false;
    board.showhead = true;
    board.nsteps = 32;
  }

  void keyPressed() {
    if (key == 'f') {
      board.fadeaway = !board.fadeaway;
    }

    if (key >= '1' && key <= '9') {
      int num = key - '1';
      if (num < turmites.size()) {
        active = num;
      }
    }
  }

  void mousePressed(int x, int y) {
    turmites.get(active).position(x, y);
  }

  String status() {
    return String.format(
      "Multiple Turmites Experiment\n" +
      "next [e]xperiment, next [t]ile, [r]eset\n" +
      "[1-%d] active (%d), [f]adeaway: %s\n" +
      "(LMB) set position", 
      turmites.size(), active + 1, board.fadeaway ? "yes" : "no ");
  }
}

class RandomPlay implements Experiment {
  void setup() {
    color[] pal1 = { #000000, #002664, #146CFD, #8CE0FF, #FAAF05, #F3631B };
    color[] pal2 = { #000000, #348888, #22BABB, #9EF8EE, #FA7F08, #F24405 };
    color[] pal3 = { #000000, #2182BF, #04B2D9, #F2CB07, #F2DC6B, #F2F2F2 };

    Turmite t = RandomTurmite(3, pal1);
    t.position(board.w / 2, board.h / 2);
    t.reset();                                                      
    turmites.add(t);

    board.tileClass = BasicTile.class;
    board.fadeaway = false;
    board.showhead = true;
    board.nsteps = 64;
  }

  void keyPressed() {
    Turmite t = turmites.get(active);
    if (key == 'g') {
      Turmite r = RandomTurmite(4, t.palette);
      r.position(t.init_x, t.init_y);
      turmites.set(active, r);
      board.reset();
    }
    if (key == 'd') {
      t.dump();
      t.dumpJSON("dumps.json");
    }
  }

  void mousePressed(int x, int y) {
  }

  String status() {
    return String.format(
      "Random Turmite Experiment\n" +
      "next [e]xperiment, next [t]ile, [r]eset\n" +
      "[g]enerate, [d]ump");
  }
}

class AdditivePlay implements Experiment {
  void setup() {
    color[] pal = { #000000, #FF4858 };

    Turmite t = ChaoticGrowth(pal);
    t.position(board.w / 2, board.h / 2);
    t.reset();                                                      
    turmites.add(t);

    board.tileClass = HeatTile.class;
    board.fadeaway = false;
    board.showhead = true;
    board.nsteps = 128;
  }

  void keyPressed() {
  }

  void mousePressed(int x, int y) {
  }

  String status() {
    return String.format(
      "Additive Turmite Experiment\n" +
      "next [e]xperiment, next [t]ile, [r]eset\n" +
      "");
  }
}

class GenerationPlay implements Experiment {
  void setup() {
    color[] pal = { #000000, #FF4858 };

    Turmite t = SnowFlake(pal);
    t.position(board.w / 2, board.h / 2);
    t.reset();                                                      
    turmites.add(t);

    board.tileClass = GenerationTile.class;
    board.fadeaway = false;
    board.showhead = true;
    board.nsteps = 128;
  }

  void keyPressed() {
  }

  void mousePressed(int x, int y) {
  }

  String status() {
    return String.format(
      "Generation Count Turmite Experiment\n" +
      "next [e]xperiment, next [t]ile, [r]eset\n" +
      "");
  }
}

class FromJSONPlay implements Experiment {
  int index;
  String filename;
  color[] pal = { #000000, #002664, #146CFD, #8CE0FF, #FAAF05, #F3631B };
  int[][][][] rules;

  FromJSONPlay(String fname) {
    filename = fname;
  }

  void setup() {
    rules = rulesFromJSON(filename);
    board.tileClass = BasicTile.class;
    board.fadeaway = false;
    board.showhead = true;
    board.nsteps = 64;
    resetExperiment();
  }

  int[][][][] rulesFromJSON(String filename) {
    JSONArray dump = loadJSONArray(filename);
    int[][][][] rules = new int[dump.size()][][][];

    for (int i = 0; i < dump.size(); i++) {
      JSONArray turmite = dump.getJSONArray(i);
      rules[i] = new int[turmite.size()][][];

      for (int j = 0; j < turmite.size(); j++) {
        JSONArray outer = turmite.getJSONArray(j);
        rules[i][j] = new int[outer.size()][3];

        for (int k = 0; k < outer.size(); k++) {
          JSONArray inner = outer.getJSONArray(k);
          rules[i][j][k][0] = inner.getInt(0);
          rules[i][j][k][1] = inner.getInt(1);
          rules[i][j][k][2] = inner.getInt(2);
        }
      }
    }
    return rules;
  }

  void resetExperiment() {
    turmites.clear();
    board.reset();
    Turmite t;
    if (rules.length > 0)
      t = new Turmite(rules[index], pal);
    else
    {
      int[][][] placeholderRules = {{{0, 0, 0}}};
      t = new Turmite(placeholderRules, pal);
    }
    t.position(board.w / 2, board.h / 2);
    t.reset();
    turmites.add(t);
  }

  void keyPressed() {
    if (key == 'n')
    {
      index = (index + 1) % rules.length;
      resetExperiment();
    }
    else if (key == 'p')
    {
      index = (index - 1 + rules.length) % rules.length; //<>//
      resetExperiment();
    }
  }

  void mousePressed(int x, int y) {
  }

  String status() {
    return String.format(
      "Turmites from %s\n" +
      "next [e]xperiment, next [t]ile, [r]eset\n" +
      "[n]ext turmite, [p]rev turmite\n" +
      "index: %d" +
      "", filename, index);
  }
}
