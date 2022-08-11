ArrayList<Branch> branches;

final int MARGIN = 10;
final float LENGTH = 0.2;

class Branch {
  PVector lastLocation;
  PVector location;
  PVector velocity;
  float diameter;

  Branch(int x, int y) {
    location = new PVector(x, y);
    lastLocation = new PVector(location.x, location.y);
    velocity = new PVector(0, -10);
    diameter = 20.0;
  }

  Branch(Branch parent) {
    location = parent.location.copy();
    lastLocation = parent.lastLocation.copy();
    velocity = parent.velocity.copy();
    diameter = parent.diameter * 0.62;
    parent.diameter = diameter;
  }

  boolean isFinished() {
    if (location.x <= 0 || location.x >= WIDTH)
      return true;
    if (location.y <= 0 || location.y >= HEIGHT)
      return true;
    if (diameter <= 0.2) 
      return true;
    return false;
  }

  void grow() {  
    lastLocation.set(location.x, location.y);

    PVector bump = PVector.random2D();
    bump.mult(0.25);

    velocity.normalize();
    velocity.mult(0.75);
    velocity.add(bump);
    velocity.mult(random(5.0, 10.0));

    location.add(velocity);
  }

  boolean maySplit() {
    return random(0, 1) < LENGTH;
  }
}
