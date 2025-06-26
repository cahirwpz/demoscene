void addSegment(float xs, float xe, float dxs, float dxe, float ys, float ye) {
  assert(ys <= ye);
  assert(xs <= xe);

  int ysi = ceil(ys);
  int yei = ceil(ye);
  float prestep = ysi - ys; 

  xs += prestep * dxs;
  xe += prestep * dxe;

  sbuf.add(new Segment(xs, xe, dxs, dxe, ysi, yei, g.fillColor));
}

void addTriangle(PVector p1, PVector p2, PVector p3) {
  PVector pt;

  if (p1.y > p2.y) {
    pt = p1;
    p1 = p2;
    p2 = pt;
  }

  if (p1.y > p3.y) {
    pt = p1;
    p1 = p3;
    p3 = pt;
  }

  if (p2.y > p3.y) {
    pt = p2;
    p2 = p3;
    p3 = pt;
  }

  float dy21 = p2.y - p1.y;
  float dy31 = p3.y - p1.y;
  float dy32 = p3.y - p2.y;

  float dx21 = (dy21 > 0.0) ? (p2.x - p1.x) / dy21 : 0.0;
  float dx31 = (dy31 > 0.0) ? (p3.x - p1.x) / dy31 : 0.0;
  float dx32 = (dy32 > 0.0) ? (p3.x - p2.x) / dy32 : 0.0;

  float x_mid = p1.x + dx31 * dy21;

  // Split the triangle into two segments by p2.y
  if (p2.x <= x_mid) {
    addSegment(p1.x, p1.x, dx21, dx31, p1.y, p2.y);
    addSegment(p2.x, x_mid, dx32, dx31, p2.y, p3.y);
  } else {
    addSegment(p1.x, p1.x, dx31, dx21, p1.y, p2.y);
    addSegment(x_mid, p2.x, dx31, dx32, p2.y, p3.y);
  }
}
