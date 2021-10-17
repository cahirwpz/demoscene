final int SIZE = 64;
final int N = 16;

Bitplane bob;
Bitplane carry[];
Bitplane sum[];

void renderBob(int x, int y, int c) {  
  for (int i = 0; i < 4; i++)
    sum[i].copy(bpl[i], x, y, SIZE, SIZE, 0, 0);
  
  int k = 0;
  carry[k].zeros();

  for (int i = 0; i < 4; i++, k ^= 1) {
    if ((c & (1 << i)) != 0)
      sum[i].addx(bob, carry[k], 0, 0, carry[k ^ 1]);
    else
      sum[i].add(carry[k], 0, 0, carry[k ^ 1]);
  }
  
  for (int i = 0; i < 3; i++)
    bpl[i].or_mask(sum[i + 1], bob, x, y);
  bpl[3].or_mask(carry[k], bob, x, y);
}

void setup() {
  size(320, 256);
  frameRate(60.0);

  bob = new Bitplane(SIZE, SIZE);
  bob.circleE(SIZE / 2, SIZE / 2, SIZE / 2 - 1);
  bob.fill();

  sum = new Bitplane[4];
  for (int i = 0; i < 4; i++)
    sum[i] = new Bitplane(SIZE, SIZE);
  carry = new Bitplane[2];
  for (int i = 0; i < 2; i++)
    carry[i] = new Bitplane(SIZE, SIZE);
  
  initOCS(4);

  for (int i = 0; i < 16; i++)
    palette[i] = lerpColor(#000000, #ffffff, float(i) / 15);
}

void draw() {
  float t = frameCount / 60.0;

  for (int i = 0; i < 4; i++)
    bpl[i].zeros();

  for (int i = 0; i < N; i++) {
    float a = lerp(0, TWO_PI, float(i) / N);
    int x = (width - SIZE) / 2 + int(SIZE * sin(t + a));
    int y = (height - SIZE) / 2 + int(SIZE * cos(t + a));
    renderBob(x, y, i);
  }

  updateOCS();
  
  println(frameRate);
}

