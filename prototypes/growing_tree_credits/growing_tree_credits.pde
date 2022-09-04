final int ORIGIN_X = 186;
final int ORIGIN_Y = 45;

final int WIDTH = 320;
final int HEIGHT = 256;

final int CENTERX = WIDTH / 2;
final int CENTERY = HEIGHT / 2;

PShape credit;
PShape[] creditParts;
PImage bg;

PGraphics screen;

int index = 0;
boolean autoplay = true;

String shapePath = "elude.svg";
String svgData[];

void setup() {
  size(640, 556);
  noSmooth();
  textFont(loadFont("Monaco-16.vlw"));
  textSize(16);

  bg = loadImage("tree-bg.png");

  credit = loadShape(shapePath);
  int count = credit.getChildCount();
  creditParts = new PShape[count];
  for (int i = 0; i < count; i++) {
    creditParts[i] = credit.getChild(i);
  }
  frameRate(25);

  screen = createGraphics(WIDTH, HEIGHT);
  screen.noStroke();
}

void draw() {
  screen.beginDraw();

  screen.background(bg);
  for (int i = 0; i < index; i++) {
    screen.shape(creditParts[i], ORIGIN_X, ORIGIN_Y - 50);
  }
  screen.endDraw();

  if (autoplay) {
    if (index == creditParts.length) {
      index = 0;
    }
    index++;
  }
  status();

  amigaPixels();
}

void status() {
  String status = String.format(
    "Tree credits prototype: X: " + ORIGIN_X + " Y: " + ORIGIN_Y + "\n" +
    "[a]previous path, [d]next path, [r]reset, [q]tgl autoplay");

  fill(64);
  rect(2, 512, width-8, 88, 8);
  fill(255);
  text(status, 6, 528);
}

void keyPressed() {
  if (key == 'd') {
    if (index < creditParts.length -1) {
      index++;
    }
  } else if (key == 'a') {
    if (index > 1) {
      index--;
    }
  } else if (key == 'r') {
    index = 0;
  } else if (key == 'q') {
    autoplay = !autoplay;
  }
}
