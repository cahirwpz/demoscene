PImage texture;
Rotator rotator;

class Rotator {  
  /* view window */
  PVector pv; /* pivot (position of left up corner) */
  PVector fw; /* forward */
  PVector dw; /* downward */

  Rotator() {
    fw = new PVector();
    dw = new PVector();
    pv = new PVector();
  }

  void control(float angle, float scale) {
    /* step downward vector */
    fw.set(cos(angle) * scale, sin(angle) * scale);

    /* step forward vector */
    dw.set(cos(angle + PI / 2) * scale, sin(angle + PI / 2) * scale);
                     
    pv.set(0.0, 0.0);

    /* center horizontally */
    pv.sub(fw.x * width / 2.0f, fw.y * width / 2.0f);

    /* center vertically */
    pv.sub(dw.x * height / 2.0f, dw.y * height / 2.0f);
  }

  void render() {
    PVector p = pv.copy();
    
    int k = 0;

    for (int y = 0; y < height; y++) {
      PVector q = p.copy();

      for (int x = 0; x < width; x++, k++) {
        int i = (int)q.x % texture.width;
        int j = (int)q.y % texture.height;

        if (i < 0)
          i += texture.width;
        if (j < 0)
          j += texture.width;

        pixels[k] = texture.pixels[i + j * texture.width];

        q.add(fw);
      }
    
      p.add(dw);
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
