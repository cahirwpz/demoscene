class Grid {
  float grid[];
  int w, h;

  Grid(int w, int h) {
    this.w = w;
    this.h = h;

    grid = new float[w * h];
  }

  float get(int x, int y) {
    return grid[y * w + x];
  }

  void add(int x, int y, float v) {
    grid[y * w + x] += v;
  }

  void reset() {
    for (int i = 0; i < grid.length; i++) {
      grid[i] = 0.0;
    }
  }
}
