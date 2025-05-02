/* Tropical Fruit Punch */
color pal[] = { #146152, #44803F, #B4CF66, #FFEC5C, #FF5A33 };

color randomColor() {
  int i = int(random(0, pal.length - 1));
  return lerpColor(pal[i], pal[i + 1], random(0.0, 1.0));
}

final float RATIO = 0.5; // sqrt(3.0) / 2.0;
final float SX = 16.0 * RATIO;
final float SY = 8.0;

void drawHexes(int w, int h) {
  int nj = int(h / SY) - 1;
  int ni = int(w / SX) - 1;

  background(0);
  // stroke(0);
  noStroke();

  for (int j = 0; j < nj; j++) {
    float py = SY * j;
    
    for (int i = 0; i < ni; i++) {
      float bx, px;
      
      if (i % 3 == 1 && j % 2 == 0) {
        bx = SX * i - SX;
        px = SX * i - SX / 2;
      } else if (i % 3 == 2 && j % 2 == 1) {
        bx = SX * i - SX / 2;
        px = SX * i;
      } else {
        continue;
      }
        
      fill(randomColor());
      beginShape();
      vertex(px, py);
      vertex(px + SX, py);
      vertex(bx + SX * 2, py + SY);
      vertex(px + SX, py + SY * 2);
      vertex(px, py + SY * 2);
      vertex(bx, py + SY);
      endShape(CLOSE);
    }
  }
}

void doublePixels() {
  loadPixels();
  for (int j = height / 2 - 1; j >= 0; j--) {
    for (int i = width / 2 - 1; i >= 0; i--) {
      int c = pixels[j * width + i];
      pixels[(j * 2 + 0) * width + (i * 2 + 0)] = c;
      pixels[(j * 2 + 1) * width + (i * 2 + 0)] = c;
      pixels[(j * 2 + 0) * width + (i * 2 + 1)] = c;
      pixels[(j * 2 + 1) * width + (i * 2 + 1)] = c;
    }
  }
  updatePixels();
}

void setup() {
  size(640, 512);
  frameRate(5);
  noSmooth();
}

void draw() {
  drawHexes(width / 2, height / 2);
  doublePixels();
}
