import java.util.Arrays;

final int near_z = 256;
final int far_z = 64;
final int stripe = 16;

final int WIDTH = 320;
final int HEIGHT = 256;
final int PART = HEIGHT / 2;

void settings() {
  size(WIDTH + 16, HEIGHT + 1);
  noSmooth();
}

void setup() {
  background(0);
  loadPixels();
  calcFloor();
  updatePixels(); 
  save("floor.png");
}

/* Convert integer to Q12.4 fixed point format */
static int Q(int i) {
  return i << 4;
}

void drawLine(int y, int start, int stripe_w) {
  for (int x = 0; x < width; x++) {
    int stripe = Q(start + x) / stripe_w; 
    int c = (stripe % 15 + 1) * 8 + 128;
    pixels[y * width + x] = color(c, c, c);
  }
}

void calcFloor() {
  /* Values from 8.0 to 16.0, unsigned Q12.4 fixed point format */
  int[] stripeWidth = new int[HEIGHT];
  /* Values from 0 (normal) to 12 (dark) */
  int[] stripeLight = new int[HEIGHT];

  int near_w = stripe * near_z / 256;
  int far_w = stripe * far_z / 256;

  int level = 11;
  
  for (int y = 0; y < PART; y++) {
    int stripe_w = Q(far_w);
    float wobble = 0.25 * (1.0 - sin(y * 4 * PI / PART));
    // int start = (y - PART) + stripe_w * 15;
    int start = (int)(stripe_w * wobble) + stripe_w * 11 / 16;

    stripeWidth[y] = stripe_w;
    stripeLight[y] = level;

    drawLine(y, start, stripe_w);
  }
  
  for (int y = 0; y < PART; y++) {
    int stripe_w = Q(far_w) + (Q(y) * (near_w - far_w) / PART);
    int start = 0;

    stripeWidth[PART + y] = stripe_w;
    stripeLight[PART + y] = level - (12 * y) / PART;

    drawLine(PART + y, start, stripe_w);
  }

  saveArrayToC(stripeLight, "stripeLight");
  saveArrayToC(stripeWidth, "stripeWidth");
}

void saveArrayToC(int[] arr, String name) {
  PrintWriter wr = createWriter(name + ".c");
  
  wr.println("/* Generated automatically from " +
             "prototypes/floor_data/floor_data.pde */");
  wr.println("short " + name + "[] = { ");

  String line = "";
  for (int i = 0; i < arr.length; i++) {
    String elem = arr[i] + ", ";
    // Break the line if adding a new value would go over 80 char limit
    if (line.length() + elem.length() >= 80) {
      wr.println(line);
      line = "";
    }
    line += elem;
  }
 
  wr.println("};");
  wr.close();
}
