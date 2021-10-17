Bitplane bob;
Bitplane borrow[];

void setup() {
  size(640, 480);
  frameRate(60);

  bob = new Bitplane(64, 64);
  bob.circleE(32, 32, 30);
  bob.fill();

  borrow = new Bitplane[2];
  borrow[0] = new Bitplane(width, height);
  borrow[1] = new Bitplane(width, height);

  initOCS(5);
  
  for (int i = 1; i < 32; i++)
    palette[i] = lerpColor(color(0, 0, 0), color(255, 255, 255), float(i) / 31);
}

void draw() {
  float t = float(frameCount) / 60.0;
  
  /* Coder-porn. Decrement by 1 with saturation. */
  borrow[0].ones();   
  bpl[0].sub(borrow[0], borrow[1]);
  bpl[1].sub(borrow[1], borrow[0]);
  bpl[2].sub(borrow[0], borrow[1]);
  bpl[3].sub(borrow[1], borrow[0]);
  bpl[4].sub(borrow[0], borrow[1]);
  
  borrow[1].not();
  for (Bitplane b: bpl)
    b.and(borrow[1]);

  for (int i = 0; i < 4; i++) {
    int xo = 120 + int(sin(t * 4 + i * PI / 2) * 60);
    int yo = (height - 64) / 2 + int(cos(t * 2 + i * PI / 2) * 160);
    
    for (Bitplane b: bpl)
      b.or(bob, xo, yo);
  }

  for (Bitplane b: bpl)
    b.rshift(8);
  
  updateOCS();
  
  println(frameRate);
}
