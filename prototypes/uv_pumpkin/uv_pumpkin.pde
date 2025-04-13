PImage[] umap;
PImage[] vmap;
PImage[] texture;

/* Per pixel information on uvmap usage */
PImage mapidx;
int[] mapidxcol = {color(255), color(0)};

/* Punpkin image overlay */
PImage pumpkin;

final int nmaps = 2;
final int ntextures = 2;

void setup() {
  size(320, 256);
  frameRate(25);
  
  pumpkin = loadImage("pumpkin.png");

  umap = new PImage[nmaps];
  vmap = new PImage[nmaps];

  for (int i = 0; i < nmaps; i++) {
    umap[i] = loadImage("uvmap" + str(i) + "-u.png");
    vmap[i] = loadImage("uvmap" + str(i) + "-v.png");
  }
  
  texture = new PImage[ntextures];

  for (int i = 0; i < ntextures; i++) {
    texture[i] = loadImage("texture-" + str(i) + ".png");
  }
  
  mapidx = loadImage("mapidx-0.png");
}

int get_mapidx(int x, int y) {
  color c = mapidx.get(x, y);
  for (int i = 0; i < nmaps; i++)
    if (c == mapidxcol[i])
      return i;
  return -1;
}

void draw() {
  int t = frameCount;
  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int i = get_mapidx(x, y);
      
      if (i == -1)
        continue;
      
      int u = (umap[i].get(x, y) + t) & 255;
      int v = (vmap[i].get(x, y) + t) & 255;
      
      set(x, y, texture[i].get(u, v));
    }
  }
 image(pumpkin, 0, 0);
}