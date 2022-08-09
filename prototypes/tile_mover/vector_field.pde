abstract class VectorField {
  PVector field[];
  int w, h;
  
  VectorField(int w, int h) {
    this.w = w;
    this.h = h;
    field = new PVector[w * h];
    for (int i = 0; i < w * h; i++) {
      field[i] = new PVector();
    }
  }
  
  PVector get(int x, int y) {
    return field[y * w + x];
  }
        
  abstract PVector func(PVector p);
  
  void calc(float fx, float tx, float fy, float ty) {
    PVector p = new PVector();
    for (int y = 0; y < h; y++) {
      p.y = map(y, 0, h - 1, ty, fy);
      for (int x = 0; x < w; x++) {
        p.x = map(x, 0, w - 1, fx, tx);
        PVector v = func(p);
        v.x = - v.x;
        field[y * w + x] = v;
      }
    }
  }
}

class TestField1 extends VectorField {
  TestField1(int w, int h) {
    super(w, h);
  }
  
  PVector func(PVector p) {
    PVector v = new PVector();
    /* Generate vector field using https://anvaka.github.io/fieldplay
     * and copy it here. */
    v.x = sin(p.mag());
    v.y = cos(p.mag()) - p.y;
    return v;
  }
}

class TestField2 extends VectorField {
  TestField2(int w, int h) {
    super(w, h);
  }
  
  PVector func(PVector p) {
    PVector v = new PVector();
    /* Generate vector field using https://anvaka.github.io/fieldplay
     * and copy it here. */
    v.x = min(p.y, p.x * p.y);
    v.y = p.mag();
    return v;
  }
}
