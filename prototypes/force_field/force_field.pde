final int WIDTH = 320;
final int HEIGHT = 256;
final float RATIO = (float)WIDTH / (float)HEIGHT;
final int N = 8;
final float THRESHOLD = 0.9;

float field[];

ForceField[] ff;

HashMap<Integer, PImage> tiles;

PImage getTile(float p0, float p1, float p2, float p3) {
  boolean b0 = p0 < THRESHOLD;
  boolean b1 = p1 < THRESHOLD;
  boolean b2 = p2 < THRESHOLD;
  boolean b3 = p3 < THRESHOLD;

  boolean i0 = b0 ^ b1;
  boolean i1 = b1 ^ b2;
  boolean i2 = b2 ^ b3;
  boolean i3 = b3 ^ b0;

  int i = (int(i0) << 12)
    | (int(i1) << 13)
    | (int(i2) << 14)
    | (int(i3) << 15);
  int ni = int(i0) + int(i1) + int(i2) + int(i3);

  assert(ni % 2 == 0);

  boolean empty = b0 && b1 && b2 && b3;
  boolean full = !(b0 || b1 || b2 || b3);

  if (full) {
    i |= 1 << 12;
  }

  int l0 = 0, l1 = 0, l2 = 0, l3 = 0;

  if (!full || !empty) {
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
      l3 = int(N * (THRESHOLD - p3) / (p0 - p3));
      assert(l3 < N && l3 >= 0);
      i |= l3 << 9;
    }
  }

  PImage tile = tiles.get(i);

  if (tile == null) {
    tile = createImage(N, N, RGB);

    if (full) {
      for (int k = 0; k < N * N; k++) {
        tile.pixels[k] = color(255);
      }
    } else {
      if (ni == 2) {
        println("generate tile:", i, l0, l1, l2, l3);
      } else if (ni == 4) {
      }

      for (int k = 0; k < N * N; k++) {
        tile.pixels[k] = color(0);
      }
    }

    tiles.put(i, tile);
  }

  return tile;
}

void setup() {
  size(640, 512);

  tiles = new HashMap<Integer, PImage>();

  field = new float[(WIDTH + N) * (HEIGHT + N)];
  ff = new ForceField[3];
  ff[0] = new ForceField(new Circle(0.1));
  ff[1] = new ForceField(new Circle(0.1));
  ff[2] = new ForceField(new Circle(0.1));

  // return sdEquilateralTriangle(p);
  // return opOnion(c, 0.1);
}

void bigPixel(int x, int y, color c) {
  for (int j = y * 2; j < (y + 1) * 2; j++) {
    for (int i = x * 2; i < (x + 1) * 2; i++) {
      pixels[j * width + i] = c;
    }
  }
}

void draw() {
  float t = frameCount / 60.0;

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
  for (int y = 0; y < HEIGHT; y += N) {
    for (int x = 0; x < WIDTH; x += N) {
      float p0 = field[y * WIDTH + x];
      float p1 = field[y * WIDTH + x + N];
      float p2 = field[(y + N) * WIDTH + x];
      float p3 = field[(y + N) * WIDTH + x + N];
      PImage tile = getTile(p0, p1, p2, p3);
      for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
          bigPixel(x + i, y + j, tile.pixels[j * N + i]);
        }
      }
    }
  }  
  updatePixels();
}
