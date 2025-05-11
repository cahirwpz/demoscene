final float scale = 2.0;

final float near_z = 256;
final float far_z = 512;
final float focal_length = 128 * scale;
final float nstripes = 8.0;

PImage pumpkin;
PImage bookshelf;
PImage lapis;

final boolean debug = false;
final String sceneSelect = "hallway";

ArrayList<Drawable> scene;

void settings() {
  size((int)(320 * scale), (int)(256 * scale));  
}  

void setup() {
  loadPixels();

  lapis = loadImage("lapis.png");
  pumpkin = loadImage("pumpkin.png");
  bookshelf = loadImage("bookshelf.png");

  scene = new ArrayList<Drawable>();
  
  switch (sceneSelect) {
    case "hallway":
      // Segment(x0, z0, x1, z1)
      Segment ceiling = new Segment(-48, 256, -160, 128);
      ceiling.addName("ceiling");
      ceiling.addTexture(lapis, 1.0, 1.0);
      scene.add(ceiling);
      
      Segment wall = new Segment(-48, 256, 48, 256);
      wall.addName("wall");
      wall.addTexture(pumpkin, 1.0, 0.5);
      scene.add(wall);
      
      Segment floor = new Segment(48, 256, 160, 128);
      floor.addName("floor");
      floor.addTexture(lapis, 1.0, 1.0);
      scene.add(floor);
      break;

    case "pipes":
      // Circle(x, z, radius)
      Circle outer = new Circle(0, 384, 256 + 32, Side.INNER);
      outer.addName("outer");
      outer.addTexture(lapis, 0.5, 1.0);
      scene.add(outer);
  
      Circle inner = new Circle(0, 384 + 64, 96, Side.OUTER);
      inner.addName("inner");
      inner.addTexture(bookshelf, 1.0, -0.5);
      scene.add(inner);
      break;
  }
}

PVector control(String name) {
  switch (name) {
    case "inner":
      return new PVector(frameCount / 32.0, 2.0 * sin(frameCount / 32.0));
      
    case "outer":
      return new PVector(0, -4.0 * sin(frameCount / 64.0 + HALF_PI));
    
    case "floor":
    case "ceiling":
    case "wall":
      return new PVector(frameCount / 32.0, 0);
      
    default:
      return new PVector(0, 0);
  }
}

void draw() {
  for (int sy = 0; sy < height; sy++) {
    Line camera = new Line(0, 0, sy - height / 2, focal_length);
    Drawable obj = null;
    float x = MAX_FLOAT;
    float z = MAX_FLOAT;
    PVector uv = null;

    for (Drawable d : scene) {
      try {
        Intersection ip = d.intersection(camera);
        if (z > ip.y) {
          x = ip.x;
          z = ip.y;
          uv = new PVector(ip.u, ip.v);
          obj = d;
        }
      } catch(ArithmeticException e) {
      }
    }

    if (obj != null) {
      float l = illumination(x, z);

      drawLine(sy, uv, control(obj.name()), z, l, obj.texture());
    } else {
      clearLine(sy);
    }
  }

  updatePixels();
}

float illumination(float x, float z) {
  float diff = lerp(-0.5, 0.75, 2.0 * focal_length / (scale * constrain(z, near_z, far_z)));
  float spec = cos(HALF_PI * abs(x) / sqrt(x * x + z * z));
  return constrain(-0.75 + 1.0 * diff * (1.0 + pow(spec, 3)), -1.0, 1.0);
}

void drawLine(int sy, PVector uv, PVector uv_off, float z, float l, Texture tex) {
  float u = nstripes * uv.x / abs(focal_length / z);
  float fv = nstripes * uv.y + uv_off.y;

  for (int sx = 0; sx < width; sx++) {
    float fu = ((float)sx / width - 0.5) * u + uv_off.x;
    
    if (!debug) {
      color texel = tex.getPixel(fu, fv);
      color pixel;
      
      if (l < 0) {
        pixel = lerpColor(color(0,0,0), texel, l + 1.0);
      } else {
        pixel = lerpColor(texel, color(255,255,255), l);
      }
      pixels[sy * width + sx] = pixel;
    } else {
      fu -= floor(fu);
      fv -= floor(fv);
      pixels[sy * width + sx] = color((int)(fu*256.0), (int)(fv*256.0), 0);
    }
  }
}

