final float THRESHOLD = 0.8;

HashMap<Integer, PImage> tiles;

int indexTile(float p0, float p1, float p2, float p3) {
  boolean b0 = p0 > THRESHOLD;
  boolean b1 = p1 > THRESHOLD;
  boolean b2 = p2 > THRESHOLD;
  boolean b3 = p3 > THRESHOLD;

  if (b0 && b1 && b2 && b3) // full ? 
    return 1;

  boolean i0 = b0 ^ b1;
  boolean i1 = b1 ^ b2;
  boolean i2 = b2 ^ b3;
  boolean i3 = b3 ^ b0;

  int i = (int(i0) << 12) | (int(i1) << 13) | (int(i2) << 14) | (int(i3) << 15)
    | (int(b0) << 16) | (int(b1) << 17) | (int(b2) << 18) | (int(b3) << 19);
  int ni = int(i0) + int(i1) + int(i2) + int(i3);

  assert(ni % 2 == 0);

  int l0 = 0, l1 = 0, l2 = 0, l3 = 0;

  if (ni > 0) {
    if (i0) {
      l0 = int(N * (THRESHOLD - p0) / (p1 - p0));
      assert(l0 < N && l0 >= 0);
      i |= l0;
    }

    if (i1) {
      l1 = int(N * (THRESHOLD - p1) / (p2 - p1));
      assert(l1 < N && l1 >= 0);
      i |= l1 << 3;
    }

    if (i2) {
      l2 = int(N * (THRESHOLD - p2) / (p3 - p2));
      assert(l2 < N && l2 >= 0);
      i |= l2 << 6;
    }

    if (i3) {
      l3 = int(N * (THRESHOLD - p0) / (p3 - p0));
      assert(l3 < N && l3 >= 0);
      i |= l3 << 9;
    }
  }

  return i;
}

PImage makeTile(float p0, float p1, float p2, float p3) {
  PImage tile = createImage(N, N, RGB);

  float dl = (p3 - p0) / N;
  float dr = (p2 - p1) / N;

  for (int ty = 0; ty < N; ty++) {
    for (int tx = 0; tx < N; tx++) {
      float p = lerp(p0, p1, (float)tx / N);
      int v = (p >= THRESHOLD) ? 255 : 0;
      tile.pixels[ty * N + tx] = color(v, v, v);
    }
    p0 += dl;
    p1 += dr;
  }

  return tile;
}


PImage getTile(float p0, float p1, float p2, float p3) {
  int ti = indexTile(p0, p1, p2, p3); 
  PImage tile = tiles.get(ti);

  if (tile == null) {
    tile = makeTile(p0, p1, p2, p3);
    tiles.put(ti, tile);
  }

  return tile;
}
