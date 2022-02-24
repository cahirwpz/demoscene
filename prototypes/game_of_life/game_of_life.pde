/* Conway's game of life effect
 * Controls:
 * q - show intermediate step Lo
 * w - show intermediate step Hi
 * e - show intermediate step X0
 * r - show intermediate step X1
 * t - show intermediate step X2
 * y - show intermediate step X3
 * u - show intermediate step X5
 * i - show intermediate step X6
 * p - redraw the board
 * 1 - show current board state
 * 2 - show board state 1 generation ago (if colors are turned on)
 * 3 - show board state 2 generations ago (if colors are turned on)
 * 4 - show board state 3 generations ago (if colors are turned on)
 * m - turn on/off auto-stepping mode
 * c - turn on/off colors
 * x - turn on/off palette display (if colors are turned on)
 * 0 - increase simulation speed
 * 9 - decrease simulation speed
 * s - increase bitplane (color) count
 * a - decrease bitplane (color) count
 * f - increase gamma (for generating the color palette)
 * d - decrease gamma (for generating the color palette)
 * g - regenerate board with random data (only when auto-stepping is off)
*/

boolean start = true;
boolean colored = true;
boolean showPalette = false;
int fps = 15;
int dispScale = 4; // how much to scale the pixels for the final display
int bitplanes = 4; // number of bitplanes to use
float gamma = 1.7; // for palette generation

// magic blitter minterm constants
int blitMinterms[][] = {
// 0, 1, 2, 3, 4, 5, 6, 7 : bit
  {0, 1, 1, 0, 1, 0, 0, 1},  // Lo
  {0, 0, 0, 1, 0, 1, 1, 1},  // Hi
  {0, 1, 1, 1, 1, 1, 1, 0},  // X0
  {1, 0, 0, 1, 0, 1, 1, 0},  // X1
  {1, 0, 0, 1, 0, 1, 1, 0},  // X2
  {1, 0, 0, 0, 0, 0, 0, 1},  // X3
  
  {0, 0, 1, 0, 1, 1, 0, 1},  // X5
  {0, 1, 0, 1, 1, 1, 0, 0},  // X6
  {0, 0, 1, 0, 0, 1, 0, 0},  // X7
};

PImage shl;
PImage shr;
  
PImage Lo;
PImage Hi;

PImage LoShup;
PImage LoShdown;
PImage HiShup;
PImage HiShdown;
  
PImage X0;
PImage X1;

PImage X2;
PImage X3;

PImage X4;

PImage X5;
PImage X6;
PImage X7;

PImage initial;

PImage[] prev; // circular buffer of previous board states
int lastp = 0; // index to the last generated board state in prev
color[] palette;

enum Shift {LEFT, RIGHT, UP, DOWN};

// perform blitter boolean function on 3 input channels
PImage blit(PImage A, PImage B, PImage C, int[] minterms)
{
  PImage dest = createImage(A.width, A.height, ARGB);
  A.loadPixels();
  B.loadPixels();
  C.loadPixels();
  dest.loadPixels();
  for (int i = 0; i < A.width * A.height; i++)
  {
    int a = A.pixels[i];
    int b = B.pixels[i];
    int c = C.pixels[i];
    dest.pixels[i] = (a & b & c & int(minterms[7]) |
                     a & b & ~c & int(minterms[6]) |
                     a & ~b & c & int(minterms[5]) |
                     a & ~b & ~c & int(minterms[4]) |
                     ~a & b & c & int(minterms[3]) |
                     ~a & b & ~c & int(minterms[2]) |
                     ~a & ~b & c & int(minterms[1]) |
                     ~a & ~b & ~c & int(minterms[0])) == 1 ? color(255, 255, 255) : color(0, 0, 0); 
  }
  dest.updatePixels();
  return dest;
}

// resize pixels to be 'scale' larger (no interpolation)
PImage resizeLinear(PImage src, int scale)
{
  PImage dest = createImage(src.width * scale, src.height * scale, ARGB);
  src.loadPixels();
  dest.loadPixels();
  for (int i = 0; i < src.width * src.height; i++) 
  {
    int x = i % src.width;
    int y = i / src.width;
    for (int j = 0; j < scale; j++)
      for (int k = 0; k < scale; k++)
        dest.pixels[(y * dest.width + x) * scale + j * dest.width + k] = src.pixels[i];
  }
  dest.updatePixels();
  return dest;
}

