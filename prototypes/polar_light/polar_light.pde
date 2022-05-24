UVMap polar, cartesian;
PImage map, buf;

void setup() {
  size(256, 256);
  frameRate(25);
  
  polar = new PolarMap(256, 256);
  cartesian = new CartesianMap(256, 256);
  
  map = loadImage("polar-map.png");
  map.loadPixels();
  
  buf = createImage(256, 256, RGB);
  buf.loadPixels();
}

void draw() {
  final color fallOff = color(250, 250, 250);

  polar.offset.u = 0.1 * sin(frameCount / TWO_PI);  
  polar.offset.v = 0.1 * cos(frameCount / TWO_PI);  
  polar.render(buf, map);
  
  for (int y = 0; y < height; y++) {
    color c = buf.get(0, y);

    for (int x = 1; x < width; x++) {
      color d = buf.get(x, y);

      c = blendColor(blendColor(c, fallOff, MULTIPLY), d, LIGHTEST);

      buf.set(x, y, c);
    }
  }
  
  cartesian.render(this.g, buf);
}
