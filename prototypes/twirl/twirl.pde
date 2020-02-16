PImage texture;
PImage polar;
int[] umap;
int[] vmap;
int[] offset;

final int SIZE = 512;

int wrap(int x) {
  int result = x % SIZE;
  return result < 0 ? result + SIZE : result;
}

void polarUV() {
  float max_radius = sqrt(sq(width / 2.0) + sq(height / 2.0));

  umap = new int[width * height];
  vmap = new int[width * height];

  for (int j = 0; j < height; j++) {
    float y = j - height / 2;

    for (int i = 0; i < width; i++) {
      float x = i - width / 2;

      float angle = atan2(y, x) / (2.0 * PI);
      float radius = sqrt(sq(x) + sq(y)) / max_radius;

      umap[i + width * j] = int(angle * (SIZE + 1));
      vmap[i + width * j] = int(radius * SIZE);
    }
  }
}

void textureToPolar() {
  polar = createImage(width, height, RGB);
  
  for (int r = 0; r < height; r++) {
    for (int a = 0; a < width; a++) {
      float angle = float(a) / SIZE * 2.0 * PI;
      float x = cos(angle) * r;
      float y = sin(angle) * r;

      int u = int(x + width / 2.0);
      int v = int(y + height / 2.0);

      polar.set(a, r, texture.get(wrap(u), wrap(v)));
    }
  }
}

void settings() {
  size(SIZE, SIZE);
}

void setup() {
  background(0);
  
  texture = loadImage("sunflowers.jpg");
  texture.loadPixels();
  
  polarUV();
  textureToPolar();
  
  offset = new int[SIZE];
}

void draw() {
  float angle = (float)frameCount / 128 * PI;
  float ds = sin(angle);

  for (int i = 0; i < SIZE; i++)
    offset[i] = int(0.5 * ds * (float)i + 256.0 * cos(angle));

  for (int y = 0; y < height; y++) {
   for (int x = 0; x < width; x++) {
     int v = vmap[x + width * y];
     int u = umap[x + width * y] + offset[wrap(v)];

     set(x, y, polar.get(wrap(u), wrap(v)));
   }
  }
}