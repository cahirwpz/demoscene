final int SIZE = 32;

final int N = 31; /* flares */
final int A = 3; /* number of "arms" */
final float speed = 0.5;

Bitplane flares[][];
Bitplane carry[];

Bitplane[] createFlare(int size) {
  Bitplane[] flare = new Bitplane[3];
  
  for (int i = 0; i < 3; i++)
    flare[i] = new Bitplane(SIZE, SIZE);

  int  s = (SIZE - size) / 2;

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      float u = lerp(-1.0, 1.0, float(y) / (size - 1));
      float v = lerp(-1.0, 1.0, float(x) / (size - 1));
      float d = sq(1.25 - sqrt(sq(u) + sq(v)));

      if (d > 0.25)
        d += 0.33 * sin(d * TWO_PI * 2);
      d = constrain(d, 0.0, 1.0);
      
      int c = int(d * 7);
      
      for (int i = 0; i < 3; i++)
        flare[i].set(x + s, y + s, boolean((c >> i) & 1));
    }
  }
  
  return flare;
}

void setup() {
  frameRate(60);
  size(320, 256);

  carry = new Bitplane[2];
  carry[0] = new Bitplane(SIZE, SIZE);
  carry[1] = new Bitplane(SIZE, SIZE);
  
  flares = new Bitplane[16][];
  for (int i = 0; i < 16; i++)
    flares[i] = createFlare(i + 16);

  initOCS(5);

  for (int i = 0; i < 8; i++)
    palette[i] = lerpColor(#000000, #ff8000, float(i) / 7);
  for (int i = 0; i < 8; i++)
    palette[i + 8] = lerpColor(#000000, #ff0080, float(i) / 7);
  for (int i = 0; i < 8; i++)
    palette[i + 16] = lerpColor(#000000, #00ff80, float(i) / 7);
  for (int i = 0; i < 8; i++)
    palette[i + 24] = lerpColor(#000000, #0080ff, float(i) / 7);
  
  bpl[3].circleE(width / 2, height / 2, 28);
  bpl[3].circleE(width / 2, height / 2, 28 * 3);
  bpl[3].fill();
  bpl[4].circleE(width / 2, height / 2, 28 * 2);
  bpl[4].fill();
}

void draw() {
  float t = frameCount * speed / 60.0;

  for (int j = 0; j < 3; j++)
    bpl[j].zeros(); 

  for (int i = 0; i < N; i++) {
    float a = lerp(0, TWO_PI, float(i) / N) + t / 1.5;
    float x = sin(t + a) * sin(A * a) * 112;
    float y = cos(t + a) * sin(A * a) * 112;
    int f = int(abs(sin(3 * a + t) * 15));
    Bitplane flare[] = flares[f];    
    int xo = int(x) + (width - SIZE) / 2;
    int yo = int(y) + (height - SIZE) / 2;
 
     /* Add with saturation. */
    bpl[0].add(flare[0], xo, yo, carry[0]);
    bpl[1].addx(flare[1], carry[0], xo, yo, carry[1]);
    bpl[2].addx(flare[2], carry[1], xo, yo, carry[0]);

    for (int j = 0; j < 3; j++)
      bpl[j].or(carry[0], xo, yo);
  }

  updateOCS();
}
