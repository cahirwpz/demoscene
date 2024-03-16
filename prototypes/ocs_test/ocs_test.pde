final int WIDTH = 320;
final int HEIGHT = 256;
final int DEPTH = 5;

OrigChipSet ocs;
Bitplane screen[];
Sprite sprite[];

void settings() {
  size(OcsRasterWidth, OcsRasterHeight);
}

void fillSprite(Sprite spr) {
  for (int y = 0; y < 16; y++)
    for (int x = 0; x < 16; x++)
      spr.set(y + 1, x, (y / 4) % 4);
}

void fillSprite2(Sprite spr) {
  for (int y = 0; y < 16; y++)
    for (int x = 0; x < 16; x++)
      spr.set(y + 1, x, y % 4);
}

void setup() {
  frameRate(OcsFrameRate);

  ocs = new OrigChipSet();

  for (int i = 0; i < 8; i++)
    ocs.setColor(i, color(i * 16 + 128, i * 16 + 128, i * 16 + 128));
  for (int i = 0; i < 8; i++)
    ocs.setColor(i + 8, color(i * 16 + 128, 0, i * 16 + 128));
  for (int i = 0; i < 8; i++)
    ocs.setColor(i + 16, color(i * 16 + 128, i * 16 + 128, 0));
  for (int i = 0; i < 8; i++)
    ocs.setColor(i + 24, color(0, i * 16 + 128, i * 16 + 128));

  screen = ocs.chip.allocBitmap(WIDTH, HEIGHT, DEPTH);
 
  /*
  for (int i = 0; i < height; i++) {
    float a = 8 * TWO_PI * float(i) / height;
    float c = constrain(sin(a) * 128 + 128, 0, 255);
    copper(0, i, 0, color(0, 0, c));
  }
  */
  
  sprite = new Sprite[8];

  for (int i = 0; i < 8; i++) {
    sprite[i] = new Sprite(16, 1);
    sprite[i].header(0, HPOS(0), VPOS(0), 16, i == 7);
    if (i == 6)
      fillSprite2(sprite[i]);
    else
      fillSprite(sprite[i]);
    sprite[i].end(17);
  }
  
  for (int i = 0; i < DEPTH; i++) {
    Blit.lineE(screen[i], i * 32 + 20, 40, i * 32 + 100, 180);
    Blit.lineE(screen[i], 100 + i * 32, 180, i * 32 + 180, 40);
    Draw.circleE(screen[i], 100 + i * 32, 180, 64);
    Blit.fill(screen[i]);
  }  
}

void draw() {
  float t = 0.5 * frameCount / 60.0;
  
  for (int i = 0; i < 6; i++) {
    int x = (ocs.width() - 16) / 2 + int(sin(t) * 64) - 64 + 16 * i;
    sprite[i].update(0, HPOS(x), VPOS(0));
  }
  
  for (int i = 6; i < 8; i++) {
    int x = (ocs.width() - 16) / 2 + int(sin(t) * 64) - 64 + 16 * 6;
    sprite[i].update(0, HPOS(x), VPOS(0));
  }
  
  ocs.setupScreen(screen, 0, DEPTH);
  ocs.setupDisplayWindow(HPOS(0), VPOS(0), WIDTH, HEIGHT);
  ocs.setupBitplaneFetch(HPOS(0), WIDTH);
  ocs.update();
}
