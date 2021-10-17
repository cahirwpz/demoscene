PImage gradient;
PImage twister;

void setup() {
  size(256, 256, P3D); 
  noStroke();
  noSmooth();
  
  twister = createImage(256, 256, RGB);
  twister.loadPixels();
  
  // generate gradient texture
  gradient = new PImage(256, 256);
  gradient.loadPixels();
  for (int y = 0; y < 256; y++)
    for (int x = 0; x < 256; x++) {
      int c = (int)(map(x, 0, 256, 0, 16) * 8);
      c = constrain(c + 128, 128, 255);
      gradient.set(x, y, (c << 16) | (c << 8) | c);
    }
    
  ((PGraphicsOpenGL)g).textureSampling(2);
}

void draw() {
  background(0);
  translate(width / 2, height / 2);
  rotateY((frameCount & 255) * PI / 512.0);
  // scale(22.5, 64, 22.5); // 64
  scale(43.5, 64, 43.5); // 128

  beginShape(QUADS);

  texture(gradient);

  vertex(-1,  1,  1,   0, 255);
  vertex( 1,  1,  1, 255, 255);
  vertex( 1, -1,  1, 255,   0);
  vertex(-1, -1,  1,   0,   0);

  vertex( 1,  1,  1,   0, 255);
  vertex( 1,  1, -1, 255, 255);
  vertex( 1, -1, -1, 255,   0);
  vertex( 1, -1,  1,   0,   0);

  vertex( 1,  1, -1,   0, 255);
  vertex(-1,  1, -1, 255, 255);
  vertex(-1, -1, -1, 255,   0);
  vertex( 1, -1, -1,   0,   0);

  vertex(-1,  1, -1,   0, 255);
  vertex(-1,  1,  1, 255, 255);
  vertex(-1, -1,  1, 255,   0);
  vertex(-1, -1, -1,   0,   0);

  endShape();
  
  loadPixels();
  
  // take the middle line and put it into consecutive line in twister buffer
  for (int i = 0; i < width; i++)
    twister.pixels[(frameCount & 255) * width + i] = pixels[height / 2 * width + i];
}

void keyPressed() {
  twister.save("twister.png");
}
