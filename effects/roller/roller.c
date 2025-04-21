#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <sprite.h>
#include <system/memory.h>
#include <color.h>
#include <sync.h>

#define S_WIDTH 320
#define S_HEIGHT 256
#define S_DEPTH 4

static __code CopListT *cp;

// For now, only 32 transitions (2 rows of texture)
#define NTRANSITIONS (5*16)
static CopInsT *ciTransition[NTRANSITIONS];

#include "data/roller-bg.c"
#include "data/magland16.c"


typedef u_short palentry_t;

static CopListT *MakeCopperList(CopListT *cp) {
  palentry_t *p = texture_pal_colors;
  short i, j;
  CopSetupBitplanes(cp, &roller_bp, S_DEPTH);
  CopSetColor(cp, 0, 0x000);
  
  
  for(i = 0; i < NTRANSITIONS; i++){
    // cp->curr now points to the CopWait that's about to happen
    ciTransition[i] = cp->curr;
    CopWait(cp, 0x55, 0); // vpos is overwritten in Render

    // VPOS=255 is in the middle of the roller, so we have to insert this
    // and then take care to sync it when rolling
    if(i == -1)
      CopWait(cp, 255, 0);
    
    // XXX: change texture to cmap4.
    for(j = 1; j < 16; j++){
      CopSetColor(cp, j, p[texture_bp_pixels[(i*16 + j) % (256)]]);
    }
  }
  
  return CopListFinish(cp);
}

static void Load(void) {
  
}

static void UnLoad(void) {
  
}

static void Init(void) {
  //TimeWarp(roller_start);
  
  CopListT *cp = NewCopList(0x1600);
  SetupPlayfield(MODE_LORES, S_DEPTH, X(0), Y(0), S_WIDTH, S_HEIGHT);

  cp = MakeCopperList(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_SPRITE | DMAF_BLITTER | DMAF_BLITHOG);

  (void) roller_pal_colors;
}

static void Kill(void) {
  CopperStop();
  BlitterStop();

  DeleteCopList(cp);
}

#if 0
static void PositionSprite(SpriteT sprite[8], short xo, short yo) {
  short x = X(xo);
  short y = Y(yo);
  short n = 4;

  while (--n >= 0) {
    SpriteT *spr0 = sprite++;
    SpriteT *spr1 = sprite++;

    SpriteUpdatePos(spr0, x, y);
    SpriteUpdatePos(spr1, x, y);

    x += 16;
  }
}
#endif

static void VBlank(void) {

}

static void Render(void) {

  static short framen = 0;
  short i = 0;
  
  // Patch the coppper instructions in memory
  for(i = 0; i < NTRANSITIONS; i++) {
    //ciTransition[i]->wait.vp = 0x80 + (framen>>1) + 4*i;
    
    ciTransition[i]->wait.vp = (0x50 + (framen>>1) + 2*i);
  }

  framen++;
  framen &= 0x7F;
  
  TaskWaitVBlank();
}

EFFECT(Roller, Load, UnLoad, Init, Kill, Render, VBlank);
