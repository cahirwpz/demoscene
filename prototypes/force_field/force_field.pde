final int WIDTH = 320;
final int HEIGHT = 256;
final float RATIO = (float)WIDTH / (float)HEIGHT;

float field[];

ForceField[] ff;

void setup() {
  size(640, 512);
    
  field = new float[WIDTH * HEIGHT];
  ff = new ForceField[3];
  ff[0] = new ForceField(new Circle(0.1));
  ff[1] = new ForceField(new Circle(0.1));
  ff[2] = new ForceField(new Circle(0.1));

  // return sdEquilateralTriangle(p);
  // return opOnion(c, 0.1);
}

void draw() {
  float t = frameCount / 60.0;
  
  ff[0].angle = HALF_PI * t;
  ff[0].size.set(4.0 * RATIO, 4.0);
  ff[0].origin.set(1.25, 1.25 * sin(t * PI * 0.8));

  ff[1].angle = -HALF_PI * t;
  ff[1].size.set(4.0 * RATIO, 4.0);
  ff[1].origin.set(-1.25, 1.25 * cos(t * PI * 0.7));

  ff[2].angle = -HALF_PI * t;
  ff[2].size.set(4.0 * RATIO, 4.0);
  ff[2].origin.set(0.5 * sin(t * PI), 0.5 * cos(t * PI));

  for (int i = 0; i < field.length; i++)
    field[i] = 0.0;
  for (int i = 0; i < ff.length; i++)
    ff[i].render(field);
  
  background(0);
  loadPixels(); 
  for (int y = 0; y < height - 16; y += 16) {
    for (int x = 0; x < width - 16; x += 16) {
      int fy = y / 2;
      int fx = x / 2;
 
      float p0 = field[fy * WIDTH + fx];
      float p1 = field[fy * WIDTH + fx + 8];
      float p2 = field[(fy + 8) * WIDTH + fx];
      float p3 = field[(fy + 8) * WIDTH + fx + 8];
      float dl = (p2 - p0) / 16.0;
      float dr = (p3 - p1) / 16.0;
      
      for (int ty = 0; ty < 16; ty++) {
        for (int tx = 0; tx < 16; tx++) {
          float c = lerp(p0, p1, (float)tx / 16.0);
          c = (c >= 0.9) ? 1.0 : 0.0;
          c *= 255.0;
          pixels[(y + ty) * width + (x + tx)] = color(c, c, c);
        }
        p0 += dl;
        p1 += dr;
      }
    }
  }  
  updatePixels();
}
