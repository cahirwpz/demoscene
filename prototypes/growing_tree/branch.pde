ArrayList<Branch> branches;

final boolean reallyFast = true;
  
class Branch {
  PVector position;
  PVector velocity;
  float diameter;
  
  Branch(int x, int y) {
    position = new PVector(x, y);
    velocity = new PVector(0, -10);
    diameter = 20.0;
  }

  Branch(Branch parent) {
    parent.diameter *= random(0.55, 0.65);    
    position = parent.position.copy();
    velocity = parent.velocity.copy();
    diameter = parent.diameter;
  }

  boolean isFinished() {
    if (position.x < 0 || position.x >= WIDTH)
      return true;
    if (position.y < 0 || position.y >= HEIGHT)
      return true;
    if (diameter < 0.2) 
      return true;
    return false;
  }

  void grow() {
    if (reallyFast) {
      // On A500 we use Q4.12 to represent the values in range [-8.0, 8.0)
      // assertions below are needed to catch overflows

      float scale = random(1.0, 1.5);
      float angle = random(0.0, 2 * PI);
      PVector bump = new PVector(cos(angle) * scale, sin(angle) * scale);
      
      // `abs(x) + abs(y)` is good enough to approximate unit vector length.
      // It needs to be multiplied by at most sqrt(2.0) to get the exact result.

      float mag = abs(velocity.x) / 4.0 + abs(velocity.y) / 4.0;
      assert(mag >= 0.5 && mag < 4.0);
      
      velocity.mult(scale / mag);
      assert(velocity.x >= -8.0 && velocity.x < 8.0);
      assert(velocity.y >= -8.0 && velocity.y < 8.0);
      
      velocity.add(bump);
      assert(velocity.x >= -8.0 && velocity.x < 8.0);
      assert(velocity.y >= -8.0 && velocity.y < 8.0);
      
      position.add(velocity);
    } else {
      float angle = random(0, 2 * PI);
      PVector bump = new PVector(cos(angle), sin(angle));
      bump.mult(0.25);

      velocity.normalize();
      velocity.mult(0.75);
      velocity.add(bump);
      velocity.mult(random(4.0, 8.0));

      position.add(velocity);
    }
  }

  boolean maySplit() {
    return random(0, 1) < 0.2;
  }
}
