ArrayList<PShape> shapes;
int[] colors = {color(64,64,64), color(0x40,0xE0,0x80), color(255,192,0)};
int nshapes;

void setup() {
  size(640, 480, P3D);
  frameRate(50);

  shapes = new ArrayList<PShape>();
  shapes.add(loadShape("bat.obj")); 
  shapes.add(loadShape("ghost.obj")); 
  shapes.add(loadShape("pumpkin.obj")); 

  nshapes = shapes.size();
}

final int phase = 200;

void draw() {
  int time = frameCount % phase;
  int num = (frameCount / phase) % nshapes;
  PShape s = shapes.get(num);

  int bg = colors[(num + nshapes - 1) % nshapes];
  int fg = colors[num];
  
  s.setFill(lerpColor(bg, fg, time / 25.0));
  background(bg);

  translate(width/2, height/2);
  scale(8000.0 * (float)time / phase);
  rotateZ(PI + sin(time / 32.0) * 0.05 * PI); 
  shape(s);
}