final int Y0 = 8;
final int DEF_HEIGHT = 130;
final int BPL_NUM = 4;
final int AMPL = 32;
final float FACTOR = 0.2;

void setup() {
  size(320, 256);
  frameRate(25);
  initOCS(BPL_NUM);
  
  for(int j = 0; j < 6; j++)
  {
    spr[j] = new Sprite(16);
    for (int i = 0; i < 32; i++)
     spr[j].data[i] = 0xffff;   
  }
  
  palette[0] = rgb12(0);
  palette[1] = rgb12(color(0xff,0xff,0xff));
  palette[2] = rgb12(color(0,0xff,0));
  for(int i = 8; i < 32; i++)
    palette[i] = rgb12(color(0xff, 0, 0));
  bpl[1].lineE(319, 0, 319, 15);
  bpl[1].fill();
}

void drawSpring(int x, int y, int pts_num, int pts_dist)
{
  int len = y - Y0;
  
  int delta_y = round(len/(pts_num * 2.0));
  int delta_x = ((pts_dist * pts_dist) - (delta_y * delta_y))/4; 
  
  if(delta_x > 0)
    delta_x = int(sqrt(delta_x));
  else
    delta_x = 0;
    
  int y0 = Y0;
  int y1 = y0 + delta_y;
  bpl[0].line(x, y0, x + delta_x, y0);
  
  for(int i = 0; i < pts_num-1; i++)
  {
    delta_x = -delta_x;
    y0 = y1;
    y1 = y0 + 2*delta_y;
    bpl[0].line(x - delta_x, y0, x + delta_x, y1);
  }
  y0 = y1;
  y1 = y0 + delta_y;
    
  bpl[0].line(x + delta_x, y0, x, y1);
  bpl[0].line(x, y1, x, y);
}

void draw() {
  bpl[0].zeros();
  for(int i = 0; i < 6; i++)
  {
    spr[i].x = 40 * i + 30;
    spr[i].y = DEF_HEIGHT + int(AMPL * cos(frameCount * FACTOR));
    drawSpring(spr[i].x + 9, spr[i].y, 8, 8);
  } 
  updateOCS();
}
