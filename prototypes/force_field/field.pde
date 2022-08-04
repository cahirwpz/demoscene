class ForceField {
  private PVector o, dx, dy;

  private PVector origin, size;
  private float angle;

  public ForceFunction func;

  ForceField(ForceFunction func) {
    origin = new PVector();
    size = new PVector();

    o = new PVector();
    dx = new PVector();
    dy = new PVector();

    this.func = func;
  }

  void scale(float x, float y) {
    size.set(x, y);
  }

  void origin(float x, float y) {
    origin.set(x, y);
  }

  void angle(float a) {
    angle = a;
  }

  void render(Grid grid) {
    dx.set(size.x / grid.w, 0.0);
    dy.set(0.0, size.y / grid.h);
    dx.rotate(angle);
    dy.rotate(angle);

    o.set(origin);  
    o.sub(dx.x * grid.w / 2, dx.y * grid.w / 2);
    o.sub(dy.x * grid.h / 2, dy.y * grid.h / 2);

    PVector q = new PVector();
    PVector p = new PVector();

    q.set(o);
    for (int y = 0; y < grid.h; y++, q.add(dy)) {
      p.set(q);
      for (int x = 0; x < grid.w; x++, p.add(dx)) {
        field.add(x, y, opDist(func.force(p)));
      }
    }
  }
}
