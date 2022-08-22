final int DIAMETER = 48;

class Arm implements Comparable {
  PVector pos;
  PVector vel;
  int diameter;

  Arm() {
    float px = 0.0; // cos(frameCount / 50.0 * PI) * 30.0;
    float py = 0.0; // sin(frameCount / 50.0 * PI) * 30.0;
    pos = new PVector(WIDTH / 2 + px, HEIGHT / 2 + py);
    vel = new PVector();
    diameter = int(random(DIAMETER * 0.75, DIAMETER));
  }

  void move() {
    // On A500 we use Q4.12 to represent the values in range [-8.0, 8.0)
    // assertions below are needed to catch overflows
    
    float angle = random(0.0, 2 * PI); // + frameCount / 50.0 * PI;
    PVector accel = new PVector(cos(angle), sin(angle));
    
    vel.add(accel.mult(0.5));
    assert(vel.x >= -8.0 && vel.x < 8.0);
    assert(vel.y >= -8.0 && vel.y < 8.0);
    
    float mag = sq(vel.x) + sq(vel.y);
    assert(mag < 32.0);
    
    if (mag > diameter) {
      assert(diameter / mag < 8.0);
      
      // omitting sqrt here does not cause visible problems so...
      vel.mult(diameter / mag);
      assert(vel.x >= -8.0 && vel.x < 8.0);
      assert(vel.y >= -8.0 && vel.y < 8.0);
    }
    
    pos.add(vel);
    diameter--;
  }

  void show(PGraphics pg) {
    cache.draw(pg, int(pos.x), int(pos.y), diameter / 2);
  }

  boolean isDead() {
    return diameter < 1;
  }

  @Override
  int compareTo(Object arm) {
    // descending order by the size of arm
    return this.diameter - ((Arm)arm).diameter;
  }
};
