static ArrayList<Turmite> turmites;
static boolean fadeaway = false;

final color BGCOL = #000000;
final color FGCOL = #ff0000;
final int STEPS = 64;

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
  
  void move(Turmite turmite) {
    ci = turmite.transition(ci);
    life = ci > 0 ? 255 : 0;  
    owner = turmite;
  }

  color colorOf() {
    if (ci == 0 || owner == null) return BGCOL;
    return lerpColor(0, owner.palette[ci], life / 255.0);
  }

}

class Board {
  Tile[] tiles;
  int w, h;
  
  Board(int w, int h) {
    this.w = w;
    this.h = h;
    tiles = new Tile[w * h];
    for (int i = 0; i < tiles.length; i++) {
      tiles[i] = new Tile();
    }
  }
  
  void reset() {
    for (int i = 0; i < tiles.length; i++) {
      tiles[i].reset();
    }

    for (Turmite t : turmites) {
      t.reset();
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

  void drawTile(int x, int y, color c) {
    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < 4; i++) {
        pixels[(y * 4 + j) * width + (x * 4 + i)] = c;
      }
    }
  }
  
  void moveTurmites() {
    for (Turmite t : turmites) {
      for (int i = 0; i < STEPS; i++) {
        int x = hp(t.x);
        int y = vp(t.y);
        tiles[y * w + x].move(t);
      }
    }
  }
  
  void draw() {
    loadPixels();

    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        Tile tile = tiles[j * w + i];
        drawTile(i, j, tile.colorOf());
      }
    }
    
    for (Turmite t : turmites) {
      drawTile(hp(t.x), vp(t.y), FGCOL);
    }
    
    updatePixels(); 
  }

  void simulate() {
    if (fadeaway) {
      for (int i = 0; i < w * h; i++) {
        tiles[i].age();
      }
    }

    moveTurmites();
    draw();
  }
}
