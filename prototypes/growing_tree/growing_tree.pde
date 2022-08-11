/* 
 * Adapted and modified version of:
 * http://www.openprocessing.org/sketch/144159
 */

final int WIDTH = 320;
final int HEIGHT = 256;

PGraphics screen;
int maxBranches;

void reset() {
  maxBranches = 0;

  branches = new ArrayList<Branch>();
  for (int i = 0; i < 1; i++)
    branches.add(new Branch(WIDTH / 2, HEIGHT));

  screen.beginDraw();
  screen.background(255);
  screen.endDraw();
}

void setup() {
  size(640, 512);
  noSmooth();

  screen = createGraphics(WIDTH, HEIGHT);
  screen.ellipseMode(CENTER);
  screen.noStroke();
  screen.fill(0);

  reset();
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

void draw() {
  if (branches.size() == 0) {
    println(maxBranches);
    reset();
  }

  if (maxBranches < branches.size()) {
    maxBranches = branches.size();
  }

  screen.beginDraw();
  screen.fill(0);
  screen.stroke(0);

  for (int i = branches.size() - 1; i >= 0; i--) {
    Branch b = branches.get(i);
    PVector loc = b.location;
    PVector lastLoc = b.lastLocation;

    screen.line(lastLoc.x, lastLoc.y, loc.x, loc.y);

    b.grow();

    if (b.maySplit()) {
      branches.add(new Branch(b));
    }

    if (b.isFinished()) {
      screen.noStroke();
      screen.fill(color(255, 0, 0));
      screen.ellipse(loc.x, loc.y, 10, 10);
      screen.stroke(0);
      branches.remove(i);
    }
  }
  screen.endDraw();

  amigaPixels();
}

void mousePressed() {
  reset();
}
