int[] s4;
int[] s8;
int[] s16;
int[] xbuf, ybuf;
int a1, a2, a3, a4, a5;
color[] palette;

final int psize = 512;

int wrap(int x) {
  int result = x % psize;
  return result < 0 ? result + psize : result;
}

void setup() {
  size(512, 512);
  background(0);

  palette = new color[256];

  for(int x = 0; x < 256; x++) {
    int r = int(128.0 + 128 * sin(PI * x / 32.0));
    int g = int(128.0 + 128 * sin(PI * x / 64.0));
    int b = int(128.0 + 128 * sin(PI * x / 128.0));
    palette[x] = color(r, g, b);
  }

  /* calculate sine tables for plasma */
  s4 = new int[psize];
  s8 = new int[psize];
  s16 = new int[psize];

  xbuf = new int[width];
  ybuf = new int[height];

  for (int i = 0; i < psize; i++) {
    float a = (float)i * PI / (psize / 2.0); 

    s4[i] = (int)(8.0 * sin(a) * PI);
    s8[i] = (int)(16.0 * cos(a) * PI);
    s16[i] = (int)(32.0 * sin(a) * PI);
  }
} 

void draw() {
  int _a1 = a1;
  int _a2 = a2;
  int _a3 = a3;

  for (int i = 0; i < width; i++) {
    xbuf[i] = s16[wrap(_a1)] + s8[wrap(_a2)] + s4[wrap(_a3)];
    _a1 += 1; _a2 += 2; _a3 += 4;
  }

  a1 += 1; a2 += 3; a3 += 2;

  int _a4 = a4;
  int _a5 = a5;

  for (int i = 0; i < height; i++) {
    ybuf[i] = s16[wrap(_a4)] + s8[wrap(_a5)];
    _a4 += 1; _a5 += 3;
  }

  a4 += 1; a5 -= 1;

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      int v = (xbuf[i] + ybuf[j]) & 255;
      set(i, j, palette[v]);
    }
  }
}