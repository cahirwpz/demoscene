class ParticleSystem {
  Particle particles[];
  PVector origin;
  
  ParticleSystem(int num, PVector v) {
    particles = new Particle[num];
    origin = v.get();
    for (int i = 0; i < num; i++) {
      particles[i] = new Particle(origin);
      particles[i].active = false;
    }
  }

  void run() {
    for (Particle p : particles) {
      if (!p.active)
        continue;
      p.update();
      if (p.active)
        p.render();
    }
  }
  
  void applyForce(PVector dir) {
    for (Particle p : particles) {
      if (!p.active)
        continue;
      p.applyForce(dir);
    }
  }  

  void addParticles(int num) {
    for (int i = 0; i < particles.length && num > 0; i++) {
      Particle p = particles[i];
      if (p.active)
        continue;
      particles[i] = new Particle(origin);
      num--;
    }
  }
}
