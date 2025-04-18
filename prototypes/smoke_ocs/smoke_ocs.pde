final int SIZE = 32;
final int LIFESPAN = 40;
final int WIDTH = 320;
final int HEIGHT = 256;
final int DEPTH = 5;

OrigChipSet ocs;
ParticleSystem ps;
Bitplane bob;
Bitplane screen[];

class Particle {
  PVector loc;
  PVector vel;
  PVector acc;
  int lifespan;
  boolean active;

  Particle(PVector l) {
    acc = new PVector(0, 0);
    float vx = randomGaussian() * 0.5;
    float vy = randomGaussian() * 0.5;
    vel = new PVector(vx, vy);
    loc = l.get();
    lifespan = LIFESPAN;
    active = true;
  }

  void update() {
    vel.add(acc);
    loc.add(vel);
    acc.x = 0.0;
    acc.y = 0.0;
    lifespan--;

    if (lifespan <= 0 || loc.y < SIZE / 2)
      active = false;
  }

  void render() {
    int x = int(loc.x) - bob.width / 2;
    int y = int(loc.y) - bob.height / 2;
    int c = int(lerp(0, 31, float(lifespan) / LIFESPAN));
    
    for (int i = 0; i < 5; i++) {
      if ((c & (1 << i)) != 0)
        Blit.or(screen[i], bob, x, y);
    }
  }

  void applyForce(PVector f) {
    acc.add(f);
  }
}

void settings() {
  size(OcsRasterWidth, OcsRasterHeight);
}

void setup() {
  frameRate(OcsFrameRate);

  ocs = new OrigChipSet();
  ps = new ParticleSystem(50, new PVector(WIDTH / 2, HEIGHT - 40));

  screen = ocs.chip.allocBitmap(WIDTH, HEIGHT, DEPTH);
  bob = ocs.chip.allocBitplane(SIZE, SIZE);
  Draw.circleE(bob, SIZE / 2, SIZE / 2, SIZE / 2 - 1);
  Blit.fill(bob);

  for (int i = 0; i < 32; i++)
    ocs.setColor(i, lerpColor(#000000, #ffffff, float(i) / 31));
}

void draw() {
  float t = 0.2 * sin(PI * frameCount / OcsFrameRate);

  for (int i = 0; i < 5; i++) {
    Blit.zeros(screen[i]);
  }

  PVector wind = new PVector(t, -0.2);
  ps.applyForce(wind);
  ps.run();
  ps.addParticles(1);

  ocs.setupScreen(screen, 0, DEPTH);
  ocs.setupDisplayWindow(HPOS(0), VPOS(0), WIDTH, HEIGHT);
  ocs.setupBitplaneFetch(HPOS(0), WIDTH);
  ocs.update();
  
  // println(frameRate);
}
