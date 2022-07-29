static Board board;
ArrayList<Turmite> turmites;
static boolean fadeaway = false;
static int active = 0;

final color BGCOL = #000000;
final color FGCOL = #ff0000;
final int STEPS = 64;

void setup() {
  size(512, 560);
  background(0);
  
  textFont(loadFont("Monaco-16.vlw"));
  textSize(16);
  
  color[] pal1 = { #000000, #ff0000 };
  color[] pal2 = { #000000, #00ff00 };
  
  board = new Board(128, 128);
  turmites = new ArrayList<Turmite>();
  turmites.add(new Turmite(ChaoticGrowth, pal1,
                           board.w / 4, board.h / 4));
  turmites.add(new Turmite(ChaoticGrowth, pal2,
                           board.w * 3 / 4, board.h * 3 / 4));
}

void status() {
  fill(64);
  rect(4, 512, width-8, 28+16, 8);
  
  fill(255);
  
  String bar = String.format("[r]eset [1-%d] active (%d) [f]adeaway: %s",
                             turmites.size(), active + 1,
                             fadeaway ? "yes" : "no ");
  
  text(bar, 6, 530);
  text("mouse (L) set pos", 6, 550);
}

void draw() {
  for (Turmite t : turmites) {
    for (int i = 0; i < STEPS; i++) {
      t.move(board);
    }
  }

  loadPixels();
  board.update();
  
  if (fadeaway) {
    board.fadeaway();
  }

  for (Turmite t : turmites) {
    board.updateCell(t.x, t.y, FGCOL);
  }
  updatePixels();  
  
  status();
}

void keyPressed() {
  if (key == 'f') {
    fadeaway = !fadeaway;
  }
  if (key == 'r') {
    board.reset();
    for (Turmite t : turmites) {
      t.reset();
    }
  }
  if (key >= '1' && key <= '9') {
    int num = key - '1';
    if (num < turmites.size()) {
      active = num;
    }
  }
}

void mousePressed() {
  if (mouseX < 512 && mouseY < 512) {
    turmites.get(active).position(mouseX / 4, mouseY / 4);
  }
}

class Tile {
  int ci; /* color index */
  int life;
  Turmite owner;
  
  void age() {
    life = life > 0 ? life - 1 : 0;
    if (life == 0) {
      reset();
    }
  }
  
  void reset() {
    ci = 0;
    life = 0;
    owner = null;
  }
  
  int get() {
    return ci;
  }
  
  void set(Turmite owner, int ci) {
    this.owner = owner;
    this.ci = ci;
    life = ci > 0 ? 255 : 0;
  }

  color colorOf() {
    if (ci == 0 || owner == null) return BGCOL;
    return lerpColor(0, owner.palette[ci], life / 255.0);
  }

}

class Board {
  Tile[] board;
  int w, h;
  
  Board(int w, int h) {
    this.w = w;
    this.h = h;
    board = new Tile[w * h];
    for (int i = 0; i < board.length; i++) {
      board[i] = new Tile();
    }
  }
  
  void reset() {
    for (int i = 0; i < board.length; i++) {
      board[i].reset();
    }
  }
  
  int hp(int x) {
    x %= w;
    if (x < 0)
      x += w;
    return x;
  }
  
  int vp(int y) {
    y %= h;
    if (y < 0)
      y += h;
    return y;
  }
  
  Tile tile(int x, int y) {
    return board[vp(y) * w + hp(x)];
  }
      
  void fadeaway() {
    for (int i = 0; i < w * h; i++) {
      board[i].age();
    }
  }
  
  void updateCell(int x, int y, color c) {
    drawCell(hp(x), vp(y), c);
  }

  void drawCell(int x, int y, color c) {
    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < 4; i++) {
        pixels[(y * 4 + j) * width + (x * 4 + i)] = c;
      }
    }
  }

  void update() {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        Tile tile = board[j * w + i];
        drawCell(i, j, tile.colorOf());
      }
    }
  }
}

final int SOUTH = 0;
final int WEST = 1;
final int NORTH = 2;
final int EAST = 3;

/* 2D Turing machine */
class Turmite {
  int initial_x, initial_y;
  int x, y;
  int dir;
  int state;
  int[][][] transition;
  color[] palette;
  
  Turmite(int[][][] specs, color[] pal, int x, int y) {
    transition = specs;
    palette = pal;
    dir = int(random(SOUTH, EAST));
    state = 0;
    initial_x = x;
    initial_y = y;
    reset();
  }

  void reset() {
    position(initial_x, initial_y);
  }

  void position(int x, int y) {
    this.x = x;
    this.y = y;
    initial_x = x;
    initial_y = y;
  }

  void move(Board b) {
    Tile tile = b.tile(x, y);
    
    int[] change = transition[state][tile.get()];
    
    tile.set(this, change[0]);
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
  }
}
