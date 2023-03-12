UVMap uvmap;

UVGenerator[] generators = {
  new FancyEye(),
  new BentPlane(),
  new HorizontalPlanes(),
  new VerticalPlanes(),
  new WavyStarBurst(),
  new MagneticFlare(),
  new HypnoticRainbowSpiral(),
  new Anamorphosis(),
  new Flush(),
  new Ball(),
  new HotMagma(),
  new RotatingTunnelOfWonder(),
  new Twist(),
  new Swirl(),
  new Some8(),
  new Some9(),
  new Some10()
};

int i = 0;
final int FRAMERATE = 25;

void settings() {
  size(640, 480);
}

void setup() {
  frameRate(FRAMERATE);
  
  println("Press LEFT or RIGHT key to change UVMap!");
  
  uvmap = new UVMap(width, height);
  uvmap.attachTexture("texture.png");
  uvmap.generate(generators[i]);
}

void keyPressed() {
  if (key == CODED) {
    // uvmap.save("uvmap" + str(i));
    if (keyCode == RIGHT) {
      i = (i + 1) % generators.length;
      uvmap.generate(generators[i]);
    }
    if (keyCode == LEFT) {
      i = (i + generators.length - 1) % generators.length;
      uvmap.generate(generators[i]);
    }
  }
}

void draw() {
  loadPixels();
  uvmap.render(this, float(frameCount) / FRAMERATE);
  updatePixels();
}
