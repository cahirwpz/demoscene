final int CIRC_RADIUS = 12;
final int CIRC_COUNT = 8;
final int BPL_NUM = 5;
final int PTS_NUM = 6;

final int m0 = 16;
final int k0 = 1;
final int x0 = 48;

final int x1 = 32;
final int m1 = 8;
final int k1 = 2;

final int x2 = 64;
final int m2 = 64;
final int k2 = 1;


final int offset = 16;
final int vert_len = 8;

Sprite block;

int coords[] = {32, 31, 29, 26, 22, 17, 12, 5, -1, -7, -13, -19, -24, -27, -30, -32, -32, -31, -29, -25, -21, -16, -10, -4, 3, 9, 15, 20, 25, 28, 31, 32};
int dx[] = {0, 0, 0, 0, 0, 0, 3, 5, 6, 7, 8, 9, 9, 10, 10, 10, 10, 10, 10, 9, 9, 8, 8, 7, 5, 4, 1, 0, 0, 0, 0, 0};
int interval[] = {17, 17, 17, 16, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 17};

void setup() {
  size(320, 256);
  frameRate(25);
  initOCS(BPL_NUM);
  
  for(int j = 0; j < 6; j++)
  {
    spr[j] = new Sprite(32);
    for (int i = 0; i < 64; i++)
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




void drawSpring(int spr_num, int frame)
{
  int x0 = spr[spr_num].x + 8;
  int y0 = offset;
  bpl[0].line(x0, y0, x0 + dx[frame], y0 + interval[frame]);
  bpl[0].line(x0+1, y0, x0 + dx[frame]+1, y0 + interval[frame]);
  y0 += interval[frame];
  for(int i = 1; i < PTS_NUM; i++)
  {
    dx[frame] = -dx[frame];
    int y1 = y0 + 2*interval[frame];
    bpl[0].line(x0 - dx[frame], y0, x0 + dx[frame], y1);
    bpl[0].line(x0 - dx[frame] + 1, y0, x0 + dx[frame] + 1, y1);
    y0 = y1;
  }
  bpl[0].line(x0 + dx[frame], y0, x0, y0 + interval[frame]);
  bpl[0].line(x0 + dx[frame]+1, y0, x0+1, y0 + interval[frame]);
  bpl[0].line(x0, y0 + interval[frame], x0, y0 + interval[frame] + 8);
  bpl[0].line(x0+1, y0 + interval[frame], x0+1, y0 + interval[frame] + 8);
  dx[frame] = -dx[frame];
}

void draw() {
  int frame0 = frameCount % 32;
  int frame1 = (frameCount + 2) % 32;
  int frame2 = (frameCount + 4) % 32;
  int frame3 = (frameCount + 6) % 32;
  int frame4 = (frameCount + 8) % 32;
  int frame5 = (frameCount + 10) % 32;
  bpl[0].zeros();
  
  spr[0].x = 40;
  spr[0].y = 192 + coords[frame0];
  drawSpring(0, frame0);
  
  
  spr[1].x = 80;
  spr[1].y = 192 + coords[frame1];
  drawSpring(1, frame1);
  
  
  spr[2].x = 120;
  spr[2].y = 192 + coords[frame2];
  drawSpring(2, frame2);
  
  spr[3].x = 160;
  spr[3].y = 192 + coords[frame3];
  drawSpring(3, frame3);
  
  
  spr[4].x = 200;
  spr[4].y = 192 + coords[frame4];
  drawSpring(4, frame4);
  
  
  spr[5].x = 240;
  spr[5].y = 192 + coords[frame5];
  drawSpring(5, frame5);
  
  
  updateOCS();
}
