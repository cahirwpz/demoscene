/*
 * Adapted and modified version of:
 * https://openprocessing.org/sketch/1339477
 */

final int WIDTH = 320;
final int HEIGHT = 256;

ArrayList<Circle> circles = new ArrayList<Circle>();
PGraphics screen;

void setup() {
  size(640, 556);
  background(0);
  noSmooth();

  textFont(loadFont("Monaco-16.vlw"));
  textSize(16);

  screen = createGraphics(WIDTH, HEIGHT);
}

void amigaPixels() {
  loadPixels();
  screen.loadPixels();
  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {
      int c = screen.pixels[j * WIDTH + i];
      c &= 0xF0F0F0;
      c |= c >> 4;
      c |= 0xFF000000;
      pixels[(j * 2 + 0) * width + (i * 2 + 0)] = c;
      pixels[(j * 2 + 1) * width + (i * 2 + 0)] = c;
      pixels[(j * 2 + 0) * width + (i * 2 + 1)] = c;
      pixels[(j * 2 + 1) * width + (i * 2 + 1)] = c;
    }
  }
  screen.updatePixels();
  updatePixels();
}

void status() {
  String status = String.format(
    "Sea Anemone prototype\n" +
    "[c]lear, [s]olid, [r]andomize colors");

  fill(64);
  rect(2, 512, width-8, 88, 8);
  fill(255);
  text(status, 6, 528);
}

void draw() {
  circles.add(new Circle());

  screen.beginDraw();
  for (int i = circles.size() - 1; i >= 0; i--) {
    Circle c = circles.get(i);
    c.show(screen);
    c.move();
    if (c.isDead()) {
      circles.remove(i);
    }
  }
  screen.endDraw();

  amigaPixels();
  status();
}

void reset() {
  screen.beginDraw();
  screen.background(0);
  screen.endDraw();

  circles.clear();
  circles.add(new Circle());
}

void keyPressed() {
  if (key == 'c') {
    reset();
  } else if (key == 's') {
    solid = !solid;
    reset();
  } else if (key == 'r') {
    armLight = color(random(255), random(255), random(255));
    armDark = color(random(255), random(255), random(255));
    reset();
  }
}
