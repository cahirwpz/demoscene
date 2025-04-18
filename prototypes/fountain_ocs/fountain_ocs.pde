final int LIFESPAN = 128;

ParticleSystem ps;
Bitplane bob[];

class Particle {
  PVector loc;
  PVector vel;
  PVector acc;
  int lifespan;
  boolean active;

  Particle(PVector l) {
    acc = new PVector(0, 0.05);
    vel = new PVector(random(-1, 1), random(-2, 0));
    loc = l.get();
    lifespan = LIFESPAN;
    active = true;
  }

  void update() {
    vel.add(acc);
    loc.add(vel);
    lifespan--;

    if (lifespan <= 0 || loc.y <= 16 || loc.y >= height - 16)
      active = false;
  }

  void render() {
    int c = int(lerp(0, 31, float(lifespan) / LIFESPAN));
    int x = int(loc.x) - 16;
    int y = int(loc.y) - 16;

    for (int i = 4; i >= 0; i--) {
      if ((c & (1 << i)) != 0) {
        bpl[i].or(bob[(c - 1) / 2], x, y);
        break;
      }
    }
  }

  void applyForce(PVector f) {
  }
}

void setup() {
  size(320, 256);
  frameRate(60.0);

  ps = new ParticleSystem(80, new PVector(width / 2, 64 ));

  bob = new Bitplane[15];
  for (int i = 0; i < 15; i++) {
    bob[i] = new Bitplane(32, 32);
    bob[i].circleE(16, 16, i + 1);
    bob[i].fill();
  }

  initOCS(5);

  for (int i = 0; i < 31; i++)
    palette[i] = lerpColor(#000000, #ffffff, float(i) / 15);
}

void draw() {
  float t = frameCount / 60.0;

  for (int i = 0; i < 5; i++)
    bpl[i].zeros();

  ps.run();
  ps.addParticles(1);

  updateOCS();

  println(frameRate);
}

