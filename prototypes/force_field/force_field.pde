final int WIDTH = 320;
final int HEIGHT = 256;
final int FPS = 60; 
final float RATIO = (float)WIDTH / (float)HEIGHT;
final int N = 8;
final float THRESHOLD = 0.9;

float field[];

ForceField[] ff;

HashMap<Integer, PImage> tiles;

void setup() {
  size(640, 512);
  frameRate(FPS);
  noSmooth();

  tiles = new HashMap<Integer, PImage>();
  field = new float[WIDTH * HEIGHT];

  ff = new ForceField[3];
  ff[0] = new ForceField(new Circle(0.1));
  ff[1] = new ForceField(new Circle(0.1));
  ff[2] = new ForceField(new Circle(0.1));

  // return sdEquilateralTriangle(p);
  // return opOnion(c, 0.1);
}

int tileIndex(float p0, float p1, float p2, float p3) {
  boolean b0 = p0 > THRESHOLD;
  boolean b1 = p1 > THRESHOLD;
  boolean b2 = p2 > THRESHOLD;
  boolean b3 = p3 > THRESHOLD;

  if (b0 && b1 && b2 && b3) // full ? 
    return 1;

  boolean i0 = b0 ^ b1;
  boolean i1 = b1 ^ b2;
  boolean i2 = b2 ^ b3;
  boolean i3 = b3 ^ b0;

  int i = (int(i0) << 12) | (int(i1) << 13) | (int(i2) << 14) | (int(i3) << 15)
        | (int(b0) << 16) | (int(b1) << 17) | (int(b2) << 18) | (int(b3) << 19);
  int ni = int(i0) + int(i1) + int(i2) + int(i3);

  assert(ni % 2 == 0);

  int l0 = 0, l1 = 0, l2 = 0, l3 = 0;

  if (ni > 0) {
    if (i0) {
      l0 = int(N * (THRESHOLD - p0) / (p1 - p0));
      assert(l0 < N && l0 >= 0);
      i |= l0;
    }

    if (i1) {
      l1 = int(N * (THRESHOLD - p1) / (p2 - p1));
      assert(l1 < N && l1 >= 0);
      i |= l1 << 3;
    }

    if (i2) {
      l2 = int(N * (THRESHOLD - p2) / (p3 - p2));
      assert(l2 < N && l2 >= 0);
      i |= l2 << 6;
    }

    if (i3) {
      l3 = int(N * (THRESHOLD - p0) / (p3 - p0));
      assert(l3 < N && l3 >= 0);
      i |= l3 << 9;
    }
  }
  
  return i;
}

PImage getTile(float p0, float p1, float p2, float p3) {
  int ti = tileIndex(p0, p1, p2, p3); 
  PImage tile = tiles.get(ti);

  if (tile == null) {
    tile = createImage(N, N, RGB);

    float dl = (p3 - p0) / N;
    float dr = (p2 - p1) / N;

    for (int ty = 0; ty < N; ty++) {
      for (int tx = 0; tx < N; tx++) {
        float p = lerp(p0, p1, (float)tx / N);
        int v = (p >= THRESHOLD) ? 255 : 0;
        tile.pixels[ty * N + tx] = color(v, v, v);
      }
      p0 += dl;
      p1 += dr;
    }

    tiles.put(ti, tile);
  }

  return tile;
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

  ff[0].angle = HALF_PI * t;
  ff[0].size.set(4.0 * RATIO, 4.0);
  ff[0].origin.set(1.25, 1.25 * sin(t * PI * 0.8));

  ff[1].angle = -HALF_PI * t;
  ff[1].size.set(4.0 * RATIO, 4.0);
  ff[1].origin.set(-1.25, 1.25 * cos(t * PI * 0.7));

  ff[2].angle = -HALF_PI * t;
  ff[2].size.set(4.0 * RATIO, 4.0);
  ff[2].origin.set(0.5 * sin(t * PI), 0.5 * cos(t * PI));

  for (int i = 0; i < field.length; i++)
    field[i] = 0.0;
  for (int i = 0; i < ff.length; i++)
    ff[i].render(field);

  background(0);
  loadPixels(); 
  for (int y = 0; y < HEIGHT - N; y += N) {
    for (int x = 0; x < WIDTH - N; x += N) {
      float p0 = field[y * WIDTH + x];
      float p1 = field[y * WIDTH + x + N];
      float p2 = field[(y + N) * WIDTH + x + N];
      float p3 = field[(y + N) * WIDTH + x];
      PImage tile = getTile(p0, p1, p2, p3);
      for (int ty = 0; ty < N; ty++) {
        for (int tx = 0; tx < N; tx++) {
          bigPixel(x + tx, y + ty, tile.pixels[ty * N + tx]);
        }
      }
    }
  }  
  updatePixels();
}