// make black pixels of an image transparent
PImage blackTransparent(PImage src)
{
  PImage dest = createImage(src.width, src.height, ARGB);
  src.loadPixels();
  dest.loadPixels();
  for (int i = 0; i < src.width * src.height; i++)
  {
    color c = src.pixels[i];
    dest.pixels[i] = color(255, red(c), green(c), blue(c));
  }
  dest.updatePixels();
  return dest;
}

// shift pixels in an image up, down, left or right 1 pixel
PImage shift(PImage src, Shift shiftDir)
{
  PImage dest = createImage(src.width, src.height, ARGB);
  src.loadPixels();
  dest.loadPixels();
  for (int i = 0; i < src.width * src.height; i++)
  {
    int x = i % src.width;
    int y = i / src.width;
    
    switch(shiftDir)
    {
      case LEFT:
        if (x != src.width - 1)  // not last vertical line
          dest.pixels[y * src.width + x] = src.pixels[i+1];
        else
          dest.pixels[y * src.width + x] = color(0, 0, 0);
        break;
      case RIGHT:
        if (x != 0)  // not first vertical line
          dest.pixels[y * src.width + x] = src.pixels[i-1];
        else
          dest.pixels[y * src.width + x] = color(0, 0, 0);
        break;
      case DOWN:
        if (y != 0)  // not first horizontal line
          dest.pixels[y * src.width + x] = src.pixels[(y-1) * src.width + x];
        else
          dest.pixels[y * src.width + x] = color(0, 0, 0);
        break;
      case UP:
        if (y != src.height - 1)  // not last horizontal line
          dest.pixels[y * src.width + x] = src.pixels[(y+1) * src.width + x];
        else
          dest.pixels[y * src.width + x] = color(0, 0, 0);
        break;
    }
  }
  dest.updatePixels();
  return dest;
}

// produce an image as it would look on the amiga if multiple 'srcs'
// bitplanes were active, with 'palette' as the color palette
PImage bitplaneOverlay(PImage[] srcs, color[] palette, int numSrcs)
{
  PImage dest = createImage(srcs[0].width, srcs[0].height, ARGB);
  dest.loadPixels();
  for (int i = 0; i < numSrcs; i++)
    srcs[i].loadPixels();
    
  for (int i = 0; i < dest.width*dest.height; i++)
  {
    int cidx = 0;
    for (int j = 0; j < numSrcs; j++)
      if ((srcs[j].pixels[i] & 0x00FFFFFF) == 0xFFFFFF)
        cidx |= 1 << j;
    dest.pixels[i] = palette[cidx];
  }
    
  dest.updatePixels();
  return dest;
}

void drawGame()
{
  // if we're drawing multiple bitplanes
  if (colored) {
    // make black on all images transparent so that they can be overlaid over each other
    PImage srcs[] = new PImage[4];
    for (int i = 0; i < bitplanes; i++)
      srcs[i] = blackTransparent(prev[(lastp + bitplanes-i-1) % bitplanes]);
    
    // generate color palette
    colorMode(HSB, 360, 100, 100);
    int cs = int(pow(2.0, float(bitplanes)));
    palette = new color[cs];
    for (int i = 0; i <= cs/2; i++)
      palette[i] = color(195, 100, 100.0*pow((100.0/float(cs/2) * i)/100.0, gamma));
    for (int i = cs/2 + 1; i < cs; i++)
      palette[i] = palette[cs-i];

    colorMode(RGB, 255, 255, 255);

    // overlay images on each other and scale them
    image(resizeLinear(bitplaneOverlay(srcs, palette, bitplanes), dispScale), 0, 0);
    
    // display current palette if necessary
    if (showPalette)
    {
      for (int i = 0; i < cs; i++)
      {
        fill(palette[i]);
        rect(i*50, 0, 50, 100);
      }
    }
  } else {
    image(resizeLinear(prev[lastp], dispScale), 0, 0);
  }
}

