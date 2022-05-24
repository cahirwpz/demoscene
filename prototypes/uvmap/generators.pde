class BentPlane implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);
    
    p.u = 0.1 * y / (0.11 + r * 0.15);
    p.v = 0.1 * x / (0.11 + r * 0.15);
    p.w = sqrt(sq(p.u) + sq(p.v)) - 0.5;
  }
}

class RotatingTunnelOfWonder implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);
    
    p.u = 0.3 / (r + 0.5 * x);
    p.v = 3.0 * a / PI;
    p.w = -p.u / 2;
  }
}

class Twist implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = pow(r, 1.0 / 2.0);
    p.v = a / PI + r;
  }
}

class Swirl implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = x * cos(2.0 * r) - y * sin(2.0 * r);
    p.v = y * cos(2.0 * r) + x * sin(2.0 * r);
  }
}

class Anamorphosis implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);
    
    p.u = cos(a) / (3.0 * r);
    p.v = sin(a) / (3.0 * r);
    p.w = -1 / (6 * r);
  }
}

class FancyEye implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);
    
    p.u = 0.04 * y + 0.06 * cos(a * 3.0) / r;
    p.v = 0.04 * x + 0.06 * sin(a * 3.0) / r;
  }
}

class Flush implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = 0.5 * a / PI + 0.25 * r;
    p.v = pow(r, 0.25);
  }
}

class HotMagma implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = 0.5 * a / PI;
    p.v = sin(2.0 * r);
  }
}

class Some8 implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = 3.0 * a / PI;
    p.v = sin(6.0 * r) + 0.5 * cos(7.0 * a);
  }
}

class Some9 implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = x * log(0.5 * sq(r));
    p.v = y * log(0.5 * sq(r));
  }
}

class Some10 implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = 8 * x * sq(1.5 - r);
    p.v = 8 * y * sq(1.5 - r);
  }
}

class HypnoticRainbowSpiral implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.v = sin(a + cos(3 * r)) / pow(r, 0.25);
    p.u = cos(a + cos(3 * r)) / pow(r, 0.25);
    p.w = 1.2 - r * 2;
  }
}

class WavyStarBurst implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.v = (-0.4 / r) + 0.1 * sin(8.0 * a);
    p.u = 1.0 + a / PI;
    p.w = - p.v / (16 * r);
  }
}

class MagneticFlare implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);
    
    p.u = 1 / (r + 1.5 + 0.5 * sin(5 * a));
    p.v = a * 3 / PI;
    p.w = 0.6 + p.u - r * 1.5;
  }
}

class VerticalPlanes implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = 0.25 * x / abs(y);
    p.v = 0.25 / abs(y);
    p.w = -0.5 * p.v;
  }
}

class HorizontalPlanes implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);

    p.u = 0.25 * y / abs(x);
    p.v = 0.25 / abs(x);
    p.w = -0.5 * p.v;
  }
}

class Ball implements UVGenerator {
  void calculate(UVCoord p, float x, float y) {
    float a = atan2(x, y);
    float r = dist(x, y, 0.0, 0.0);
    float r2 = sq(r);

    p.v = x * (1.5 - sqrt(1 - r2)) / (r2 + 1);
    p.u = y * (1.5 - sqrt(1 - r2)) / (r2 + 1);
  }
}
