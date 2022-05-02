import java.util.Arrays;

final int near_z = 256;
final int far_z = 64;
final int stripe = 16;

final int shift = 4;
final int WIDTH = 320;
final int HEIGHT = 256;
final int PART = HEIGHT / 2;

int[] stripeWidth = new int[HEIGHT];
int[] stripeLight = new int[HEIGHT];

PrintWriter wr;

void settings() {
  size(WIDTH + 16, HEIGHT + 1);
  noSmooth();
}

void setup() {
  int near_w = stripe * near_z / 256;
  int far_w = stripe * far_z / 256;
  
  background(0);
  loadPixels();
  
  int line = 0;
  
  for (int y = 0; y < PART; y++, line += width) {
    int stripe_w = far_w << shift;
    // int start = (y - PART) + stripe_w * 15;
    int start = (int)(stripe_w * 0.25 * (1.0 - sin(y * 4 * PI / PART))) + stripe_w * 11 / 16;

    stripeWidth[y] = stripe_w;

    for (int x = 0; x < width; x++) {
      int stripe = (start + x << shift) / stripe_w; 
      int c = (stripe % 15 + 1) * 8 + 128;
      pixels[line + x] = color(c, c, c);
    }
  }
  
  for (int y = 0; y < PART; y++, line += width) {
    int stripe_w = (far_w << shift) + ((y << shift) * (near_w - far_w) / PART);
    int start = 0;

    stripeWidth[PART+y] = stripe_w;

    for (int x = 0; x < width; x++) {
      int stripe = start + (x << shift) / stripe_w; 
      int c = (stripe % 15 + 1) * 8 + 128;
      pixels[line + x] = color(c, c, c);
    }
  }
  
  updatePixels();
  
  generateStripeLight();
  
  saveArrayToC(stripeLight, "stripeLight");
  saveArrayToC(stripeWidth, "stripeWidth");
  save("floor.png");
}

void generateStripeLight() {
  int level = 11;

  for (int y = 0; y < PART; y++) {
    stripeLight[y] = level;
  }
  
  for (int y = 0; y < PART; y++) {
    stripeLight[PART+y] = level - (12 * y) / PART;
  }
}

void saveArrayToC(int[] arr, String name) {
  String header = "// Generated automatically from prototypes/floor_data/floor_data.pde";
  String out = new String(header + System.lineSeparator() + "short " + name + "[] = { " +  System.lineSeparator());
  String line = new String();
  for (int i = 0; i < arr.length; i++) {
    String lineTest = new String(line + arr[i] + ", ");
    // Break the line if adding a new value would go over 80 char limit
    if (lineTest.length() >= 80) {
      line +=  System.lineSeparator();
      out += line;
      line = "";
    }
    line += new String(arr[i] + ", ");
  }
  out += "};";
  String filename = name + ".c";
  
  wr = createWriter(filename);
  wr.println(out);
  wr.close();
}
