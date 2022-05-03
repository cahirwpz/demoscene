import java.util.Arrays;

final int near_z = 256;
final int far_z = 64;
final int stripe = 16;

final int shift = 4;
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

void drawLine(int y, int start, int stripe_w) {
  for (int x = 0; x < width; x++) {
    int stripe = (start + x << shift) / stripe_w; 
    int c = (stripe % 15 + 1) * 8 + 128;
    pixels[y * width + x] = color(c, c, c);
  }
}

void calcFloor() {
  /* Values from 8.0 to 16.0, unsigned 12.4 fixed point format */
  int[] stripeWidth = new int[HEIGHT];
  /* Values from 0 (normal) to 12 (dark) */
  int[] stripeLight = new int[HEIGHT];

  int near_w = stripe * near_z / 256;
  int far_w = stripe * far_z / 256;

  int level = 11;
  
  for (int y = 0; y < PART; y++) {
    int stripe_w = far_w << shift;
    float wobble = 0.25 * (1.0 - sin(y * 4 * PI / PART));
    // int start = (y - PART) + stripe_w * 15;
    int start = (int)(stripe_w * wobble) + stripe_w * 11 / 16;

    stripeWidth[y] = stripe_w;
    stripeLight[y] = level;

    drawLine(y, start, stripe_w);
  }
  
  for (int y = 0; y < PART; y++) {
    int stripe_w = (far_w << shift) + ((y << shift) * (near_w - far_w) / PART);
    int start = 0;

    stripeWidth[PART + y] = stripe_w;
    stripeLight[PART + y] = level - (12 * y) / PART;

    drawLine(PART + y, start, stripe_w);
  }

  saveArrayToC(stripeLight, "stripeLight");
  saveArrayToC(stripeWidth, "stripeWidth");
}

void saveArrayToC(int[] arr, String name) {
  String header = "/* Generated automatically from "
                  "prototypes/floor_data/floor_data.pde */";
  String nl = System.lineSeparator();
  String out = new String(header + nl + "short " + name + "[] = { " + nl);
  String line = new String();
  for (int i = 0; i < arr.length; i++) {
    String lineTest = new String(line + arr[i] + ", ");
    // Break the line if adding a new value would go over 80 char limit
    if (lineTest.length() >= 80) {
      out += line + nl;
      line = "";
    }
    line += new String(arr[i] + ", ");
  }
  out += "};";
  
  PrintWriter wr = createWriter(name + ".c");
  wr.println(out);
  wr.close();
}
