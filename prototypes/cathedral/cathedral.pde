PImage imgLight;
PImage imgDark;
PImage mask;
PImage generatedMask;

void updateMask() {
  mask.loadPixels();
  generatedMask.loadPixels();
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      int loc = x+ y * width;
      int pixel = generatedMask.pixels[loc];
      float r = red(pixel);
      float g = green(pixel);
      float b = blue(pixel);
      generatedMask.pixels[loc] = pixel != color(255) ? color(r+frameCount/50, g+frameCount/50, b+frameCount/50) : color(255);
    }
  }
  generatedMask.updatePixels();
  imgLight.mask(generatedMask);
}

void copyPixels() {
  mask.loadPixels();
  generatedMask.loadPixels();
  for (int i = 0; i < generatedMask.pixels.length; i++) {
    generatedMask.pixels[i] = mask.pixels[i];
  }
  generatedMask.updatePixels();
}

void setup() {
  size(320, 256);
  imgLight = loadImage("light.png");
  imgDark = loadImage("dark.png");
  mask = loadImage("mask.png");
  generatedMask = createImage(320, 256, ALPHA);
  copyPixels();
  imageMode(CENTER);
}

void draw() {
  background(0);
  updateMask();
  image(imgDark, width/2, height/2);
  image(imgLight, width/2, height/2);
}
