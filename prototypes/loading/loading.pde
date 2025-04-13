PImage bg;
PImage pumpkin;

final int pumpkin_x = 144;
final int pumpkin_low = 191;
final int pumpkin_high = 200;

void setup() {
  size(320, 256);
  frameRate(50);
 
  bg = loadImage("background.png");
  pumpkin = loadImage("pumpkin.png");
}

void draw() {
  int time = (frameCount / 2) % 25;
  int pumpkin_y = (int)lerp(pumpkin_low, pumpkin_high + 2, 1.0 - sin(time / 25.0 * PI));
  int sy = 0;
  
  if (pumpkin_y >= pumpkin_high)
    sy += 32;
  if (pumpkin_y >= pumpkin_high + 1)
    sy += 32;
  
  image(bg, 0, 0);
  copy(pumpkin, 0, sy, 32, 32,
                pumpkin_x, pumpkin_y, 32, 32);
}

