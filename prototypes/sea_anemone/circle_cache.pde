// final int RADIUS = 28;
final boolean useCache = false;

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
    circles = new PImage[RADIUS + 1];
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

  void add(int r) {
    buffer.beginDraw();
    buffer.background(color(0, 1.0));
    if (solid) {
      color c = lerpColor(light, dark, float(r) / RADIUS);
      buffer.noStroke();
      buffer.fill(c);
    } else {
      buffer.fill(0);
      buffer.stroke(255);
    }
    buffer.ellipse(r, r, r * 2, r * 2);
    buffer.endDraw();

    circles[r] = buffer.get(0, 0, r * 2, r * 2);
  }

  void draw(PGraphics pg, float fx, float fy, int r) {
    // assert(r <= RADIUS);

    if (useCache) {
      int x = int(fx);
      int y = int(fy);

      if (circles[r] == null) {
        add(r);
      }

      pg.image(circles[r], x - r, y - r);
    } else {
      if (solid) {
        color c = lerpColor(light, dark, float(r) / RADIUS);
        screen.noStroke();
        screen.fill(c);
      } else {
        screen.fill(0);
        screen.stroke(255);
      }
      screen.ellipse(fx, fy, r, r);
    }
  }
}
