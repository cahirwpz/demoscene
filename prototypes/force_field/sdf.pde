/* https://iquilezles.org/articles/distfunctions2d/ */

float length(PVector p) {
  return sqrt(p.x * p.x + p.y * p.y);
}

float length(float x, float y) {
  return sqrt(x * x + y * y);
}

float sign(float x) {
  return x >= 0.0 ? 1.0 : -1.0;
}

interface ForceFunction {
  public float force(PVector p);
}

class Circle implements ForceFunction {
  float r;
  
  Circle(float _r) {
    r = _r;
  }
  
  float force(PVector p) {
    return length(p) - r;
  }
}

class Box implements ForceFunction {
  PVector b;
  
  Box(PVector _b) {
    b = new PVector();
    b.set(_b);
  }
  
  float force(PVector p) {
    float x = abs(p.x) - b.x;
    float y = abs(p.y) - b.y;
    return length(max(x, 0.0), max(y, 0.0)) + min(max(x, y), 0.0);
  }
}

float sdEquilateralTriangle(PVector p) {
  final float k = sqrt(3.0);
  float x = abs(p.x) - 1.0;
  float y = p.y + 1.0 / k;
  if (x + k * y > 0.0) {
    float _x = 0.5 * (x - k * y);
    float _y = 0.5 * (-k * x - y);
    x = _x; y = _y;
  }
  x -= constrain(x, -2.0, 0.0);
  return -length(x, y) * sign(y);
}

float opRound(float d, float r) {
  return d - r;
}

float opOnion(float d, float r) {
  return abs(d) - r;
}

float opDist(float d) {
  if (d < 0.0)
    return 1.0;
  return 1.0 / sq(1.0 + d);
}

class ForceField {
  private PVector o, dx, dy;
  
  public PVector origin, size;
  public float angle;
  
  public ForceFunction func;
  
  ForceField(ForceFunction ff) {
    origin = new PVector();
    size = new PVector();
    
    o = new PVector();
    dx = new PVector();
    dy = new PVector();
  
    func = ff;
  }

  void render(float field[]) {
    dx.set(size.x / WIDTH, 0.0);
    dy.set(0.0, size.y / HEIGHT);
    dx.rotate(angle);
    dy.rotate(angle);
    
    o.set(origin);  
    o.sub(dx.x * WIDTH / 2, dx.y * WIDTH / 2);
    o.sub(dy.x * HEIGHT / 2, dy.y * HEIGHT / 2);
  
    PVector q = new PVector();
    PVector p = new PVector();
   
    q.set(o);
    for (int y = 0, i = 0; y < HEIGHT; y++, q.add(dy)) {
      p.set(q);
      for (int x = 0; x < WIDTH; x++, i++, p.add(dx)) {
        field[i] += opDist(func.force(p));
      }
    }
  }
}
