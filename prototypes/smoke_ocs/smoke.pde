final int SIZE = 32;
final int LIFESPAN = 40;

ParticleSystem ps;
Bitplane bob;

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
        bpl[i].or(bob, x, y);
    }
  }

  void applyForce(PVector f) {
    acc.add(f);
  }
}

void setup() {
  size(320, 256);
  frameRate(60.0);

  ps = new ParticleSystem(50, new PVector(width / 2, height - 40));

  bob = new Bitplane(SIZE, SIZE);
  bob.circleE(SIZE / 2, SIZE / 2, SIZE / 2 - 1);
  bob.fill();
  
  initOCS(5);

  for (int i = 0; i < 32; i++)
    palette[i] = lerpColor(#000000, #ffffff, float(i) / 31);
}

void draw() {
  float t = frameCount / 60.0;

  for (int i = 0; i < 5; i++)
    bpl[i].zeros();

  PVector wind = new PVector(0, -0.2);
  ps.applyForce(wind);
  ps.run();
  ps.addParticles(1);

  updateOCS();
  
  println(frameRate);
}

