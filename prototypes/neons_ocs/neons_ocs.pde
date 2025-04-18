final int H = 64;
final int A = 128;

void bplset(int x, int y, int v) {
  v &= 15;
  
  if (y < 0)
    return;
  if (y >= 256)
    return;

  for (int i = 0; i < 4; i++)
    bpl[i].set(x, y, boolean((v >> i) & 1));
}

void setup() {
  size(320, 256);
  frameRate(60);    

  initOCS(4);

  for (int i = -H; i < H; i++)
    for (int j = 0; j < height; j++)
      bplset(i + width / 2, j, j);

  int w = width / 2 - H;

  for (int i = 0; i < w; i++) {
    int y0 = (i * A) / w - A;
    int y1 = height - y0;
    
    for (int j = y0; j < y1; j++) {
      int v = int(float(j - y0) * 256.0 / (y1 - y0));
      bplset(i, j, v);
      bplset(width - i - 1, j, v);
    }
  }
}

void draw() {
  int t = frameCount / 2;
  
 for (int i = 0; i < 8; i++) {
   int i0 = (i + t) & 15;
   int i1 = (i + 8 + t) & 15;
   palette[i0] = lerpColor(#000000, #00ffff, float(i + 1) / 7);
   palette[i1] = lerpColor(#00ffff, #000000, float(i + 1) / 7);
 }
 
  updateOCS();
}
