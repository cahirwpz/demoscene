final float SQRT2 = sqrt(2.0);
  
class PolarMap extends UVMap {
  PolarMap(int width, int height) {
    super(width, height);
  }
  
  void calculate(UVCoord p, float x, float y) {
    y = 0.5 * (y + 1.0);
    x = 0.5 * (x + 1.0);
    
    float r = x / SQRT2;
    float a = y * TWO_PI;
    float u = cos(a) * r + 0.5f;
    float v = sin(a) * r + 0.5f;

    p.u = constrain(u, 0.0, 0.995);
    p.v = constrain(v, 0.0, 0.995);
  }
}

class CartesianMap extends UVMap {
  CartesianMap(int width, int height) {
    super(width, height);
  }

  void calculate(UVCoord p, float x, float y) {
    p.u = dist(x, y, 0, 0) / SQRT2;
    p.v = atan2(y, x) / TWO_PI;
  }
}
