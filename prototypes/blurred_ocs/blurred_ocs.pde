final int BOB_SIZE = 256;
final boolean onlyShowBob = false;

Bitplane bob;
Bitplane carry[];

void setup() {
  frameRate(60);
  size(320, 256);  

  initOCS(5);
  
  bob = new Bitplane(BOB_SIZE, BOB_SIZE);
  
  carry = new Bitplane[2];
  carry[0] = new Bitplane(BOB_SIZE, BOB_SIZE);
  carry[1] = new Bitplane(BOB_SIZE, BOB_SIZE);
  
  for (int i = 0; i < 16; i++)
    palette[i] = lerpColor(#000000, #ff0080, float(i) / 15);
  for (int i = 0; i < 16; i++)
    palette[i + 16] = lerpColor(#000000, #0080ff, float(i) / 15);
  
  bpl[4].circleE(width / 2, height / 2, BOB_SIZE / 4 - 1);
  bpl[4].fill();
}

void rotatingTriangle(float t, float phi, float size) {
  int xs[] = new int[3];
  int ys[] = new int[3];
  
  /* Calculate vertices of a rotating triangle. */
  for (int i = 0; i < 3; i++) {
    float k = sin(t + phi) * PI;
    float x = sin(k + i * TWO_PI / 3);
    float y = cos(k + i * TWO_PI / 3);

    xs[i] = int(size / 2 * x) + bob.width / 2;
    ys[i] = int(size / 2 * y) + bob.height / 2;
  }

  /* Create a bob with rotating triangle. */
  for (int i = 0; i < xs.length; i++)
    bob.lineE(xs[i], ys[i], xs[(i + 1) % 3], ys[(i + 1) % 3]);
}

void draw() {
  float t = frameCount / 60.0;

  /* Create a bob with two rotating triangle xor'ed. */
  bob.zeros();
  rotatingTriangle(t * 0.5, 0.0, BOB_SIZE);
  rotatingTriangle(t * 0.5, PI / 3, BOB_SIZE);
  rotatingTriangle(-t * 0.5, PI / 3, BOB_SIZE / 2);
  bob.fill();

  int xo = (width - bob.width) / 2;
  int yo = (height - bob.height) / 2;

  if (onlyShowBob) {
    for (Bitplane b: bpl)
      b.copy(bob, xo, yo);
  } else {
    if (frameCount % 2 == 0) {
      /* Decrement by 1 with saturation. */
      carry[0].ones();   
      bpl[0].sub(carry[0], xo, yo, carry[1]);
      bpl[1].sub(carry[1], xo, yo, carry[0]);
      bpl[2].sub(carry[0], xo, yo, carry[1]);
      bpl[3].sub(carry[1], xo, yo, carry[0]);
      
      carry[0].not();
      for (int i = 0; i < 4; i++)
        bpl[i].and(carry[0], xo, yo);
    }
  
    /* Add with saturation. */
    bpl[0].add(bob, xo, yo, carry[0]);
    bpl[1].add(carry[0], xo, yo, carry[1]);
    bpl[2].add(carry[1], xo, yo, carry[0]);
    bpl[3].add(carry[0], xo, yo, carry[1]);

    for (int i = 0; i < 4; i++)
      bpl[i].or(carry[1], xo, yo);
  }

  updateOCS();
  
  println(frameRate);
}
