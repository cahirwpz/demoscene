/* Placeholders for vector fields generated using
 * https://anvaka.github.io/fieldplay */

import java.util.function.Function;

class VectorField {
  PVector field[];
  int w, h;
  Function<PVector, PVector> func;

  VectorField(Function<PVector, PVector> func) {
    this.w = width / size + 1;
    this.h = height / size + 1;
    this.func = func;
    field = new PVector[w * h];
    for (int i = 0; i < w * h; i++) {
      field[i] = new PVector();
    }
  }

  PVector get(int x, int y) {
    return field[y * w + x];
  }

  void calc(float fx, float tx, float fy, float ty) {
    PVector p = new PVector();
    for (int y = 0; y < h; y++) {
      p.y = map(y, 0, h - 1, ty, fy);
      for (int x = 0; x < w; x++) {
        p.x = map(x, 0, w - 1, fx, tx);
        PVector v = this.func.apply(p);
        v.x = - v.x;
        field[y * w + x] = v;
      }
    }
  }
}

class TestField1 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = sin(p.mag());
    v.y = cos(p.mag()) - p.y;
    return v;
  }
}

class TestField2 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = min(p.y, p.x * p.y);
    v.y = p.mag();
    return v;
  }
}

class TestField3 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = sin(p.y);
    v.y = sin(p.x) - p.x;
    return v;
  }
}

class TestField4 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = 0.5 * sin(p.y);
    v.y = 0.0;
    return v;
  }
}

class TestField5 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = p.y;
    v.y = p.x;
    return v;
  }
}

class TestField6 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = 0.5 * cos(p.x);
    v.y = p.x;
    return v;
  }
}

class TestField7 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = sin(min(p.y, p.x));
    v.y = sin(exp(p.mag()));
    return v;
  }
}

class TestField8 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = sin(p.y * p.y);
    v.y = sin(p.x * p.x);
    return v;
  }
}

class TestField9 implements Function<PVector, PVector> {
  @Override
  PVector apply(PVector p) {
    PVector v = new PVector();
    v.x = sin(p.y * 10) / 5;
    v.y = sin(p.x * 10) / 5;
    return v;
  }
}

ArrayList<VectorField> fields = new ArrayList<VectorField>();

void addVectorField(Function<PVector, PVector> func, 
                    float fx, float tx, float fy, float ty)
{
  VectorField f = new VectorField(func);
  f.calc(fx, tx, fy, ty);
  fields.add(f);
}
