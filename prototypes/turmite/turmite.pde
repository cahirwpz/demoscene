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
  loadPixels();

  for (int i = 0; i < STEPS; i++) {
    turmite.move(board);
  }
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

class Board {
  int board[];
  int w, h;
  
  Board(int w, int h) {
    this.w = w;
    this.h = h;
    board = new int[w * h];
  }
  
  void reset() {
    for (int i = 0; i < w * h; i++) {
      board[i] = 0;
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
  
  int get(int x, int y) {
    return board[vp(y) * w + hp(x)];
  }
  
  void set(int x, int y, int v) {
    board[vp(y) * w + hp(x)] = v;
  }
  
  void fadeaway() {
    for (int i = 0; i < w * h; i++) {
      if (board[i] > 0)
        board[i]--;
    }
  }
  
  void updateCell(int x, int y, int c) {
    _updateCell(hp(x), vp(y), c);
  }

  void _updateCell(int x, int y, int c) {
    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < 4; i++) {
        pixels[(y * 4 + j) * width + (x * 4 + i)] = c;
      }
    }
  }

  void update() {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        int v = board[j * w + i];
        int c = color(v,v,v);
        _updateCell(i, j, c);
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
    int col = b.get(x, y) > 0 ? 1 : 0;
    int change[] = transition[state][col];
    
    b.set(x, y, boolean(change[0]) ? 255 : 0);
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
