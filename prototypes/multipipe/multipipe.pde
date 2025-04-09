/* Tested on Processing 3.3.5 */

final int shift = 4;
final int WIDTH = 320;
final int HEIGHT = 256;

final float near_z = 256;
final float far_z = near_z + 256;
final float focal_length = 256;
final float stripe = WIDTH / 7.0;

PImage colors;
PImage flares;
PImage lines;

void settings() {
  size(WIDTH, HEIGHT);
}

void rgb12(PImage img) {
  for (int y = 0; y < img.height; y++) {
    for (int x = 0; x < img.width; x++) {
      int c = img.get(x, y);
      c &= 0xf0f0f0f0;
      c |= c >> 4;
      img.set(x,y,c);
    }
  }
}

final float minW = 21.0;
final float maxW = 37.0;
final int prec = 8;

void calcLines() {
  int linesW = width + int(maxW * 2);
  int linesH = int((maxW - minW) * prec);
  
  lines = createImage(linesW, linesH + 1, RGB);
  
  for (int j = 0; j <= linesH; j++) {
    float w = lerp(minW, maxW, float(j) / linesH);

    assert(w >= minW);
    assert(w <= maxW);

    float x = 0.0;
    int c = 0;
           
    for (int i = 0; i < linesW; i++) {
      lines.set(i, j, color(c == 0 ? 0 : 255));
      x += 1.0;
      if (x > w) {
        x -= w;
        c ^= 1;
      }
    }
  }
}

class Stripe {
  int width;
  int line;
  int number;
  
  Stripe(int _w, int _y, int _n) {
    width = _w;
    line = _y;
    number = _n;
  }
  
  public String toString() {
    return "{ .w = " + str(width) + ", .y = " + str(line) + ", .n = " + str(number) + " }";
  }
};

ArrayList<Stripe> stripes;
final int[] speed = {1, -2, -2};

void calcStripes() {
  final int npipes = 3;
  Drawable[] pipe;

  /* Drawables must be Z-sorted from farthest to nearest. */
  pipe = new Drawable[npipes];
  pipe[0] = new Circle(0, 384, 128 + 32 + 12, Side.INNER);
  pipe[1] = new Circle(0, 384 + 32, 46, Side.INNER);
  pipe[2] = new Circle(0, 384 + 32, 46, Side.OUTER);
  
  stripes = new ArrayList<Stripe>();
    
  for (int y = 0; y < height; y++) {
    Line camera = new Line(0, 0, y - height / 2, focal_length);

    for (int j = 0; j < npipes; j++) { 
      Drawable p = pipe[j];
      
      try {
        PVector meet = p.intersection(camera);
        float z = meet.y;
        float w = abs(stripe * focal_length / z); // 3d -> 2d
        Stripe s = new Stripe(int(w * prec), y, j);
        stripes.add(s);
        // println(s);
      } catch(ArithmeticException e) {
      }
    }
  }
}

void setup() {
  frameRate(50);
  
  colors = loadImage("colors.png");
  rgb12(colors);
  flares = loadImage("particles.png");

  calcLines();
  calcStripes();
}

void drawLine(int y, int x, int w, boolean swap, boolean transparent) {
  w -= int(minW * prec);
  
  int c0, c1;

  if (swap) {
    c0 = colors.get(1, y);
    c1 = colors.get(0, y);
  } else {
    c0 = colors.get(0, y);
    c1 = colors.get(1, y);
  }
  
  for (int i = 0; i < width; i++) {
    boolean cs = lines.get(i + x, w) == color(0);
    int c = cs ? c0 : c1;
    if (transparent) {
      int bg = get(i, y);
      c = lerpColor(bg, c, 0.5);
    }
    set(i, y, c);
  }
}

void draw() {
  final int center = - int(prec * (width / 2.0));
  
  for (Stripe s : stripes) {
    int w = s.width;
    int y = s.line;
    int n = s.number;
    // calculate stripe offset
    int x = w * speed[n] * frameCount / 64 + center;

    int _x = x % w;
    int _s = x / w;
    if (_x < 0) {
      _x += w;
      _s += 1;
    }
    
    drawLine(y, _x / prec, w, _s % 2 == 0, n > 1);
  }

  if (keyPressed) {
    save("multipipe.png");
  }
}