int N = 1024;
int M = 1024;
int ROWS = 8;
int COLS = 8;
int near_z = 100;
int far_z = 300;
int far_y;

class Line {
  int xs, ys;
  int xe, ye;
  
  Line(int _xs, int _ys, int _xe, int _ye) {
    xs = _xs; ys = _ys; xe = _xe; ye = _ye;
  }
};

color randomColor() {
  return color(int(random(0, 255)),
               int(random(0, 255)),
               int(random(0, 255))); 
}

int horiz[];
Line vert[];
PImage cmap;
int hmap[];

void setup() {
  frameRate(60);
  size(640, 480);  

  cmap = new PImage(ROWS + 8, COLS + 8, RGB);
  for (int i = 0; i < cmap.width; i++)
    for (int j = 0; j < cmap.height; j++)
      cmap.set(i, j, randomColor());

  far_y = height * near_z / far_z;

  hmap = new int[height];

  int far_w = width * far_z / 256; 

  vert = new Line[N];
  horiz = new int[M];

  for (int i = 0; i < N; i++) {
    int x = round(far_w * lerp(-0.5, 0.5, float(i) / N));
    int far_x = width / 2 + x * 256 / far_z;
    int near_x = width / 2 + x * 256 / near_z;
    int near_y = height;
    
    if (near_x < 0) {
      near_y = far_y + (0 - far_x) * (height - far_y) / (near_x - far_x);
      near_x = 0;
    }
    if (near_x >= width) {
      near_y = far_y +  (width - 1 - far_x) * (height - far_y) / (near_x - far_x);
      near_x = width - 1;
    }
    
    vert[i] = new Line(far_x, far_y, near_x, near_y);
  }

  for (int i = 0; i < M; i++) {
    float z = lerp(far_z, near_z, float(i) / M);
    
    horiz[i] = height * near_z / round(z);
  }
  
  initOCS(3);

  palette[0] = #000000;
  palette[1] = #ffffff;
}

void copperLine(int x1, int y1, int x2, int y2, int i) {
  if (y1 > y2) {
    int xt = x1; x1 = x2; x2 = xt;
    int yt = y1; y1 = y2; y2 = yt;    
  }
  
  int dx = x2 - x1;
  int dy = y2 - y1;
  
  if (dy == 0)
    return;
    
  int di = dx / dy;
  int df = abs(dx) % dy;
  int xi = x1;
  int xf = 0;
  int s = (dx >= 0) ? 1 : -1;

  while (y1 < y2) {
    copper(xi & ~7, y1, i % 2, cmap.get(i, hmap[y1]));
    xi += di;
    xf += df;
    if (xf >= dy) {
      xf -= dy;
      xi += s;
    }
    y1++;
  }
}

void draw() {
  float t = frameCount / 60.0;

  copperClear();
  
  for (int i = 0; i < far_y; i++)
    copper(0, i, 0, lerpColor(#0080ff, #80ffff, float(i) / far_y));
  copper(0, far_y, 0, 0);

  bpl[0].zeros();

  int yo = int((1.0 + cos(t)) * M / 4);
  int xo = int((1.0 + sin(t)) * N / 4);
  int kyo = 7 - yo * ROWS / M;
  int kxo = 7 - xo * COLS / N;
  Line l0;

  /* Render stripes. */
  l0 = new Line(0, 0, width - 1, far_y);
  for (int k = COLS - 1; k >= 0; k--) {
    int xi = xo % (N / COLS) + k * (N / COLS);
    int col = k + kxo;
    Line l1 = vert[xi & (N - 1)];

    bpl[0].lineE(l1.xs, l1.ys, l1.xe, l1.ye);
    
    if ((col % 2 == 1) && (l0.xe == width - 1))
      bpl[0].lineE(width - 1, l0.ye, width - 1, l1.ye);
    
    l0 = l1;
  }
  
  bpl[0].fill();

  /* Calculate tile row for each line. */
  for (int k = 0, y0 = far_y; k <= 16; k++) {
    int row = k + kyo;
    int yi = yo % (M / ROWS) + k * (M / ROWS); 
    int y1 = horiz[(yi < M) ? yi : M - 1];    
    
    while (y0 < y1)
      hmap[y0++] = row;
  }

  /* Render tiles. */
  l0 = new Line(0, far_y, 0, far_y);
  for (int k = 0; k < COLS; k++) {
    int xi = xo % (N / COLS) + k * (N / COLS);
    int col = k + kxo;
    Line l1 = vert[xi & (N - 1)];

    if (l0.xe == 0)
      copperLine(0, l0.ye, 0, l1.ye, col - 1);

    copperLine(l1.xs, l1.ys, l1.xe, l1.ye, col);
    
    l0 = l1;
  }
 
  updateOCS();
  
  println(frameRate);
}
