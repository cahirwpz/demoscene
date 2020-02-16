PImage image;

int bumpmap[];
float light[];

final int FRAMERATE = 25;
final int SIZE = 512;

void settings() {
  image = loadImage("texture.png");
  image.loadPixels();
  
  size(image.width, image.height);
}

void setup() {
  frameRate(FRAMERATE);
  
  light = new float[SIZE * SIZE];

  for (int y = 0, i = 0; y < SIZE; y++) {
    for (int x = 0; x < SIZE; x++, i++) {
      float u = lerp(-4.0, 4.0, float(y) / SIZE);
      float v = lerp(-4.0, 4.0, float(x) / SIZE);
      
      light[i] = constrain(1.1 - sqrt(sq(u) + sq(v)), -1.0, 1.0);
    }
  }
 
  bumpmap = new int[height * width];

  for (int y = 0, i = 0; y < height; y++) {
    for (int x = 0; x < width; x++, i++) {
      int p = image.pixels[y * width + x];
      int down, right;
      
      if (y + 1 < height)
        down = image.pixels[(y + 1) * width + x];
      else
        down = image.pixels[x];
      
      if (x + 1 < width)
        right = image.pixels[y * width + (x + 1)];
      else
        right = image.pixels[y * width];

      float du = brightness(p) - brightness(down);
      float dv = brightness(p) - brightness(right);

      int u = constrain(int(1.5 * du), -255, 255);
      int v = constrain(int(1.5 * dv), -255, 255);
      
      bumpmap[i] = u * 512 + v;
    }
  }   
  
  loadPixels();
}

int brightness(int c, float b) {
  if (b >= 0.0)
    return lerpColor(c, color(255,255,255), b);
  else
    return lerpColor(c, color(0,0,0), -b);
}

void draw() {
  float f = float(frameCount) / FRAMERATE;
  
  int lx = (width - SIZE) / 2 + int(100 * sin(f));
  int ly = (height - SIZE) / 2 + int(100 * cos(f));
  int iy = int(50 * sin(f));
  int ix = int(50 * cos(f));
  
  int image_pos = iy * height + ix;
  int light_pos = -ly * SIZE - lx;
  
  for (int y = 0, i = 0; y < height; y++, light_pos += SIZE - width) {
    for (int x = 0; x < width; x++, i++, light_pos++) {
      int s = (image_pos + i) & 0xffff;
      int r = (bumpmap[s] + light_pos) & 0x3ffff;      
      pixels[i] = brightness(image.pixels[s], light[r]);
    }
  }
 
  updatePixels();
}