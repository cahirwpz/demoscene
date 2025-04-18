/* Tested on Processing 3.3.5 */

final int shift = 4;
final int WIDTH = 640;
final int HEIGHT = 512;

final float near_z = 256;
final float far_z = near_z + 256;
final float focal_length = 256;
final float stripe = WIDTH / 7.0;

int palette[][];

void settings() {
  size(WIDTH, HEIGHT);
  noSmooth();
}

void drawLine(int y, float x, float w, float l, int palnum) {
  float ds = 1.0 / abs(w);
  float s = -width / 2.0 * ds - x;
  int sc = (int)floor(s);
  
  s -= sc;
         
  for (int i = 0; i < width; i++) {
    s += ds;
    if (s > x + 0.0001) /* damn mantissa noise! */ {
      s -= 1.0;
      sc++;
    }
    int c = palette[palnum][(sc % 2 == 0) ? 0 : 1];
    pixels[y * width + i] = lerpColor(color(0,0,0), c, l);
  }
}

void clearLine(int y) {
  for (int i = 0; i < width; i++) {
    pixels[y * width + i] = color(0, 0, 0);
  }
}

void setup() {
  background(0);
  loadPixels();
  
  palette = new int[2][2];
  palette[0][0] = color(170, 40, 170);
  palette[1][0] = color(0, 110, 255);
  palette[0][1] = color(30, 255, 50);
  palette[1][1] = color(255, 210, 0);
}

void draw() {
  ArrayList<Drawable> sl = new ArrayList<Drawable>();
  
  //sl.add(new Segment(-96, 512, -320, 256));
  //sl.add(new Segment(-96, 512, 96, 512));
  //sl.add(new Segment(96, 512, 320, 256));

  sl.add(new Circle(0, 384, 256 + 32, Side.INNER));
  sl.add(new Circle(0, 384 + 64, 96, Side.OUTER));
  
  for (int i = 0; i < height; i++) {
    Line camera = new Line(0, 0, i - height / 2, focal_length);
    boolean present = false;
    float z = MAX_FLOAT;
    int palnum = 0;
    
    for (int j = 0; j < sl.size(); j++) {
      Drawable s = sl.get(j);
      try {
        PVector meet = s.intersection(camera);
        if (z > meet.y) {
          z = meet.y;
          palnum = j;
        }
        present = true;
      } catch(ArithmeticException e) {
      }
    }
    
    if (present) {
      /* calculate stripe width */
      /* 3d -> 2d */
      float w = abs(stripe * focal_length / z);
      /* calculate stripe luminescence */
      z = constrain(z, near_z - 64, far_z + 64);
      float l = (z - near_z - 64) / (far_z - near_z + 128);
      /* calculate stripe offset */
      float x = frameCount / 64.0;
      drawLine(i, x - floor(x), w, lerp(1.0, 0.0, l), palnum);
    } else {
      clearLine(i);
    }
  }
  
  updatePixels();
}

interface Drawable {
  PVector intersection(Line other);
}

class Line {
  /* General equation: A * x + B * y + C = 0 */
  float a, b, c;
  
  float x0, y0;
  float x1, y1;
  
  Line(float _x0, float _y0, float _x1, float _y1) {
    x0 = _x0; y0 = _y0; x1 = _x1; y1 = _y1;
    a = y0 - y1;
    b = x1 - x0;
    c = (x0 - x1) * y0 + (y1 - y0) * x0;
  }

  float dx() { return x1 - x0; }
  float dy() { return y1 - y0; }

  PVector intersection(Line other) {
    float x = (b * other.c - other.b * c) / (other.b * a - b * other.a);
    float y = - (a * x + c) / b;
    return new PVector(x, y);
  }
  
  public String toString() {
    return "Line(" + str(a) + ", " + str(b) + ", " + str(c) + ")";
  }
}

class Segment extends Line implements Drawable {
  /* Parametric equation: p0 + (p1 - p0) * t = 0, t \in [0,1] */
  
  Segment(float x0, float y0, float x1, float y1) {
    super(x0, y0, x1, y1);
  }
    
  PVector intersection(Line line) {
    PVector pk = super.intersection(line);
        
    float tx = (pk.x - x0) / dx();
    float ty = (pk.y - y0) / dy();
    
    if (tx < 0 || tx > 1)
      throw new ArithmeticException();
    if (ty < 0 || ty > 1)
      throw new ArithmeticException();
    
    return pk;
  }
  
  public String toString() {
    return "Segment(" + str(x0) + ", " + str(y0) + " -> " + str(x1) + ", " + str(y1) + ")";
  }
}

float sgn(float x) {
  if (x < 0)
    return -1;
  return 1;
}

enum Side { INNER, OUTER }; 

class Circle implements Drawable {
  /* Equation: (x - a)^2 + (y - b)^2 = r^2 */
  float a, b;
  float r;
  Side side;
  
  Circle(float _x, float _y, float _r, Side _side) {
    a = _x; b = _y; r = _r; side = _side;
  }

  PVector intersection(Line line) {
    /* http://mathworld.wolfram.com/Circle-LineIntersection.html */
    float x0 = line.x0 - a;
    float y0 = line.y0 - b;
    float x1 = line.x1 - a;
    float y1 = line.y1 - b;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float dr2 = sq(dx) + sq(dy);
    float D = x0 * y1 - x1 * y0;
    float Delta = sq(r) * dr2 - sq(D);
    
    if (Delta < 0) {
      throw new ArithmeticException();
    }
    
    if (Delta == 0) {
      return new PVector(D * dy / dr2 + a, -D * dx / dr2 + b);
    }
    
    float xi = (D * dy + sgn(dy) * dx * sqrt(Delta)) / dr2;
    float yi = (-D * dx + abs(dy) * sqrt(Delta)) / dr2;

    float xj = (D * dy - sgn(dy) * dx * sqrt(Delta)) / dr2;
    float yj = (-D * dx - abs(dy) * sqrt(Delta)) / dr2;

    boolean which = (side == Side.OUTER) ? (yi < yj) : (yj < yi);
    
    if (which) {
      return new PVector(xi + a, yi + b);  
    } else {
      return new PVector(xj + a, yj + b);
    }
  }
}
