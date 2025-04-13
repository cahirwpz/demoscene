final int refreshRate = 50;

int colors[] = { color(255,192,0), color(0,192,255), color(255,0,192), color(192,0,255) };
 
void setup() {
  size(640, 480, P3D);
  frameRate(refreshRate);
}

void draw() {
  background(color(128));
  stroke(color(0));
  ortho();

  for (int i = 0; i < 5; i++) {
    float x = lerp(width * 0.1, width * 0.9, (float)i / 4.0);
    float i_speed = i * refreshRate * 0.5;
    
    pushMatrix();
    translate(x, height/2, -50);
    rotateY((frameCount + i_speed) / refreshRate);
    fill(color(255));
    box(40, height, 40);
    popMatrix();

    for (int j = 0; j < 4; j++) {
      float y = lerp(height * 0.15, height * 0.85, (float)j / 3.0);
      float j_speed = j * refreshRate;
      
      pushMatrix();
      translate(x, y, -50);
      rotateY((-frameCount + i_speed + j_speed) / refreshRate);
      fill(colors[j]);
      box(80, 80, 80);
      popMatrix();
    }
  }
}