RayCaster raycast;
UVMapRenderer uvmap;
float pitch = 0, yaw = 0, roll = 0;

void setup() {
  size(640, 480, P3D);
  frameRate(25);

  raycast = new RayTunnel();
  // raycast = new RaySphere();
  // raycast = new RayPlane();
  // ((RayPlane)raycast).calculateForm(new PVector(0, 1, 0), new PVector(0, 1, 0));

  uvmap = new UVMapRenderer("rork-1.png", raycast);
}

void mouseDragged() {
  if (mouseButton == LEFT) {
    pitch += float(mouseX - pmouseX) / width;
    yaw += float(mouseY - pmouseY) / height;
  } else if (mouseButton == RIGHT) {
    roll += float(mouseX - pmouseX) / width;
  }
}

void draw() {
  float time = frameCount / frameRate;

  if (keyPressed) {
    if (key == 'a')
      raycast.setEyeX(raycast.eye.x - 0.25 / frameRate); 
    if (key == 'd')
      raycast.setEyeX(raycast.eye.x + 0.25 / frameRate); 
    if (key == 'w')
      raycast.setEyeY(raycast.eye.y - 0.25 / frameRate); 
    if (key == 's')
      raycast.setEyeY(raycast.eye.y + 0.25 / frameRate); 
    if (key == 'q')
      raycast.setEyeZ(raycast.eye.z - 0.5 / frameRate); 
    if (key == 'e')
      raycast.setEyeZ(raycast.eye.z + 0.5 / frameRate);
  }

  PMatrix3D m = new PMatrix3D();
  m.rotateX(TWO_PI * yaw);
  m.rotateY(TWO_PI * pitch);
  m.rotateZ(TWO_PI * -roll);
  raycast.recalcView(m);
  uvmap.render();
}
