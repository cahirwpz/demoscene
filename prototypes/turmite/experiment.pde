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
      "[n]ext [p]revious [r]eset\n" +
      "[1-%d] active (%d) [f]adeaway: %s\n" +
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
    if (key == 'g') {
      Turmite t = turmites.get(active);
      Turmite r = RandomTurmite(4, t.palette);
      r.position(t.init_x, t.init_y);
      turmites.set(active, r);
      board.reset();
    }
  }

  void mousePressed(int x, int y) {
  }

  String status() {
    return String.format(
      "Random Turmite Experiment\n" +
      "[n]ext [p]revious [r]eset\n" +
      "[g]enerate");
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
      "[n]ext [p]revious [r]eset\n" +
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
      "[n]ext [p]revious [r]eset\n" +
      "");
  }
}
