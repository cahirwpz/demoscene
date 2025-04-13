PImage umap;
PImage vmap;
PImage texture;
PImage bake;

final int green = color(0, 255, 0);

/* Per pixel information on uvmap usage */
PImage mapidx;

void setup() {
  size(640, 480);
  frameRate(50);
  
  umap = loadImage("torus-u.png");
  vmap = loadImage("torus-v.png");
  bake = loadImage("torus-bake.png");
  texture = loadImage("texture.png");
    
  mapidx = createImage(width, height, RGB);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int i = (umap.get(x, y) == green) ? 0 : 255;
      mapidx.set(x, y, color(i));
    }
  }
}

void draw() {  
  background(0);
  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int i = mapidx.get(x, y);
      
      if (i == color(0))
        continue;

      int ub = (umap.get(x, y) - frameCount) & 255;
      int vb = (vmap.get(x, y)) & 255;
      
      int uc = (umap.get(x, y) + frameCount) & 255;
      int vc = (vmap.get(x, y) + frameCount) & 255;

      float l = brightness(bake.get(ub, vb));
      int c = texture.get(uc, vc);
      
      if (l < 128.0)
        c = lerpColor(color(0), c, l / 128.0);
      else
        c = lerpColor(c, color(255), (l - 128.0) / 128.0);
      
      set(x, y, c);
    }
  }
  
  if (keyPressed) {
    save("uv_torus.png");
  }
}