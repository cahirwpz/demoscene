import java.util.Arrays;

final int max_z = 256;
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

/* Convert float to Q12.4 fixed point format */
static int QF(float f) {
  return (int)(f * 16.0);
}

static int stripeNum(float f, int n) {
    int r = (int)f % n;
    if (r < 0)
      r += n;
    return r + 1;
}

void drawLine(int y, float offset, float stripe_w) {
  for (int x = 0; x < width; x++) {
    float snum = x / stripe_w + offset;
    int c = stripeNum(snum, 15) * 8 + 128;
    pixels[y * width + x] = color(c, c, c);
  }
}

void calcFloor() {
  /* Values from 8.0 to 16.0, unsigned Q12.4 fixed point format */
  int[] stripeWidth = new int[HEIGHT];
  /* Values from 0 (normal) to 12 (dark) */
  int[] stripeLight = new int[HEIGHT];

  int light = 11;
  
  for (int y = 0; y < PART; y++) {
    float z = far_z;
    float stripe_w = stripe * z / max_z;
    float wobble = 4 * sin(4 * PI * (PART - y) / PART);

    stripeWidth[y] = QF(stripe_w);
    stripeLight[y] = light;

    drawLine(y, wobble, stripe_w);
  }
  
  for (int y = 0; y < PART; y++) {
    float z = far_z + (near_z - far_z) * y / PART;
    float stripe_w = stripe * z / max_z;

    stripeWidth[PART + y] = QF(stripe_w);
    stripeLight[PART + y] = light - (12 * y) / PART;

    drawLine(PART + y, 0, stripe_w);
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
