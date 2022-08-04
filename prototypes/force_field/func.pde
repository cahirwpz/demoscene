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

class Circle extends ForceField {
  float r;

  Circle(float r) {
    this.r = r;
  }

  float force(PVector p) {
    return length(p) - r;
  }
}

class Box extends ForceField {
  PVector b;

  Box(PVector b) {
    this.b = new PVector();
    this.b.set(b);
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
    x = _x; 
    y = _y;
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
