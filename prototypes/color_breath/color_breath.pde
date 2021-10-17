class ColorBreath {
  color[] range;
  int steps;
  int first;
};

ColorBreath[] pal;

color randomColor() {
  return color(int(random(0, 255)),
               int(random(0, 255)),
               int(random(0, 255))); 
}

int randomSteps() {
  return int(frameRate * random(0.5, 2.5));
}

void setup() {
  size(256, 256);
  background(0);
  
  pal = new ColorBreath[256];

  for (int i = 0; i < 256; i++) {
    pal[i] = new ColorBreath();
    pal[i].range = new color[2];
    
    pal[i].steps = randomSteps();
    pal[i].range[0] = randomColor();
    pal[i].range[1] = randomColor();
  }
} 

void draw() {
  noStroke();
  
  for (int y = 0; y < 256; y += 16) {
    for (int x = 0; x < 256; x += 16) {
      ColorBreath p = pal[y + x / 16];
      color c = lerpColor(p.range[0], p.range[1],
                          float(frameCount - p.first) / p.steps);
      
      fill(c);
      rect(x, y, 15, 15);
      
      if (p.steps + p.first <= frameCount) {
        p.range[0] = p.range[1];
        p.range[1] = randomColor();
        p.steps = randomSteps();
        p.first = frameCount;
      }
    }
  }
}
