import java.util.*;

final int stripes = 20;
final int size = 128;

ArrayList<PVector> stripe;
PVector center;

void setup() {
  size(320, 256);
  noStroke();
  
  center = new PVector(0, 0, 256);
  
  stripe = new ArrayList<PVector>();
  for (int i = 0; i < stripes; i++) {
    float y = random(size) - size / 2;
    float z = random(size) - size / 2;
    
    stripe.add(new PVector(0, y, z));
  }
}

class CompareDepth implements Comparator<PVector> {
  int compare(PVector v1, PVector v2) {
    return int(v1.z - v2.z);
  }
}

void draw() {
  background(0);

  float a = (float)frameCount / 32.0;  

  ArrayList<PVector> temp = new ArrayList<PVector>();
  
  for (PVector s : stripe) {
    /* rotate stripe */
    float y = s.y * cos(a) - s.z * sin(a); 
    float z = s.y * sin(a) + s.z * cos(a);

    float yp = (y + center.y) * 256 / (z + center.z);

    temp.add(new PVector(0, yp, z)); 
  }

  Collections.sort(temp, new CompareDepth());

  float far = center.z - size / 2;
  float near = center.z + size / 2;

  for (PVector s : temp) {
    float h = (s.z + 128) / 64 + 4; 
    float c = s.z * 256 / size + 128;
    
    println(c, h);
    
    fill(constrain((int)c * 0.75, 0, 255));
    rect(0, s.y + height / 2 - h / 2, width, h);
    fill(constrain((int)c, 0, 255));
    rect(0, s.y + height / 2 - h / 3, width, h * 2 / 3);
  }
}

void keyPressed() {
  save("screenshot.png");
}
