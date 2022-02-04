#include "effect.h"

#include "stdio.h"
#include "custom.h"
#include "copper.h"
#include "blitter.h"
#include "bitmap.h"
#include "sprite.h"
#include "fx.h"
#include "gfx.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4 

#define X_OFFSET 56 

#define AMPL 32  
#define Y0 130
#define START_DRAW 48 

#include "sprite.c"
#include "data/background.c"

static CopListT *cp;
static CopInsT *sprptr[8];

/* Offsets for diverging spiders movement. */
static short factors[6] = {-28, 6, -20, 20, 30, -27};

/* Draws lines with blitter to create a spring.
 * It takes position of a spider, numbers of points in a spring
 * and distance between each 2 in the line. */
static void DrawSpring(int y, int x, int pts_num, int pts_dist)
{
  short x0, x1;
  short y0, y1;
  int temp, len;
  int delta_x, delta_y;
  short i; 

  /* Length od the spring. */ 
  len = y - START_DRAW;

  /* Calculate vertical delta_y between points and horizontal offset from center. */ 
  delta_y = (len)/(pts_num << 1);
  delta_x = ((pts_dist * pts_dist) - (delta_y * delta_y)) >> 2;
  if(delta_x > 0)
    delta_x = isqrt(delta_x);
  else
    delta_x = 0;
  
  x0 = x + delta_x;
  x1 = x - delta_x;
  
  y0 = START_DRAW;
  y1 = y0 + delta_y;

  BlitterLineSetup(&background_bmp, 0, LINE_SOLID);
  BlitterLine(x, y0, x0, y1);

  for(i = 0; i < pts_num - 1; i++)
  {
    temp = x0;
    x0 = x1;
    x1 = temp;
    y0 = y1;
    y1 = y0 + 2*delta_y;
    BlitterLine(x1, y0, x0, y1);
  }
  y0 = y1;
  y1 = y0 + delta_y;
  BlitterLine(x0, y0, x, y1);
  BlitterLine(x, y1, x, y); 
}

static void MakeCopperList(CopListT *cp)
{
  CopInit(cp);
  CopSetupSprites(cp, sprptr);
  ITER(i, 0, 6, CopInsSet32(sprptr[i], sprite[i]));
  CopSetupBitplanes(cp, NULL, &background_bmp, DEPTH);

  CopMove16(cp, bplcon2, 0x20);
  CopLoadPal(cp, &back_pal, 0);
  CopLoadPal(cp, &sprite_pal, 17);
  /* Make sure that drawn strings are white. */
  CopWait(cp, Y(START_DRAW), 0);
  CopSetColor(cp, 1, 0xfff);
  CopWait(cp, Y(Y0 + AMPL), 0);
  CopSetColor(cp, 1, back_pal.colors[1]);
  CopEnd(cp);
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);
  
  cp = NewCopList(200);
  MakeCopperList(cp);
  CopListActivate(cp);
  CopListRun(cp);
  
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);
}

static void Kill(void) {
  DisableDMA(DMAF_BLITTER|DMAF_RASTER|DMAF_SPRITE);
  DeleteCopList(cp);
}

static void Render(void) {
  short i, phase, y_offset, k; 
  phase = frameCount >> 1;
  /* Draw one half of springs and sprites every second frame. */
  k = (frameCount & 1) ? 3 : 0;
  for(i = 0 + k; i < 3 + k; i++)
  {
    y_offset = normfx(AMPL * COS(phase * 128 + 8 * factors[i]));
    SpriteUpdatePos(sprite[i], 16, X(X_OFFSET*i), Y(Y0 + y_offset));
    {
      Area2D area = {.x = X_OFFSET * i + 1, .y = START_DRAW, .w = 16, .h = 130};
      BlitterClearArea(&background_bmp, 0, &area);
    }
    DrawSpring(Y0 + y_offset, X_OFFSET * i + 9, 6, 8);
  } 
  CopListRun(cp);
  TaskWaitVBlank();
}

EFFECT(spring, NULL, NULL, Init, Kill, Render);