// calculate next generation
void cycle(boolean dry) {
  shl = shift(initial, Shift.LEFT);
  shr = shift(initial, Shift.RIGHT);
  
  Lo = blit(shl, initial, shr, blitMinterms[0]);
  Hi = blit(shl, initial, shr, blitMinterms[1]);
  
  LoShup = shift(Lo, Shift.UP);
  LoShdown = shift(Lo, Shift.DOWN);
  HiShup = shift(Hi, Shift.UP);
  HiShdown = shift(Hi, Shift.DOWN);
  
  X0 = blit(LoShup, Lo, LoShdown, blitMinterms[2]);
  X1 = blit(LoShup, Lo, LoShdown, blitMinterms[3]);
  
  X2 = blit(HiShup, Hi, HiShdown, blitMinterms[4]);
  X3 = blit(HiShup, Hi, HiShdown, blitMinterms[5]);
  
  X4 = initial;
  
  X5 = blit(X2, X4, X3, blitMinterms[6]);
  X6 = blit(X1, X5, X3, blitMinterms[7]);
  X7 = blit(X2, X0, X6, blitMinterms[8]);
  
  if (!dry) {
    lastp = (lastp + 1) % bitplanes;
    prev[lastp] = X7;
    initial = X7;
  }
  drawGame();
}

// initialize starting board either from file or from random noise
void initImage(boolean generate)
{
  if (generate)
  {
    initial = createImage(1200/dispScale, 1200/dispScale, ARGB);
    initial.loadPixels();
    for (int i = 0; i < initial.width*initial.height; i++)
      initial.pixels[i] = random(1) >= 0.25 ? color(0, 0, 0) : color(255, 255, 255);
    initial.updatePixels();
  }
  else
  {
    initial = loadImage("initial-board.png");
    prev = new PImage[4];
    for (int i = 0; i < 4; i++)
    {
      prev[i] = createImage(initial.width, initial.height, ARGB);
      prev[i].loadPixels();
      for (int j = 0; j < prev[i].width*prev[i].height; j++)
      {
        prev[i].pixels[j] = color(0, 0, 0);
      }
      prev[i].updatePixels();
    }
    prev[0] = initial;
    lastp = 0;
  }
  cycle(true);
}

void setup() {
  size(640, 512);
  initImage(false);
  frameRate(fps);
}

void mousePressed()
{
  cycle(false);
}

// effect keyboard control
void keyPressed()
{
  if (key == 'q')
    image(resizeLinear(Lo, dispScale), 0, 0);
  else if (key == 'w')
    image(resizeLinear(Hi, dispScale), 0, 0);
  else if (key == 'e')
    image(resizeLinear(X0, dispScale), 0, 0);
  else if (key == 'r')
    image(resizeLinear(X1, dispScale), 0, 0);
  else if (key == 't')
    image(resizeLinear(X2, dispScale), 0, 0);
  else if (key == 'y')
    image(resizeLinear(X3, dispScale), 0, 0);
  else if (key == 'u')
    image(resizeLinear(X5, dispScale), 0, 0);
  else if (key == 'i')
    image(resizeLinear(X6, dispScale), 0, 0);
  else if (key == 'p')
    drawGame();
    
  else if (key == '1')
    image(resizeLinear(prev[lastp], dispScale), 0, 0);
  else if (key == '2')
    image(resizeLinear(prev[(lastp+1)%bitplanes], dispScale), 0, 0);
  else if (key == '3')
    image(resizeLinear(prev[(lastp+2)%bitplanes], dispScale), 0, 0);
  else if (key == '4')
    image(resizeLinear(prev[(lastp+3)%bitplanes], dispScale), 0, 0);
  else if (key == 'm')
    start = !start;
  else if (key == 'c')
  {
    colored = !colored;
    drawGame();
  }
  else if (key == 'x')
  {
    showPalette = !showPalette;
    drawGame();
  }
  else if (key == '0')
  {
    fps += 5;
    frameRate(fps);
  }
  else if (key == '9')
  {
    fps -= 5;
    if (fps <= 0) fps = 5;
    frameRate(fps);
  }
  else if (key == 'a')
  {
    bitplanes--;
    if (bitplanes <= 0)
      bitplanes = 1;
  }
  else if (key == 's')
  {
    bitplanes++;
    if (bitplanes >= 5)
      bitplanes = 4;
  }
  else if (key == 'd')
    gamma -= 0.1;
  else if (key == 'f')
    gamma += 0.1;
  else if (!start && key == 'g')
    initImage(true);
}

void draw() {
  if (start)
    cycle(false);
}
