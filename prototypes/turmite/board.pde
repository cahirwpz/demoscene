import java.lang.reflect.*;

final PApplet PAPPLET = this;

static ArrayList<Turmite> turmites;
static int generation = 0;

final color BGCOL = #000000;
final color FGCOL = #ff0000;

class Board {
  boolean fadeaway;
  boolean showhead;
  int nsteps;

  Class<? extends Tile> tileClass;
  int activeClass;

  Tile[] tiles;
  int w, h;

  Board(int w, int h) {
    this.w = w;
    this.h = h;

    tileClass = BasicTile.class;
    tiles = new Tile[w * h];

    reset();
  }

  void reset() {
    // MAGIC: https://discourse.processing.org/t/newinstance-method/25324/5
    try {
      Constructor<? extends Tile> ctor =
        tileClass.getDeclaredConstructor(PAPPLET.getClass());
      for (int i = 0; i < tiles.length; i++) {
        tiles[i] = ctor.newInstance(PAPPLET);
      }
    } 
    catch (Exception ex) {
      System.out.println(ex.toString());
    }

    for (Turmite t : turmites) {
      t.reset();
    }
  }

  void nextTileClass() {
    final Class[] classes = {
      BasicTile.class, HeatTile.class, GenerationTile.class 
    };
    activeClass = (activeClass + 1) % classes.length;
    tileClass = classes[activeClass];
    reset();
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
      for (int i = 0; i < nsteps; i++) {
        int x = hp(t.x);
        int y = vp(t.y);
        tiles[y * w + x].move(t);
      }
    }

    generation++;
  }

  void draw() {
    loadPixels();

    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        Tile tile = tiles[j * w + i];
        drawTile(i, j, tile.colorOf());
      }
    }

    if (showhead) {
      for (Turmite t : turmites) {
        drawTile(hp(t.x), vp(t.y), FGCOL);
      }
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
