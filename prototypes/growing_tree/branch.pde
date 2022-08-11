ArrayList<Branch> branches;

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
    position = parent.position.copy();
    velocity = parent.velocity.copy();
    diameter = parent.diameter * 0.62;
    parent.diameter = diameter;
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
    PVector bump = PVector.random2D();
    bump.mult(0.25);

    velocity.normalize();
    velocity.mult(0.75);
    velocity.add(bump);
    velocity.mult(random(4.0, 8.0));

    position.add(velocity);
  }

  boolean maySplit() {
    return random(0, 1) < 0.2;
  }
}
