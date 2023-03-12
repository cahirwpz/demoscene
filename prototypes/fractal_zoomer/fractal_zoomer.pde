/*
 * Shamelessly copied from:
 * https://bel.fi/alankila/rotzoomer.html
 */

final int size = 32;
color tab[] = { #ffff00, #00ffff, #ff8000, #ff0080 };
int seed = 0;

/* shuffle image center around deterministically */  
int randomShift() {
  int shift = 0;
  seed++;
  for (int i = 0; i < 5; i++) {
    shift <<= 1;
    shift |= (seed >> i) & 1;
  }
  /* inject some long-term variation */
  return shift ^ (seed >> 5) & (size - 1);
}

void setup() {
  frameRate(25);
  size(512, 512);
  background(0);
}

boolean showVectors = false;
int rotation = 1;
int zoom = 1;

void draw() {  
  int shift = randomShift();

  if (showVectors) {
    background(0);
    stroke(255);
  } else {
    noStroke();
    /* XXX work out how to compensate for the bias.
     * the bias is really a function of where the stationary center
     * of the image is combined with the average properties of the
     * center shuffling mechanism. The center is defined as the point
     * which is moved as little as possible by the deformations. */
    int r = (rotation <= 0) ? size / 2 : 0;
    int k = 3;
    fill(tab[(int)random(4)]);
    rect(width / 2 + size / 2 - r - k, height / 2 + size / 2 - r - k, k, k);
  }

  /* now move graphics around. */
  PImage buffer = get();
  int n = width / size;

  for (int y = 0; y <= n; y++) {
    for (int x = 0; x <= n; x++) {
      int dx = x * size;
      int dy = y * size;
      int sx = dx + (n / 2 - y) * rotation + (n / 2 - x) * zoom;
      int sy = dy + (x - n / 2) * rotation + (n / 2 - y) * zoom;
      
      if (showVectors) {
        stroke(128); line(sx, sy, dx, dy);
        stroke(255); point(sx, sy);
        stroke(255, 0, 0); point(dx, dy);
      } else {
        int s = shift - size / 2;
        image(buffer.get(sx + s, sy + s, size, size), dx + s, dy + s);
      }
    }
  }
}

void keyPressed() {
  int r = (rotation <= 0) ? size / 2 : 0;
  int k = 32;
  fill(color(random(255), random(255), random(255)));
  rect(width / 2 + size / 2 - r - k,
       height / 2 + size / 2 - r - k,
       k * 2, k * 2);
}
