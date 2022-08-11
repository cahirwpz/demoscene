final float LIMIT = 6.0;
final int RADIUS = 42;
boolean solid = true;

class Circle {
  PVector pos;
  PVector vel;
  PVector accel;
  int s;

  Circle() {
    pos = new PVector(WIDTH / 2, HEIGHT / 2);
    vel = new PVector();
    accel = new PVector();
    s = int(random(RADIUS / 2, RADIUS));
  }
  
  void move() {
    accel = PVector.random2D();
    vel.add(accel.mult(0.5));
    vel.limit(min(LIMIT, sqrt(s)));
    pos.add(vel);
    s--;
  }
  
  void show(PGraphics pg) {
    if (solid) {
      color c = lerpColor(#DFF5F4, #217373, float(s) / RADIUS);
      pg.noStroke();
      pg.fill(c);
    } else {
      pg.fill(0);
      pg.stroke(255);
    }
    pg.ellipse(pos.x, pos.y, s, s);
  }
  
  boolean isDead() {
    return s < 1;
  }
}
