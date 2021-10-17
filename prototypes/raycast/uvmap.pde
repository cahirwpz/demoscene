class UVMapRenderer {
  final float threshold = 0.75;
  final color black = color(0, 0, 0);

  PImage texture;
  PVector[][] rows;
  VectorLerp[] rays;
  RayCaster caster;

  UVMapRenderer(String path, RayCaster raycaster) {    
    caster = raycaster;

    texture = loadImage(path);
    texture.loadPixels();

    rows = new PVector[2][width / 8 + 1];
    rays = new VectorLerp[width / 8 + 1];
  }

  void calculateRow(PVector[] row, VectorLerp l, VectorLerp r) {
    VectorLerp ray = new VectorLerp(l.value(), r.value(), width / 8);
  
    for (int i = 0; i <= width / 8; i++, ray.next())
      row[i] = caster.castRay(ray.value());
  
    l.next();
    r.next();
  }

  void render() {
    VectorLerp l = new VectorLerp(caster.view[0], caster.view[1], height / 8);
    VectorLerp r = new VectorLerp(caster.view[2], caster.view[3], height / 8);
  
    PVector[] row0;
    PVector[] row1;
  
    calculateRow(rows[0], l, r);
  
    // scanline uv-map expander
    for (int j = 0, k = 1; j < height / 8; j++, k ^= 1) {
      row0 = rows[k ^ 1];
      row1 = rows[k ^ 0];
  
      calculateRow(row1, l, r);
      
      for (int i = 0; i <= width / 8; i++)
        rays[i] = new VectorLerp(row0[i], row1[i], 8, threshold);
  
      for (int y = 0; y < 8; y++) {
        for (int i = 0; i < width / 8; i++) {
          VectorLerp ray = new VectorLerp(rays[i].value(), rays[i+1].value(), 8, threshold);
  
          for (int x = 0; x < 8; x++, ray.next()) {
            PVector p = ray.value();
            int u = int(frpart(p.x) * texture.width);
            int v = int(frpart(p.y) * texture.height);
            color c = lerpColor(texture.get(u, v), black, p.z);
  
            set(i * 8 + x, j * 8 + y, c);
          }
        }
      
        for (int i = 0; i <= width / 8; i++)
          rays[i].next();
      }
    }
  }
}
