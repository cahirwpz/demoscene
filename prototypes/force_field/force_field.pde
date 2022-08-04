final int N = 8;
final int WIDTH = 320 / N + 1;
final int HEIGHT = 256 / N + 1;
final int FPS = 60;

ArrayList<ForceField> fields;
Grid grid;

void setup() {
  size(640, 512);
  frameRate(FPS);
  noSmooth();

  tiles = new HashMap<Integer, PImage>();
  grid = new Grid(WIDTH, HEIGHT);

  fields = new ArrayList<ForceField>();
  fields.add(new Circle());
  fields.add(new EquilateralTriangle());
  fields.add(new Box());

  // return sdEquilateralTriangle(p);
  // return opOnion(c, 0.1);
}

void bigPixel(int xp, int yp, color c) {
  xp *= 2; 
  yp *= 2;
  for (int y = yp; y < yp + 2; y++) {
    for (int x = xp; x < xp + 2; x++) {
      pixels[y * width + x] = c;
    }
  }
}

float lastSec = 0.0;

void draw() {
  float t = float(frameCount) / FPS;

  if (t >= lastSec + 1.0) {
    println("#tiles =", tiles.size());
    lastSec += 1.0;
  }

  ForceField ff;
  
  ff = fields.get(0);
  ff.angle(HALF_PI * t);
  ff.scale(.1, .1);
  ff.origin(2.5, 2.5 * sin(t * PI * 0.85));

  ff = fields.get(1);
  ff.angle(-HALF_PI * t * 2);
  ff.scale(.1, .1);
  ff.origin(-2.5, 2.5 * cos(t * PI * 0.65));

  ff = fields.get(2);
  ff.angle(-HALF_PI * t * 2);
  ff.scale(.15, .15);
  ff.origin(0.5 * sin(t * PI), 0.5 * cos(t * PI));

  grid.reset();
  for (ForceField f : fields)
    f.render(grid);

  background(0);
  loadPixels(); 
  for (int y = 0; y < grid.h - 1; y++) {
    for (int x = 0; x < grid.w - 1; x++) {
      float p0 = grid.get(x, y);
      float p1 = grid.get(x + 1, y);
      float p2 = grid.get(x + 1, y + 1);
      float p3 = grid.get(x, y + 1);

      PImage tile = getTile(p0, p1, p2, p3);
      for (int ty = 0; ty < N; ty++) {
        for (int tx = 0; tx < N; tx++) {
          bigPixel(x * N + tx, y * N + ty, tile.pixels[ty * N + tx]);
        }
      }
    }
  }  
  updatePixels();
}
