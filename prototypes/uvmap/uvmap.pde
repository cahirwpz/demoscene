float frpart(float x) {
  return x - floor(x);
}

class UVCoord {
  float u, v, w;
}

interface UVGenerator {
  void calculate(UVCoord p, float x, float y);
}

class UVMap {
  UVCoord[][] map;
  PImage texture;
  int width, height;

  UVMap(int width, int height) {
    this.width = width;
    this.height = height;
    map = new UVCoord[height][width];
  }
  
  void attachTexture(String path) {
    texture = loadImage(path);
    texture.loadPixels(); 
  }

  void generate(UVGenerator g) {
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {      
        float x = lerp(-1.0, 1.0, float(i) / width);
        float y = lerp(-1.0, 1.0, float(j) / height);

        map[j][i] = new UVCoord();

        g.calculate(map[j][i], x, y);
      }
    }
  }
  
  void save(String name) {
    PImage umap = createImage(width, height, RGB);
    PImage vmap = createImage(width, height, RGB);
    
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        int u = int(frpart(map[j][i].u) * 255.5);
        int v = int(frpart(map[j][i].v) * 255.5);
        umap.set(i, j, color(u,u,u));
        vmap.set(i, j, color(v,v,v));
      }
    }
    
    umap.save(name + "-u.png");
    vmap.save(name + "-v.png");
  }
  
  void render(PApplet applet, float t) {
    for (int y = 0, p = 0; y < height; y++) {
      for (int x = 0; x < width; x++, p++) {
        float u = map[y][x].u + t / 4;
        float v = map[y][x].v + t / 4;
        float w = constrain(map[y][x].w, -1.0, 1.0);
        int i = int(frpart(u) * texture.width);
        int j = int(frpart(v) * texture.height);
        color c = texture.pixels[(i * 256 + j) & 0xffff];
        
        if (w >= 0) {
          c = lerpColor(c, color(255, 255, 255), w);
        } else {
          c = lerpColor(c, color(0, 0, 0), abs(w));
        } 
      
        applet.pixels[p] = c;
      }
    }
  }
}