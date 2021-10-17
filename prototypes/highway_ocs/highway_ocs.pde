final int LANE_W = 320 + 48;
final int LANE_H = 40;
final int CARS = 40;

Bitplane flare[];
Bitplane borrow[];
Bitplane carry[];
Bitplane laneR[];
Bitplane laneL[];
Car cars[];

class Car {
  float speed;
  float x;
  int y;
  boolean active;
  
  Car() {
    active = false;
  }

  void init() {
    speed = random(0.8, 1.6) * ((random(0, 1) < 0.5) ? -1 : 1);
    x = 0;
    y = int(random(0, 4)) * 8;
    active = true;
  }

  void move() {
    x += abs(speed);
    if (x >= LANE_W - 16)
      active = false;
  }
}

void createFlare() {
  flare = new Bitplane[3];
  for (int i = 0; i < 3; i++)
    flare[i] = new Bitplane(16, 16);

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      float u = lerp(-1.0, 1.0, float(y) / 15);
      float v = lerp(-1.0, 1.0, float(x) / 15);
      float d = sqrt(sq(u) + sq(v));
      
      if (d < 0.8) {
        int c = constrain(int(sq(1.0 - d) * 16), 0, 7);
        
        for (int i = 0; i < 3; i++)
          flare[i].set(x, y, boolean((c >> i) & 1));
      }
    }
  }
}

void setup() {
  size(320, 256);
  frameRate(60);

  createFlare();

  borrow = new Bitplane[2];
  carry = new Bitplane[2];
  for (int i = 0; i < 2; i++) {
    borrow[i] = new Bitplane(LANE_W, LANE_H);
    carry[i] = new Bitplane(16, 16);
  }

  laneR = new Bitplane[3];
  laneL = new Bitplane[3];
  for (int i = 0; i < 3; i++) {
    laneR[i] = new Bitplane(LANE_W, LANE_H);
    laneL[i] = new Bitplane(LANE_W, LANE_H);
  }
  
  cars = new Car[CARS];
  for (int i = 0; i < CARS; i++)
    cars[i] = new Car();

  initOCS(3);

  for (int i = 0; i < 8; i++)
    palette[i] = lerpColor(#000000, #ff4040, float(i) / 7);
    
  for (int i = 0; i < 8; i++)
    copper(i * 8, height / 2, i, lerpColor(#000000, #ffffc0, float(i) / 7));
}

void dimLane(Bitplane lane[]) {
  borrow[0].ones();   
  lane[0].sub(borrow[0], borrow[1]);
  lane[1].sub(borrow[1], borrow[0]);
  lane[2].sub(borrow[0], borrow[1]);

  borrow[1].not();
  for (int i = 0; i < 3; i++)
    lane[i].and(borrow[1]);
}

void addFlare(Bitplane lane[], int xo, int yo) {
  /* Add with saturation. */
  lane[0].add(flare[0], xo, yo, carry[0]);
  lane[1].addx(flare[1], carry[0], xo, yo, carry[1]);
  lane[2].addx(flare[2], carry[1], xo, yo, carry[0]);

  for (int i = 0; i < 3; i++)
    lane[i].or(carry[0], xo, yo);
}

void draw() {
  float t = float(frameCount) / 60.0;

  dimLane(laneR);
  dimLane(laneL);

  /* Add new car if there's a free slot. */
  if (frameCount % 15 == 0) {
    for (Car car : cars) {
      if (car.active)
        continue;
      car.init();
      break;
    }
  }

  /* Draw each active cars. */
  for (Car car : cars) {
    if (!car.active)
      continue;
    if (car.speed < 0)
      addFlare(laneR, int(car.x), car.y);
    else
      addFlare(laneL, LANE_W - int(car.x) - 16, car.y);
    car.move();
  }

  /* Copy lanes to the screen. */
  for (int i = 0; i < 3; i++) {
    laneR[i].lshift(4);
    laneL[i].rshift(4);
    
    bpl[i].copy(laneL[i], 32, 0, width, LANE_H,
                0, height / 2 - LANE_H - 16);
    bpl[i].copy(laneR[i], 16, 0, width, LANE_H,
                0, height / 2 + 16);
  }

  updateOCS();
}

