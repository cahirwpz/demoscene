PImage texture;
Rotator rotator;

class Rotator {
  /* control variables (input) */
  PVector scale;
  PVector translate;
  float rotate;
  
  /* view window (output) */
  PVector p;   /* pivot (position of left up corner) */
  PVector f;   /* forward */
  PVector d;   /* downward */

  Rotator() {
    scale = new PVector(1.0, 1.0);
    translate = new PVector(0.0, 0.0);
    rotate = 0.0;
  }

  void control(float angle, float scale) {
    /* step downward vector */
    f = new PVector(cos(angle) * scale,
                    sin(angle) * scale);

    /* step forward vector */
    d = new PVector(cos(angle + PI / 2) * scale,
                    sin(angle + PI / 2) * scale);
                    
    p = new PVector(0.0, 0.0);

    /* center horizontally */
    p.x -= d.x * width / 2.0f;
    p.y -= d.y * width / 2.0f;

    /* center vertically */
    p.x -= f.x * height / 2.0f;
    p.y -= f.y * height / 2.0f;
  }

  void render() {
    PVector pq = p.copy();
    
    int k = 0;

    for (int y = 0; y < height; y++) {
      PVector uv = pq.copy();

      for (int x = 0; x < width; x++, k++) {
        int i = (int)uv.x % texture.width;
        int j = (int)uv.y % texture.height;

        if (i < 0)
          i += texture.width;
        if (j < 0)
          j += texture.width;

        pixels[k] = texture.pixels[i + j * texture.width];

        uv.add(f);
      }
    
      pq.add(d);
    }
  }
};

void setup() {
  size(512, 512);
  background(0);
  
  rotator = new Rotator();
  
  texture = loadImage("lava.png");
  texture.loadPixels();
  
  loadPixels();
} 

void draw() {
  float angle = (float)frameCount / 128 * PI;
  float scale = sin(angle) + 1.0;

  rotator.control(angle, scale);
  rotator.render();
  
  updatePixels();
}
