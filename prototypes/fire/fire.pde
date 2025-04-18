color[] palette;
int[][] screen;

void setup() {
  size(320, 256);
  colorMode(HSB);
  palette = new color[256];
  screen = new int[320][256];

  /* Clear screen */
  for (int y = 0; y < 256; ++y) {
    for (int x = 0; x < 320; ++x) {
      screen[x][y] = 0;
    }
  }
  /* Create palette */
  for (int i = 0; i < 256; i+=1) {
    palette[i] = color(i/3, 255, constrain(i*3, 0, 255));
  }

}

void draw() {
  /* Randomize bottom rows */
  for (int x = 0; x < 320; ++x) {
    screen[x][255] = (int)random(0,255);
    screen[x][254] = (int)random(0,255);
  }

  /* Calculate new screen values */
  for (int y = 0; y < 254; ++y) {
    for (int x = 1; x < 319; ++x) {
      int i1 = screen[x][y+1];
      int i2 = screen[x][y+2];
      int i3 = screen[x-1][y+1];
      int i4 = screen[x+1][y+1];
      int idx = (i1+i2+i3+i4)*64/257;
      screen[x][y] = idx;
    }
  }

  /* Draw screen */
  for (int y = 0; y < 256; y++) {
    for (int x = 0; x < 320; x++) {
      //println(screen[x][y]);
      set(x, y, palette[screen[x][y]]);
    }
  }
}