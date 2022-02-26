final int bitplane_n = 5;
final int plane_size = 320*256;

int[] background = new int[320*256 / 16 * bitplane_n];

void setup() {
  frameRate(60);
  size(320, 256);
  initOCS(bitplane_n);
  
  // Palette will probably need to be manually adjusted to fit your needs
  for (int i = 0; i < 32; i++)
    palette[i] = color(#000000);
  palette[4] = color(#ffff00);
  palette[9] = color(#ff00ff);
  palette[11] = color(#ff00ff);
  palette[12] = color(#ffff00);
  palette[13] = color(#00ff00);
  palette[18] = color(#ff00ff);
  palette[19] = color(#ff00ff);
  palette[20] = color(#ffff00);
  palette[21] = color(#00ff00);
    
  String lines[] = loadStrings("background.txt");
  for (int i = 0; i < plane_size / 16 * (bitplane_n - 2); i++)
    background[i] = Integer.reverse(Integer.decode(lines[i])) >> (32 - 16);
}

void recreateBpl() {
  for(int i = 0; i < (bitplane_n - 1); i++) {
    for(int w = 0; w < plane_size / 16; w++)
      bpl[i].data[w] = background[w + (plane_size / 16) * i];
  }
}

void circles(int x, int y, float bigR, float progress, int smallR) {
  int x0 = (int)(bigR*cos(progress) + x);
  int y0 = (int)(bigR*sin(progress) + y);
  int x1 = (int)(bigR*cos((progress + PI) % TWO_PI) + x);
  int y1 = (int)(bigR*sin((progress + PI) % TWO_PI) + y);
  
  bpl[3].zeros();
  bpl[4].zeros();
  bpl[3].circleE(x0, y0, smallR);
  bpl[4].circleE(x1, y1, smallR);
  bpl[3].fill();
  bpl[4].fill();
}

void draw() {  
  recreateBpl();
  circles(width / 2, height / 2 + 25, 75, (frameCount % 256) / 255.0 * TWO_PI, 20);
 
  updateOCS();
  
  println(frameRate);
}
