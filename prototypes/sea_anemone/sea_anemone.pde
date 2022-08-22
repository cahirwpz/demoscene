/*
 * Adapted and modified version of:
 * https://openprocessing.org/sketch/1339477
 */
import java.util.Collections;

final int WIDTH = 320;
final int HEIGHT = 256;

ArrayList<Arm> arms = new ArrayList<Arm>();
CircleCache cache = new CircleCache(); 
PGraphics screen;
PGraphics buffer;

void setup() {
  size(640, 556);
  background(0);
  noSmooth();

  textFont(loadFont("Monaco-16.vlw"));
  textSize(16);

  screen = createGraphics(WIDTH, HEIGHT);
  buffer = createGraphics(RADIUS * 2, RADIUS * 2);
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
  arms.add(new Arm());

  // If `arms` could a cyclic array sorted by ascending `Arm.s`, then we could
  // insert new arm into sorted array saving ourselves from full sort.
  Collections.sort(arms);

  screen.beginDraw();
  for (int i = arms.size() - 1; i >= 0; i--) {
    Arm a = arms.get(i);
    a.show(screen);
    a.move();
    if (a.isDead()) {
      arms.remove(i);
      println("dead", i);
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

  arms.clear();
  arms.add(new Arm());
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
