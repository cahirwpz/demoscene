void setup() {
  frameRate(60);
  size(320, 256);
  
  initOCS(5);

  for (int i = 1; i < 8; i++)
    palette[i + 1] = lerpColor(#800020, #ff0040, float(i) / 7);
  for (int i = 1; i < 8; i++)
    palette[i + 9] = lerpColor(#808020, #ffff40, float(i) / 7);
  for (int i = 1; i < 8; i++)
    palette[i + 17] = lerpColor(#202080, #4040ff, float(i) / 7);
}

void draw() {
  float t = frameCount / 60.0;
  
  for (Bitplane b : bpl)
    b.zeros();

  int zn = 300;
  int zf = 100;
  int rn = 32;
  int rf = 150;

  float k = float(rf * zf - rn * zn) / float(rn - rf);
  float r = rf * (zf + k);
  
  for (int c = 0; c < 3; c++) {
    float z = lerp(zn, zf, float((frameCount + c * 50) % 150) / 150);
    float r1 = r / (z - 3 + k);
    float r2 = r / (z + 3 + k);
    int extra;
  
    for (int i = 0; i < 3; i++) {
      extra = (2 - i) * int(r2 - r1) / 4 + 1;
      bpl[i].circleE(width / 2, height / 2, int(r1) + extra);
      bpl[i].circleE(width / 2, height / 2, int(r2) - extra);
    }
    
    if (c % 3 == 1) {
      bpl[3].circleE(width / 2, height / 2, int(r1) + 1);
      bpl[3].circleE(width / 2, height / 2, int(r2) - 1);
    }
    if (c % 3 == 2) {
      bpl[4].circleE(width / 2, height / 2, int(r1) + 1);
      bpl[4].circleE(width / 2, height / 2, int(r2) - 1);
    }
  }
  
  for (int i = 0; i < 5; i++)
    bpl[i].fill();
  
  updateOCS();
}
