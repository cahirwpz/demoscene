final int RADIUS = 48;

class Arm implements Comparable {
  PVector pos;
  PVector vel;
  int s;

  Arm() {
    float px = 0.0; // cos(frameCount / 50.0 * PI) * 30.0;
    float py = 0.0; // sin(frameCount / 50.0 * PI) * 30.0;
    pos = new PVector(WIDTH / 2 + px, HEIGHT / 2 + py);
    vel = new PVector();
    s = int(random(RADIUS * 0.75, RADIUS));
  }

  void move() {
    float angle = random(0.0, 2 * PI); // + frameCount / 50.0 * PI;
    PVector accel = new PVector(cos(angle), sin(angle));
    vel.add(accel.mult(0.5));
    float mag = vel.mag();
    float maxMag = sqrt(s);
    if (mag > maxMag) {
      vel.mult(maxMag / mag);
    }
    pos.add(vel);
    s--;
  }

  void show(PGraphics pg) {
    cache.draw(pg, pos.x, pos.y, s);
  }

  boolean isDead() {
    return s < 1;
  }

  @Override
  int compareTo(Object arm) {
    // descending order by the size of arm
    return this.s - ((Arm)arm).s;
  }
}
