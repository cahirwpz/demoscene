class CircleCache {
  boolean solid;
  color dark, light;
  PImage circles[];

  CircleCache() {
    this.solid = true;
    this.dark = #006663;
    this.light = #DFF5F4;
    reset();
  }

  void reset() {
    circles = new PImage[DIAMETER / 2 + 1];
  }

  void setSolid(boolean solid) {
    if (solid == this.solid) {
      return;
    }
    this.solid = solid;
    reset();
  }

  void setColor(color dark, color light) {
    this.dark = dark;
    this.light = light;
    reset();
  }

  void drawCircle(PGraphics pg, int x, int y, int r) {
    if (solid) {
      color c = lerpColor(light, dark, 2.0 * float(r) / DIAMETER);
      pg.noStroke();
      pg.fill(c);
    } else {
      pg.fill(0);
      pg.stroke(255);
    }
    pg.ellipseMode(RADIUS);
    pg.ellipse(x, y, r, r);
  }
  
  void add(int r) {
    buffer.beginDraw();
    buffer.background(color(0, 1.0));
    drawCircle(buffer, r, r, r);
    buffer.endDraw();

    circles[r] = buffer.get(0, 0, r * 2 + 1, r * 2 + 1);
  }

  void draw(PGraphics pg, int x, int y, int r) {
    assert(r <= DIAMETER / 2);

    if (circles[r] == null) {
      add(r);
    }

    pg.image(circles[r], x - r, y - r);
  }
}
