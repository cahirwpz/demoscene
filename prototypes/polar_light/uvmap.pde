float frpart(float x) {
  return x - floor(x);
}

class UVCoord {
  float u, v;
  
  UVCoord(float u, float v) {
    this.u = u; this.v = v;
  }
}

abstract class UVMap {
  UVCoord[][] map;
  UVCoord offset;
  int width, height;

  UVMap(int width, int height) {
    this.width = width;
    this.height = height;
    offset = new UVCoord(0, 0);
    map = new UVCoord[height][width];

    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {      
        float x = lerp(-1.0, 1.0, float(i) / width);
        float y = lerp(-1.0, 1.0, float(j) / height);

        map[j][i] = new UVCoord(0, 0);

        calculate(map[j][i], x, y);
      }
    }
  }

  abstract void calculate(UVCoord p, float x, float y);
  
  void render(PImage dest, PImage src) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        float u = map[y][x].u + offset.u;
        float v = map[y][x].v + offset.v;
        int i = int(frpart(u) * src.width);
        int j = int(frpart(v) * src.height);
      
        dest.set(x, y, src.get(i, j));
      }
    }
  }
}
