final int size = 16;

ArrayList<VectorField> fields = new ArrayList<VectorField>();

boolean showVectors = false;
boolean mouseDraw = false;
color mouseColor;
int active = 1;

void setup() {
  size(512, 512);
  background(0);

  VectorField f1 = new TestField1(512 / size + 1, 512 / size + 1);
  f1.calc(PI/3, PI, -1.0, 0.25);
  fields.add(f1);

  VectorField f2 = new TestField2(512 / size + 1, 512 / size + 1);
  f2.calc(-PI/2, PI/2, 0, PI/2);
  fields.add(f2);
}

void renderVectors() {
  background(0);
  stroke(255);
  
  VectorField f = fields.get(active);

  for (int y = 0; y < f.h; y++) {
    for (int x = 0; x < f.w; x++) {
      PVector v = f.get(x, y);
      int dx = x * size;
      int dy = y * size;
      int sx = int((x + v.x) * size);
      int sy = int((y + v.y) * size);

      stroke(128); line(sx, sy, dx, dy);
      stroke(255); point(sx, sy);
      stroke(255, 0, 0); point(dx, dy);
    }
  }
}

void renderFrame() {
  VectorField f = fields.get(active);
  int shift = int(random(0, size));

  noStroke();

  /* now move graphics around. */
  PImage buffer = get();
  buffer.loadPixels();

  loadPixels();
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
        
        if (tsy < 0 || tsy >= height || tdy < 0 || tdy >= height)
          continue;

        for (int i = 0; i < size; i++) {
          int tsx = sx + i;
          int tdx = dx + i;
          
          if (tsx < 0 || tsx >= width || tdx < 0 || tdx >= width)
            continue;
        
          int src = tsy * width + tsx;
          int dst = tdy * width + tdx;
          pixels[dst] = buffer.pixels[src];
        }
      }
    }
  }
  updatePixels();

  if (mouseDraw) {
    fill(palette[frameCount % 256]);
    rect(mouseX - size, mouseY - size, 2 * size, 2 * size);
  }
}

void draw() {  
  if (showVectors) {
    renderVectors();
  } else {
    renderFrame();
  }
}

void keyPressed() {
  if (key == 'c') {
    background(0);
  } else if (key == 'v') {
    showVectors = !showVectors;
  }
}

void mousePressed() {
  mouseDraw = true;
}

void mouseReleased() {
  mouseDraw = false;
}
