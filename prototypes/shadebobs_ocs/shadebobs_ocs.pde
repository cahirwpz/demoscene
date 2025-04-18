Bitplane bob;
Bitplane carry[];

void setup() {
  size(640, 480);
  frameRate(60);

  bob = new Bitplane(64, 64);
  bob.circleE(32, 32, 30);
  bob.circleE(32, 32, 15);
  bob.fill();

  carry = new Bitplane[2];
  carry[0] = new Bitplane(64, 64);
  carry[1] = new Bitplane(64, 64);

  initOCS(5);
  
  palette[0] = #000000;
  for (int i = 1; i < 32; i++)
    palette[i] = color(i * 8 - 1, i * 8 - 1, i * 8 - 1);
}

void draw() {
  float t = 2.0 *  float(frameCount) / 60.0;
  
  int xo = (width - 64) / 2 + int(sin(t) * 160);
  int yo = (height - 64) / 2 + int(cos(t) * 160);
 
  bpl[0].add(bob, xo, yo, carry[0]);
  for (int i = 1; i < 5; i++) {
    int k = (i - 1) & 1;
    int l = i & 1;
    bpl[i].add(carry[k], xo, yo, carry[l]);
  }
  
  updateOCS();
}
