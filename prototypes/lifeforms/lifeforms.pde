static final int org_count = 70;
static final int init_food = 30;



ArrayList<Organism> organisms;
Flowfield ff;
float weights[] = {0.9, 0, 0, 0.5, 0.3};

boolean heatmap = false;
boolean wallBounce = false;

class Flowfield {
  PVector[][] vectors;
  int w;
  int h;
  int resolution;
  
  Flowfield(int res) {
    resolution = res;
    w = round(float(width)/float(resolution));
    h = round(float(height)/float(resolution));
    vectors = new PVector[w][h];
    for (int i = 0; i < w; i++) {
      for (int j = 0; j < h; j++) {
        float sample = noise(map(i, 0, w-1, 0, 1), map(j, 0, h-1, 0, 1), float(frameCount)/100.0);
        float theta = map(sample, 0, 1, -TWO_PI, TWO_PI);
        vectors[i][j] = new PVector(cos(theta), sin(theta));
        float sample2 = noise(map(i, 0, w-1, 3, 6), map(j, 0, h-1, 3, 6), float(frameCount)/100.0);
        vectors[i][j].limit(sample2);
      }
    }
  }
  
  PVector get(int x, int y) {
    return vectors[constrain(y, 0, h - 1)][constrain(x, 0, w - 1)];
  }
  
  void draw() {
    PVector reference = new PVector(1, 0);
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        if (heatmap) {
          float sx = x * resolution;
          float sy = y * resolution;
          colorMode(HSB, TWO_PI, 1.0, 1.0);
          float h = PVector.angleBetween(reference, vectors[y][x]);
          stroke(h, 0.5, 0.5);
          fill(h, 0.5, 0.5);
          rect(sx, sy, resolution, resolution);
          colorMode(RGB);
        } else {
          PVector v = vectors[y][x];
          float sx = x * resolution + resolution/2;
          float sy = y * resolution + resolution/2;
          float dx = v.x * resolution;
          float dy = v.y * resolution;
  
          stroke(128); line(sx, sy, sx+dx, sy+dy);
          stroke(255); point(sx, sy);
          stroke(255, 0, 0); point(sx+dx, sy+dy);
        }
      }
    }
  }
}

final float MIN_Q12 = -8.0;
final float MAX_Q12 = 7.999755859375;

final float maxspeed = MAX_Q12;
final float maxforce = 0.5;
final float orgSize = 30;



class Organism {
  PVector pos;
  PVector vel;
  PVector accel;
  
  Organism(float x, float y) {
    pos = new PVector(x, y);
    vel = new PVector(random(MIN_Q12, MAX_Q12), random(MIN_Q12, MAX_Q12));
    
    accel = new PVector(random(-0.1, 0.1), random(-0.1, 0.1));
    
    vel.limit(maxspeed);
  }
  
  void update() {
    vel.add(accel);
    vel.limit(maxspeed);
    pos.add(vel);
    accel.mult(0);
    
    if (wallBounce) {
      PVector desiredVel = new PVector(vel.x, vel.y);
      if (pos.x < 0)
        desiredVel.x = maxspeed;
      else if (pos.x > 1000)
        desiredVel.x = -maxspeed;
      if (pos.y < 0)
        desiredVel.y = maxspeed;
      else if (pos.y > 1000)
        desiredVel.y = -maxspeed;
      
      applyDesiredVel(desiredVel);
    } else {
      if (pos.x < 0)
        pos.x += 1000;
      if (pos.y < 0)
        pos.y += 1000;
      pos.x = pos.x % 1000.0;
      pos.y = pos.y % 1000.0;
    }
  }

  void draw() {
    stroke(#FFFFFF);
    fill(#AAAAAA);
    circle(pos.x, pos.y, orgSize);
    
    PVector vel_graph = PVector.mult(vel, 4);
    stroke(#FF0000);
    line(pos.x, pos.y, pos.x + vel_graph.x, pos.y + vel_graph.y);
    fill(#0000FF);
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
  
  PVector separate(ArrayList<Organism> orgs) {
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
  
  PVector align(ArrayList<Organism> orgs) {
    float neighborDist = 100;
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
  
  PVector cohese(ArrayList<Organism> orgs) {
    float neighborDist = 100;
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
  
  void applyBehaviors(ArrayList<Organism> others) {
    PVector sep = separate(others);
    PVector mouse = seek(new PVector(mouseX, mouseY));
    
    int x = int(pos.x / ff.w);
    int y = int(pos.y / ff.h);
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
  size(1000, 1000); //<>//
  organisms = new ArrayList<Organism>();
  for (int i = 0; i < org_count; i++)
    organisms.add(new Organism(150, 200));
}

void draw() {
  clear();
  ff = new Flowfield(40);
  ff.draw();
  
  fill(#ffffff);
  textSize(30);
  text(String.format("Separation: %.2f\nMouse: %.2f\nFlowfield: %.2f\nAlignment: %.2f\nCohesion: %.2f", 
       weights[0], weights[1], weights[2], weights[3], weights[4]), 0, 30);

  for (Organism org : organisms) {
    org.applyBehaviors(organisms);
    org.update();
    org.draw();
  }
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
