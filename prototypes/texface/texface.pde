PImage texture;

/*
 * if triangle rasterizer is not pixel perfect
 * one should see white pixels on triangle edges
 */

class Vertex {
  int x, y, u, v;

  Vertex(float _x, float _y, float _u, float _v) {
    x = fp16(_x); y = fp16(_y); u = fp16(_u); v = fp16(_v);
  }
}

class EdgeInfo {
  int ys, ye;
  int x, u, v;
  int dx, du, dv;
  boolean valid;
  
  EdgeInfo() {
    valid = false;
  }

  EdgeInfo(Vertex p0, Vertex p1) {
    ys = fp16_int(p0.y);
    ye = fp16_int(p1.y);
    
    assert(ys <= ye);

    if (ys == ye) {
      valid = false;
      dx = (p0.x < p1.x) ? MIN_INT : MAX_INT;
      return;
    }

    int ys_centered = p0.y + fp16(0.5);
    int ys_prestep = fp16_ceil(ys_centered) - ys_centered;
    
    dx = fp16_div(p1.x - p0.x, p1.y - p0.y);
    du = fp16_div(p1.u - p0.u, p1.y - p0.y);
    dv = fp16_div(p1.v - p0.v, p1.y - p0.y);
    
    x = p0.x + fp16_mul(ys_prestep, dx);
    u = p0.u + fp16_mul(ys_prestep, du);
    v = p0.v + fp16_mul(ys_prestep, dv);
    
    valid = true;
  }
  
  EdgeInfo copy() {
    EdgeInfo n = new EdgeInfo();
    n.ys = ys; n.ye = ye;
    n.x = x; n.u = u; n.v = v;
    n.dx = dx; n.du = du; n.dv = dv;
    n.valid = valid;
    return n;
  }
  
  void step() {
    x += dx; u += du; v += dv;
  }
}

void span(int y, EdgeInfo left, EdgeInfo right) {
  int xs = fp16_int(left.x);
  int xe = fp16_int(right.x);
  
  if (xs == xe)
    return;

  assert(xs < xe);

  int xs_centered = left.x + fp16(0.5);
  int xs_prestep = fp16_ceil(xs_centered) - xs_centered;

  int du = fp16_div(right.u - left.u, right.x - left.x);
  int dv = fp16_div(right.v - left.v, right.x - left.x);
  
  int u = left.u + fp16_mul(xs_prestep, du);
  int v = left.v + fp16_mul(xs_prestep, dv);
  
  for (int x = xs; x < xe; x++) {
    //int p = get(x, y) + color(127);
    int p = texture.get(fp16_int(u) & 255, fp16_int(v) & 255);
    set(x, y, p);
    u += du;
    v += dv;
  }
}

void rasterize(EdgeInfo e01, EdgeInfo e02, EdgeInfo e12) {
  EdgeInfo left, right;
  int y = e01.ys;
  boolean shortOnLeft;
  
  if (e01.valid) {
    shortOnLeft = e01.dx < e02.dx;
  } else if (e12.valid) {
    shortOnLeft = e02.dx < e12.dx;
  } else {
    return;
  }

  left = shortOnLeft ? e01.copy() : e02.copy();
  right = shortOnLeft ? e02.copy() : e01.copy();

  if (e01.valid) {
    for (; y < e01.ye; y++) {
      span(y, left, right); 
      left.step();
      right.step();
    }
  }
  
  if (shortOnLeft) {
    left = e12.copy();
  } else {
    right = e12.copy();
  }

  if (e12.valid) {
    for (; y < e12.ye; y++) {
      span(y, left, right); 
      left.step();
      right.step();
    }
  }
}

void setup() {
  texture = loadImage("gull16.png");
  
  size(640, 480);
  frameRate(50);
  noSmooth();  
  noStroke();
}

void draw() {
  background(0);
  clear();

  float r = 200.0;
  float cx = width / 2;
  float cy = height / 2;

  for (int tri = 0; tri < 2; tri++) {
    Vertex p[] = new Vertex[3];
    for (int i = 0; i < 3; i++) {
      float a = 2 * (i + tri * 2) * PI / 4 + frameCount / 256.0;
      p[i] = new Vertex(sin(a) * r + cx, cos(a) * r + cy, 0.0, 0.0);
    }

    if (tri == 0) {
      p[1].u = fp16(255);
      p[2].u = fp16(255); p[2].v = fp16(255);
    } else if (tri == 1) {
      p[0].u = fp16(255); p[0].v = fp16(255);
      p[1].v = fp16(255);
    }
    
    if (p[0].y > p[1].y) { Vertex tmp = p[0]; p[0] = p[1]; p[1] = tmp; }
    if (p[0].y > p[2].y) { Vertex tmp = p[0]; p[0] = p[2]; p[2] = tmp; }
    if (p[1].y > p[2].y) { Vertex tmp = p[1]; p[1] = p[2]; p[2] = tmp; }
    
    assert(p[0].y < p[1].y);
    assert(p[1].y < p[2].y);
    
    EdgeInfo e01 = new EdgeInfo(p[0], p[1]);
    EdgeInfo e02 = new EdgeInfo(p[0], p[2]);
    EdgeInfo e12 = new EdgeInfo(p[1], p[2]);
  
    rasterize(e01, e02, e12);
    
    set((int)p[0].x, (int)p[0].y, color(255, 255, 0));
    // println("P0", p[0].x, p[0].y);
    set((int)p[1].x, (int)p[1].y, color(0, 255, 255));
    // println("P1", p[1].x, p[1].y);
    set((int)p[2].x, (int)p[2].y, color(255, 0, 255));
    // println("P2", p[2].x, p[2].y);
  }
}