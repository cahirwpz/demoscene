float frpart(float x) {
  return x - floor(x);
}

class VectorLerp {
  PVector v;
  PVector dv;
  
  VectorLerp(PVector a, PVector b, int steps) {
    v = a.get();
    dv = PVector.div(PVector.sub(b, a), steps);
  }

  VectorLerp(PVector a, PVector b, int steps, float threshold) {
    PVector d = PVector.sub(b, a);
    if (d.x > threshold)
      d.x -= 1.0;
    if (d.x < -threshold)
      d.x += 1.0;
    if (d.y > threshold)
      d.y -= 1.0;
    if (d.y < -threshold)
      d.y += 1.0;
    v = a.get();    
    dv = PVector.div(d, steps);
  }

  void next() {
    v.add(dv);
  }
  
  PVector value() {
    return v.get();
  }
}
