final int size = 16;

PGraphics screen;
boolean showDebug = true;
boolean showBoundingBoxes = false;
boolean mouseDraw = false;
boolean motionBlur = false;
color mouseColor;
int active = 0;

void setup() {
  size(512, 578);
  background(0);
  noStroke();

  textFont(loadFont("Monaco-16.vlw"));
  textSize(16);

  screen = createGraphics(512, 512);

  addVectorField(new TestField1(), PI/3, PI, -1.0, 0.25);
  addVectorField(new TestField2(), -PI/2, PI/2, 0, PI/2);
  addVectorField(new TestField3(), -PI, PI, -PI/2, PI/2);
  addVectorField(new TestField4(), -2 * PI, 2 * PI, -PI, PI);
  addVectorField(new TestField5(), -1.0, 1.0, -1.0, 1.0);
  addVectorField(new TestField6(), -PI / 4, PI / 4, -PI, PI);
  addVectorField(new TestField7(), PI / 8, PI / 2, -PI / 4, PI / 4);
  addVectorField(new TestField8(), -PI / 2, PI / 2, -PI / 2, PI / 2);
  addVectorField(new TestField8(), -PI / 3, PI / 3, PI / 3, PI * 1.5);
  addVectorField(new TestField9(), -1.0, 1.0, -1.0, 1.0);

  active = fields.size() - 1;
}

void renderVectors() {
  VectorField f = fields.get(active);

  screen.beginDraw();
  screen.background(0);
  screen.stroke(255);

  for (int y = 0; y < f.h; y++) {
    for (int x = 0; x < f.w; x++) {
      PVector v = f.get(x, y);
      int dx = x * size;
      int dy = y * size;
      int sx = int((x + v.x) * size);
      int sy = int((y + v.y) * size);

      screen.stroke(128);
      screen.line(sx, sy, dx, dy);
      screen.stroke(255);
      screen.point(sx, sy);
      screen.stroke(255, 0, 0);
      screen.point(dx, dy);
    }
  }
  screen.endDraw();

  image(screen, 0, 0);
}

void renderBoxes() {
  VectorField f = fields.get(active);

  screen.beginDraw();
  screen.background(0);
  screen.stroke(255);
  for (int y = 0; y < f.h; y++) {
    for (int x = 0; x < f.w; x++) {
      PVector v = f.get(x, y);
      int dx = x * size;
      int dy = y * size;
      int sx = int((x + v.x) * size);
      int sy = int((y + v.y) * size);

      screen.stroke(128);
      screen.line(dx, dy, dx+size, dy);
      screen.line(dx+size, dy, dx+size, dy+size);
      screen.stroke(255);
      screen.line(sx, sy, sx+size, sy);
      screen.line(sx+size, sy, sx+size, sy+size);
    }
  }
  screen.endDraw();

  image(screen, 0, 0);
}

void moveTiles() {
  VectorField f = fields.get(active);
  int shift = int(random(0, size));

  loadPixels();
  screen.beginDraw();
  screen.loadPixels();

  for (int y = 0; y < f.h; y++) {
    for (int x = 0; x < f.w; x++) {
      PVector v = f.get(x, y);
      int dx = x * size;
      int dy = y * size;
      int sx = int((x + v.x) * size);
      int sy = int((y + v.y) * size);

      int s = shift - size / 2;

      for (int j = 0; j < size; j++) {
        int tsy = sy + s + j;
        int tdy = dy + s + j;

        if (tsy < 0 || tsy >= screen.height ||
            tdy < 0 || tdy >= screen.height)
          continue;

        for (int i = 0; i < size; i++) {
          int tsx = sx + i;
          int tdx = dx + i;

          if (tsx < 0 || tsx >= screen.width ||
              tdx < 0 || tdx >= screen.width)
            continue;

          int src = tsy * width + tsx;
          int dst = tdy * width + tdx;
          int p = pixels[src];
          screen.pixels[dst] = p;
        }
      }
    }
  }

  screen.updatePixels();
  screen.endDraw();
  updatePixels();
}

void renderFrame() {
  moveTiles();

  if (mouseDraw) {
    screen.beginDraw();
    screen.fill(palette[frameCount % 256]);
    screen.noStroke();
    screen.rect(mouseX - size, mouseY - size, 2 * size, 2 * size);
    screen.endDraw();
  }

  if (motionBlur) {
    tint(255, 128);
  } else {
    noTint();
  }

  image(screen, 0, 0);
}

void status() {
  String status = String.format(
    "Tile Mover prototype\n" +
    "[c]lear, show [d]ebug, toggle [b]ounding box view\n, [n]ext vector field\n" +
    "[m]otion blur, (LMB) draw rectangle");

  fill(64);
  rect(2, 512, width-8, 88, 8);
  fill(255);
  text(status, 6, 528);
}

void draw() {  
  if (showDebug) {
    if (showBoundingBoxes) {
    renderBoxes(); 
    } else {
      renderVectors();
    }
  } else {
    renderFrame();
  }
  status();
}

void keyPressed() {
  if (key == 'c') {
    fill(0);
    rect(0, 0, screen.width, screen.height);
    screen.beginDraw();
    screen.background(0);
    screen.endDraw();
  } else if (key == 'd') {
    showDebug = !showDebug;
  } else if (key == 'b') {
    showBoundingBoxes = !showBoundingBoxes;
  } else if (key == 'm') {
    motionBlur = !motionBlur;
  } else if (key == 'n') {
    active = (active + 1) % fields.size();
  }
}

void mousePressed() {
  mouseDraw = true;
}

void mouseReleased() {
  mouseDraw = false;
}
