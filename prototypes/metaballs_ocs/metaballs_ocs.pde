Bitplane metaball[];
Bitplane carry[];

final int SIZE = 80;
final float D = 0.9;

void setup() {
  size(320, 256);
  frameRate(60);

  metaball = new Bitplane[5];
  for (int i = 0; i < 5; i++)
    metaball[i] = new Bitplane(SIZE, SIZE);
    
  for (int y = 0; y < SIZE; y++) {
    for (int x = 0; x < SIZE; x++) {
      float u = lerp(-D, D, float(y) / SIZE);
      float v = lerp(-D, D, float(x) / SIZE);
      float d = sqrt(sq(u) + sq(v));
      
      if (d < D) {
        int p = constrain(int(sq(1.0 - d) * 128), 0, 31);
        
        for (int i = 4; i >= 0; i--)
          metaball[i].set(x, y, boolean((p >> i) & 1));
      }
    }
  }
  
  carry = new Bitplane[2];
  for (int i = 0; i < 2; i++)
    carry[i] = new Bitplane(SIZE, SIZE);

  initOCS(5);

  for (int i = 0; i < 16; i++)
    palette[i + 8] = lerpColor(color(0, 0, 0), color(255, 255, 255), float(i + 1) / 16);
  for (int i = 0; i < 4; i++)
    palette[i + 24] = #ffffff;
  for (int i = 0; i < 4; i++)
    palette[i + 28] = lerpColor(color(255, 255, 255), color(0, 0, 0), float(i + 1) / 4);
}

void addMetaball(int x, int y) {
  /* Coder-porn... pixel add with saturation on blitter. */
  bpl[0].add(metaball[0], x, y, carry[0]);
  bpl[1].addx(metaball[1], carry[0], x, y, carry[1]);
  bpl[2].addx(metaball[2], carry[1], x, y, carry[0]);
  bpl[3].addx(metaball[3], carry[0], x, y, carry[1]);
  bpl[4].addx(metaball[4], carry[1], x, y, carry[0]);
  for(int i = 0; i < 5; i++)
    bpl[i].or(carry[0], x, y);
}

void draw() {
  float t = PI * float(frameCount) / 60.0 / 1.5;
  
  int xo1 = (width - SIZE) / 2 + int(sin(t) * SIZE * 2 / 3);
  int xo2 = (width - SIZE) / 2 + int(cos(t) * SIZE * 2 / 3);
  int yo1 = (height - SIZE) / 2 + int(sin(t + PI / 2) * SIZE);
 
  for (Bitplane b : bpl)
    b.zeros(); 
  
  addMetaball(xo1, (height - SIZE) / 2);
  addMetaball(xo2, (height - SIZE) / 2);
  addMetaball((width - SIZE) / 2, yo1);
  
  updateOCS();
  
  println(frameRate);
}
