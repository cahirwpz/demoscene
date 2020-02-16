PImage texture;

void setup() {
  size(512, 512);
  background(0);
  
  texture = loadImage("lava.png");
  texture.loadPixels();
  
  loadPixels();
} 

void draw() {
  float angle = (float)frameCount / 128 * PI;
  float scale = sin(angle) + 1.0;

  /* step downward vector */
  float dp = cos(angle) * scale;
  float dq = sin(angle) * scale;

  /* step forward vector */
  float du = cos(angle + PI / 2) * scale;
  float dv = sin(angle + PI / 2) * scale;

  float p = 0.0f;
  float q = 0.0f;

  /* center horizontally */
  p -= du * width / 2.0f;
  q -= dv * width / 2.0f;

  /* center vertically */
  p -= dp * height / 2.0f;
  q -= dq * height / 2.0f;
  
  int k = 0;

  for (int y = 0; y < height; y++) {
    float u = p;
    float v = q;

    for (int x = 0; x < width; x++, k++) {
      int i = (int)u % texture.width;
      int j = (int)v % texture.height;

      if (i < 0)
        i += texture.width;
      if (j < 0)
        j += texture.width;

      pixels[k] = texture.pixels[i + j * texture.width];

      u += du; v += dv;
    }
    
    p += dp; q += dq;
  }
  
  updatePixels();
}