color[] palette;

void setup() {
  size(256, 256);
  background(0);
  
  palette = new color[256];
  
  for(int x = 0; x < 256; x++) {
    int r = int(128.0 + 128 * sin(PI * x / 32.0));
    int g = int(128.0 + 128 * sin(PI * x / 64.0));
    int b = int(128.0 + 128 * sin(PI * x / 128.0));
    palette[x] = color(r, g, b);
  } 
} 

void draw() {
  float time = frameCount;
 
  for (int j = 0; j < height; j++) {
    float y = float(j);
    
    for (int i = 0; i < width; i++) {
      float x = float(i);

      float value = sin(dist(x + time, y, 128.0, 128.0) / 8.0)
                  + sin(dist(x, y + time, 64.0, 64.0) / 8.0)
                  + sin(dist(x, y + time / 7, 192.0, 64) / 7.0)
                  + sin(dist(x, y, 192.0, 100.0) / 8.0);
      int c = int((4 + value) * 32);
      
      set(i, j, palette[c]);
    }
  }
}
