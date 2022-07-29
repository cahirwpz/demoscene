static Board board;
static Turmite turmite;
static boolean fadeaway = false;

final int STEPS = 64;

void setup() {
  size(512, 560);
  background(0);
  
  textFont(loadFont("Monaco-16.vlw"));
  textSize(16);
  
  board = new Board(128, 128);
  turmite = new Turmite(ChaoticGrowth);
  turmite.position(board.w / 2, board.h / 2);
}

void status() {
  fill(64);
  rect(4, 512, width-8, 28+16, 8);
  
  fill(255);
  text("[f]adeaway: " + (fadeaway ? "yes" : "no ") + " [r]eset", 6, 530); 
  text("mouse (L) set pos", 6, 550);
}

void draw() {
  for (int i = 0; i < STEPS; i++) {
    turmite.move(board);
  }

  loadPixels();
  board.update();
  
  if (fadeaway) {
    board.fadeaway();
  }

  board.updateCell(turmite.x, turmite.y, #ff0000);
  
  updatePixels();  
  
  status();
}

void keyPressed() {
  if (key == 'f') {
    fadeaway = !fadeaway;
  }
  if (key == 'r') {
    board.reset();
    turmite.position(board.w / 2, board.h / 2);
  }
}

void mousePressed() {
  if (mouseX < 512 && mouseY < 512) {
    turmite.position(mouseX / 4, mouseY / 4);
  }
}

class Tile {
  int ci; /* color index */
  int life;
  
  void age() {
    life = life > 0 ? life - 1 : 0;
    if (life == 0)
      ci = 0;
  }
  
  int get() {
    return ci;
  }
  
  void set(int ci) {
    this.ci = ci;
    life = ci > 0 ? 255 : 0;
  }
}

class Board {
  Tile[] board;
  color[] palette = { #000000, #ffffff };
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
      board[i].set(0);
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

  color colorOf(Tile tile) {
    int ci = tile.get();
    if (ci == 0) return palette[ci];
    return lerpColor(0, palette[ci], tile.life / 255.0);
  }

  void update() {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        Tile tile = board[j * w + i];
        drawCell(i, j, colorOf(tile));
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
  int x, y;
  int dir;
  int state;
  int[][][] transition;
  
  Turmite(int[][][] specs) {
    transition = specs;
    dir = NORTH;
    state = 0;
  }

  void position(int x, int y) {
    this.x = x;
    this.y = y;
  }

  void move(Board b) {
    Tile tile = b.tile(x, y);
    
    int[] change = transition[state][tile.get()];
    
    tile.set(change[0]);
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
