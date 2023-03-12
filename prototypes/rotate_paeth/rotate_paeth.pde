PImage rot0, rot1, rot2, rot3;

void setup() {
  size(512, 384);

  rot0 = createImage(width, height, ALPHA);
  rot1 = createImage(width, height, ALPHA);
  rot2 = createImage(width, height, ALPHA);
  rot3 = createImage(width, height, ALPHA);

  rot0.loadPixels();
  rot1.loadPixels();
  rot2.loadPixels();
  rot3.loadPixels();

  PImage img = loadImage("lena.png");
  
  int xc = (width - img.width) / 2;
  int yc = (height - img.height) / 2;
  
  rot0.copy(img,
            0, 0, img.width, img.height,
            xc, yc, img.width, img.height);

}

void shear_x(PImage dst, PImage src, float alpha) {
  float x = - alpha * height / 2;
  int k = 0;
  
  for (int y = 0; y < height; y++, x += alpha, k += width) {
    int dk, sk, n, xi;

    xi = (int)x;

    if (xi < 0) {
      sk = k; dk = k - xi; n = width + xi;
    } else {
      sk = k + xi; dk = k; n = width - xi;
    }

    while (--n >= 0)
      dst.pixels[dk + n] = src.pixels[sk + n];
  }
}

void shear_y(PImage dst, PImage src, float beta) {
  float y = - beta * width / 2;
  int k = 0;
  
  for (int x = 0; x < width; x++, y += beta, k++) {
    int dk, sk, n, yi;

    yi = (int)y;

    if (yi < 0) {
      sk = k; dk = k - yi * width; n = height + yi;
    } else {
      sk = k + yi * width; dk = k; n = height - yi;
    }

    for (int i = 0; --n >= 0; i += width)
      dst.pixels[dk + i] = src.pixels[sk + i];
  }
}

void draw() {
  int value = constrain(mouseX, 0, width);
  int degrees = (180 * value) / width - 90;

  float phi = radians(degrees);
  float alpha = -tan(phi / 2.0);
  float beta = sin(phi);  

  background(0);

  shear_x(rot1, rot0, alpha);
  shear_y(rot2, rot1, beta);
  shear_x(rot3, rot2, alpha);
  
  rot3.updatePixels();
  image(rot3, 0, 0);

  text(str(degrees), 10, 10, 70, 80);
}

