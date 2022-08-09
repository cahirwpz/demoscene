final int size = 16;

ArrayList<VectorField> fields = new ArrayList<VectorField>();

boolean showVectors = false;
boolean mouseDraw = false;
color mouseColor;
int active = 0;

void setup() {
  size(512, 512);
  background(0);

  VectorField f1 = new TestField1();
  f1.calc(PI/3, PI, -1.0, 0.25);
  fields.add(f1);

  VectorField f2 = new TestField2();
  f2.calc(-PI/2, PI/2, 0, PI/2);
  fields.add(f2);
  
  VectorField f3 = new TestField3();
  f3.calc(-PI, PI, -PI/2, PI/2);
  fields.add(f3);
  
  VectorField f4 = new TestField4();
  f4.calc(-2 * PI, 2 * PI, -PI, PI);
  fields.add(f4);
  
  VectorField f5 = new TestField5();
  f5.calc(-1.0, 1.0, -1.0, 1.0);
  fields.add(f5);
  
  VectorField f6 = new TestField6();
  f6.calc(-PI / 4, PI / 4, -PI, PI);
  fields.add(f6);
  
  VectorField f7 = new TestField7();
  f7.calc(PI / 8, PI / 2, -PI / 4, PI / 4);
  fields.add(f7);

  VectorField f8 = new TestField8();
  f8.calc(-PI / 2, PI / 2, -PI / 2, PI / 2);
  fields.add(f8);

  VectorField f9 = new TestField8();
  f9.calc(-PI / 3, PI / 3, PI / 3, PI * 1.5);
  fields.add(f9);

  active = fields.size() - 1;
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