void clearLine(int y) {
  for (int i = 0; i < width; i++) {
    pixels[y * width + i] = color(0, 0, 0);
  }
}

interface Drawable {
  Intersection intersection(Line other);
  Texture texture();
  String name();
}

class Intersection {
  float x, y;
  float u, v;

  Intersection(float _x, float _y, float _u, float _v) {
    x = _x; y = _y; u = _u; v = _v;
  }
}

class Texture {
  PImage texture;
  float us, vs;

  Texture(PImage _texture, float _us, float _vs) {
    texture = _texture;
    us = _us;
    vs = _vs;
  }
 
  color getPixel(float u, float v) {
    u *= us;
    v *= vs;
    u -= floor(u);
    v -= floor(v);
    int iu = (int)(u * texture.width);
    int iv = (int)(v * texture.height);
    return texture.pixels[iv * texture.width + iu];
  }
}

class Line implements Drawable {
  /* General equation: A * x + B * y + C = 0 */
  private float a, b, c;
  
  float x0, y0;
  float x1, y1;
  
  private float len;
  
  Line(float _x0, float _y0, float _x1, float _y1) {
    x0 = _x0 * scale;
    y0 = _y0 * scale;
    x1 = _x1 * scale;
    y1 = _y1 * scale;
    
    a = y0 - y1;
    b = x1 - x0;
    c = (x0 - x1) * y0 + (y1 - y0) * x0;

    float dx = x1 - x0;
    float dy = y1 - y0;
    
    len = sqrt(dx * dx + dy * dy);
  }

  Intersection intersection(Line other) {
    float x = (b * other.c - other.b * c) / (other.b * a - b * other.a);
    float y = - (a * x + c) / b;
    
    float dx = x - x0;
    float dy = y - y0;
    
    float v = sqrt(dx*dx + dy*dy) / len;

    return new Intersection(x, y, 1.0, v);
  }
  
  public String toString() {
    return "Line(" + str(a) + ", " + str(b) + ", " + str(c) + ")";
  }

  private String name;

  void addName(String _name) {
    name = _name;
  }
  
  String name() {
    return name;
  }

  private Texture tex;

  void addTexture(PImage img, float us, float vs) {
    tex = new Texture(img, us, vs);
  }

  Texture texture() {
    return tex;
  }
}

class Segment extends Line implements Drawable {
  /* Parametric equation: p0 + (p1 - p0) * t = 0, t \in [0,1] */

  Segment(float x0, float y0, float x1, float y1) {
    super(x0, y0, x1, y1);
  }

  Intersection intersection(Line line) {
    Intersection pk = super.intersection(line);

    float tx = (pk.x - x0) / (x1 - x0);
    float ty = (pk.y - y0) / (y1 - y0);

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
  private float a, b;
  private float r;
  private Side side;
  
  Circle(float _x, float _y, float _r, Side _side) {
    a = _x; b = _y; r = _r; side = _side;
  }

  Intersection intersection(Line line) {
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
      return new Intersection(D * dy / dr2 + a, -D * dx / dr2 + b, 0, 0);
    }

    float xi = (D * dy + sgn(dy) * dx * sqrt(Delta)) / dr2;
    float yi = (-D * dx + abs(dy) * sqrt(Delta)) / dr2;

    float xj = (D * dy - sgn(dy) * dx * sqrt(Delta)) / dr2;
    float yj = (-D * dx - abs(dy) * sqrt(Delta)) / dr2;

    boolean which = (side == Side.OUTER) ? (yi < yj) : (yj < yi);

    if (which) {
      return new Intersection(xi + a, yi + b, 1.0, atan2(xi, yi) / PI);
    } else {
      return new Intersection(xj + a, yj + b, 1.0, atan2(xj, yj) / PI);
    }
  }

  private String name;

  void addName(String _name) {
    name = _name;
  }
  
  String name() {
    return name;
  }

  Texture tex;

  void addTexture(PImage img, float us, float vs) {
    tex = new Texture(img, us, vs);
  }

  Texture texture() {
    return tex;
  }
}
