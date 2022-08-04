final int N = 8;
final int WIDTH = 320 / N + 1;
final int HEIGHT = 256 / N + 1;
final int FPS = 60; 
final float RATIO = (float)WIDTH / (float)HEIGHT;

ForceField[] ff;
Grid field;

void setup() {
  size(640, 512);
  frameRate(FPS);
  noSmooth();

  tiles = new HashMap<Integer, PImage>();
  field = new Grid(WIDTH, HEIGHT);

  ff = new ForceField[3];
  ff[0] = new ForceField(new Circle(0.1));
  ff[1] = new ForceField(new Circle(0.1));
  ff[2] = new ForceField(new Circle(0.1));

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

  ff[0].angle(HALF_PI * t);
  ff[0].scale(4.0 * RATIO, 4.0);
  ff[0].origin(1.25, 1.25 * sin(t * PI * 0.8));

  ff[1].angle(-HALF_PI * t);
  ff[1].scale(4.0 * RATIO, 4.0);
  ff[1].origin(-1.25, 1.25 * cos(t * PI * 0.7));

  ff[2].angle(-HALF_PI * t);
  ff[2].scale(4.0 * RATIO, 4.0);
  ff[2].origin(0.5 * sin(t * PI), 0.5 * cos(t * PI));

  field.reset();
  for (int i = 0; i < ff.length; i++)
    ff[i].render(field);

  background(0);
  loadPixels(); 
  for (int y = 0; y < field.h - 1; y++) {
    for (int x = 0; x < field.w - 1; x++) {
      float p0 = field.get(x, y);
      float p1 = field.get(x + 1, y);
      float p2 = field.get(x + 1, y + 1);
      float p3 = field.get(x, y + 1);

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
