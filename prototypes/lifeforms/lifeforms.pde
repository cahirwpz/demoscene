static final int w = 320;
static final int h = 256;
static final int scale = 3;
static final int tilesize = 8;
static final int tilew = w/tilesize;
static final int tileh = h/tilesize;

static final int orgcount = 30;

PGraphics screen;

Organism buckets[];
Organism organisms[];
Flowfield ff;
float weights[] = {0.9, 0, 0, 0.5, 0.3};

boolean heatmap = false;
boolean wallBounce = false;

class Flowfield {
  PVector[][] vectors;
  int ffw;
  int ffh;
  int resolution;
  
  Flowfield(int res) {
    resolution = res;
    ffw = w/resolution;
    ffh = h/resolution;
    vectors = new PVector[w][h];
    for (int i = 0; i < ffw; i++) {
      for (int j = 0; j < ffh; j++) {
        float sample = noise(map(i, 0, ffw-1, 0, 1), map(j, 0, ffh-1, 0, 1), float(frameCount)/100.0);
        float theta = map(sample, 0, 1, -TWO_PI, TWO_PI);
        vectors[i][j] = new PVector(cos(theta), sin(theta));
        float sample2 = noise(map(i, 0, ffw-1, 3, 6), map(j, 0, ffh-1, 3, 6), float(frameCount)/100.0);
        vectors[i][j].limit(sample2);
      }
    }
  }
  
  PVector get(int x, int y) {
    return vectors[constrain(x, 0, ffw - 1)][constrain(y, 0, ffh - 1)];
  }
  
  void draw() {
    PVector reference = new PVector(1, 0);
    for (int y = 0; y < ffh; y++) {
      for (int x = 0; x < ffw; x++) {
        if (heatmap) {
          float sx = x * resolution;
          float sy = y * resolution;
          screen.colorMode(HSB, TWO_PI, 1.0, 1.0);
          float h = PVector.angleBetween(reference, vectors[x][y]);
          screen.stroke(h, 0.5, 0.5);
          screen.fill(h, 0.5, 0.5);
          screen.rect(sx, sy, resolution, resolution);
          screen.colorMode(RGB);
        } else {
          PVector v = vectors[x][y];
          float sx = x * resolution + resolution/2;
          float sy = y * resolution + resolution/2;
          float dx = v.x * resolution;
          float dy = v.y * resolution;
  
          screen.stroke(128); screen.line(sx, sy, sx+dx, sy+dy);
          screen.stroke(255); screen.point(sx, sy);
          screen.stroke(255, 0, 0); screen.point(sx+dx, sy+dy);
        }
      }
    }
  }
}

final float MIN_Q12 = -8.0;
final float MAX_Q12 = 7.999755859375;

final float maxspeed = MAX_Q12/2;
final float maxforce = 0.5/2;
final float orgSize = 10;

class Organism {
  PVector pos;
  PVector vel;
  PVector accel;
  int bucketid;
  Organism next;
  
  Organism(float x, float y, int id) {
    pos = new PVector(x, y);
    vel = new PVector(random(MIN_Q12, MAX_Q12), random(MIN_Q12, MAX_Q12));
    
    accel = new PVector(random(-0.1, 0.1), random(-0.1, 0.1));
    
    vel.limit(maxspeed);
    bucketid = id;
    next = null;
  }
  
  void update() {
    vel.add(accel);
    vel.limit(maxspeed);
    pos.add(vel);
    accel.mult(0);
    Organism _buckets[] = buckets; // force to show up in debugger
    
    if (wallBounce) {
      PVector desiredVel = new PVector(vel.x, vel.y);
      if (pos.x < 0)
        desiredVel.x = maxspeed;
      else if (pos.x > w)
        desiredVel.x = -maxspeed;
      if (pos.y < 0)
        desiredVel.y = maxspeed;
      else if (pos.y > h)
        desiredVel.y = -maxspeed;
      
      applyDesiredVel(desiredVel);
    } else {
      if (pos.x < 0)
        pos.x += w;
      if (pos.y < 0)
        pos.y += h;
      pos.x = pos.x % w;
      pos.y = pos.y % h;
    }

    int tx = constrain(floor(pos.x / tilesize), 0, tilew-1);
    int ty = constrain(floor(pos.y / tilesize), 0, tileh-1);
    int bucket = ty*tilew + tx;

    Organism org = buckets[bucketid];
    while (org.next != null) {
      assert(org.bucketid == bucketid);
      org = org.next;
    }

    if (bucket != bucketid) {
      Organism self = buckets[bucketid];
      Organism prev = buckets[bucketid];
      while (self != this) {
        assert(self.next != null);
        prev = self;
        self = self.next;
      }
      assert(self.next != prev);
      prev.next = self.next;
      // only one boid in the bucket
      if (self == prev)
        buckets[bucketid] = self.next;

      if (_buckets[bucket] == null)
        buckets[bucket] = self;
      else {
        Organism cur = buckets[bucket];
        while (cur.next != null)
          cur = cur.next;
        cur.next = self;
      }
      self.next = null;
      self.bucketid = bucket;
    }
  }

  void draw() {
    screen.stroke(#FFFFFF);
    screen.fill(#AAAAAA);
    screen.circle(pos.x, pos.y, orgSize);
    
    PVector vel_graph = PVector.mult(vel, 4);
    screen.stroke(#FF0000);
    screen.line(pos.x, pos.y, pos.x + vel_graph.x, pos.y + vel_graph.y);
    screen.fill(#0000FF);
  }
  
  void applyForce(PVector force) {
    accel.add(force);
  }
  
  void applyDesiredVel(PVector desired) {
    PVector steer = PVector.sub(desired, vel);
    steer.limit(maxforce);
    applyForce(steer);
  }
  
  PVector steerForce(PVector desiredVel) {
    PVector steer = PVector.sub(desiredVel, vel);
    steer.limit(maxforce);
    return steer;
  }
  
  PVector steerForce(PVector desiredVel, float max) {
    PVector steer = PVector.sub(desiredVel, vel);
    steer.limit(max);
    return steer;
  }
  
  PVector seek(PVector target) {
    PVector desired = PVector.sub(target, pos);
    desired.normalize();
    desired.mult(maxspeed);
    return steerForce(desired);
  }
  
  PVector arrive(PVector target) {
    PVector desired = PVector.sub(target, pos);
    float dist = desired.mag();
    float speed = maxspeed;
    if (dist < 100) {
      speed = map(dist, 0, 100, 0, maxspeed);
    }
    desired.normalize();
    desired.mult(speed);
    return steerForce(desired);
  }
  
  PVector separate(Organism[] orgs) {
    float desiredSep = orgSize * 1.5;
    PVector avg = new PVector();
    int cnt = 0;
    
    for (Organism other : orgs) {
      float d = PVector.dist(pos, other.pos);
      if (d > 0 && d < desiredSep) {
        PVector away = PVector.sub(pos, other.pos);
        away.normalize();
        away.div(d);
        avg.add(away);
        cnt++;
      }
    }
    
    if (cnt > 0) {
      avg.div(cnt);
      avg.normalize();
      avg.mult(maxspeed);
      avg = steerForce(avg);
    }
    return avg;
  }
  
  PVector align(Organism[] orgs) {
    float neighborDist = 32;
    PVector avg = new PVector();
    int cnt = 0;
    
    for (Organism other : orgs) {
      float d = PVector.dist(pos, other.pos);
      if (d > 0 && d < neighborDist) {
        avg.add(other.vel);
        cnt++;
      }
    }
    
    if (cnt > 0) {
      avg.div(cnt);
      avg.normalize();
      avg.mult(maxspeed);
      avg = steerForce(avg);
    }
    return avg;
  }
  
  PVector cohese(Organism[] orgs) {
    float neighborDist = 32;
    PVector avg = new PVector();
    int cnt = 0;
    
    for (Organism other : orgs) {
      float d = PVector.dist(pos, other.pos);
      if (d > 0 && d < neighborDist) {
        avg.add(other.pos);
        cnt++;
      }
    }
    
    if (cnt > 0) {
      avg.div(cnt);
      avg = seek(avg);
    }
    return avg;
  }
  
  void applyBehaviors(Organism[] others) {
    PVector sep = separate(others);
    PVector mouse = seek(new PVector(mouseX/scale, mouseY/scale));
    
    int x = int(pos.x / ff.ffw);
    int y = int(pos.y / ff.ffh);
    PVector ffForce = steerForce(PVector.mult(ff.get(x, y), 20), maxforce);
    
    PVector alignment = align(others);
    PVector cohesion = cohese(others);
    
    sep.mult(weights[0]);
    mouse.mult(weights[1]);
    ffForce.mult(weights[2]);
    alignment.mult(weights[3]);
    cohesion.mult(weights[4]);
    
    applyForce(sep);
    applyForce(alignment);
    applyForce(cohesion);
    applyForce(mouse);
    applyForce(ffForce);
  }
  
}

void setup() {
  size(960, 768); //<>//
  noSmooth();
  randomSeed(42);
  screen = createGraphics(scale*w, scale*h);

  buckets = new Organism[tilew * tileh];
  organisms = new Organism[orgcount];

  int i = 0;
  while (i < orgcount) {
    int x = floor(random(0, float(tilew)));
    int y = floor(random(0, float(tileh)));
    int bucket = y*tilew + x;
    if (buckets[bucket] == null) {
      organisms[i] = new Organism(x*tilesize - tilesize/2, y*tilesize - tilesize/2, bucket);
      buckets[bucket] = organisms[i];
      i++;
    }
  }
}

void draw() {
  clear();
  screen.beginDraw();
  screen.clear();
  screen.scale(scale);

  ff = new Flowfield(tilesize);
  ff.draw();
  
  screen.fill(#ffffff);
  screen.textSize(10);
  screen.text(String.format("Separation: %.2f\nMouse: %.2f\nFlowfield: %.2f\nAlignment: %.2f\nCohesion: %.2f",
              weights[0], weights[1], weights[2], weights[3], weights[4]), 0, 10);

  for (Organism org : organisms) {
    org.applyBehaviors(organisms);
    org.update();
    org.draw();
  }

  int count = 0;
  for (int i = 0; i < buckets.length; i++) {
    if (buckets[i] != null) {
      Organism org = buckets[i];
      while (org.next != null) {
        org = org.next;
        count++;
      }
      count++;
    }
  }
  assert(count == orgcount);

  screen.endDraw();
  image(screen, 0, 0);
}

void keyPressed() {
  if (key == '1')
    weights[0] -= 0.05;
  if (key == '2')
    weights[0] += 0.05;
  if (key == '3')
    weights[1] -= 0.05;
  if (key == '4')
    weights[1] += 0.05;
  if (key == '5')
    weights[2] -= 0.05;
  if (key == '6')
    weights[2] += 0.05;
  if (key == '7')
    weights[3] -= 0.05;
  if (key == '8')
    weights[3] += 0.05;
  if (key == '9')
    weights[4] -= 0.05;
  if (key == '0')
    weights[4] += 0.05;
}
