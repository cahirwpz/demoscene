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


// 16 transitions = 1 row of texture
#define NTRANSITIONS (5*16)
#define VPSTART (0xc8 - 32)
#define ROWHEIGHT 2
#define __ANIMATE 0

static CopInsT *ciTransition[NTRANSITIONS+1];

#include "data/roller-bg.c"
#include "data/magland16.c"


typedef u_short palentry_t;

static CopListT *MakeCopperList(CopListT *cp) {
  palentry_t *p = texture_pal_colors;
  short i, j;
  short vp = VPSTART;
  short ffcross = 0;
  (void) i;
  CopSetupBitplanes(cp, &roller_bp, S_DEPTH);
  CopSetColor(cp, 0, 0x000);
#if 1
  // Test - all green before vp = 0xff
  for(j = 1; j < 16; j++){
      CopSetColor(cp, j, 0x0F0);
      (void) p;
  }
#endif
 
  CopSetColor(cp, 0, 0xFFF);

  for(i = 0; i < NTRANSITIONS+1; i++){
    // cp->curr now points to the CopWait that's about to happen
   
    vp += ROWHEIGHT;
    if(!ffcross && vp > 0xFF){
      ffcross = 1;
      ciTransition[i] = cp->curr;
      CopWait(cp, 0xFF, 0xDF); // Wait for VPOS overflow
      CopSetColor(cp, 0, 0x222); // Debug, show copperlist position on raster
      Log("inserted copper safe wait at %d (%p)\n", i, ciTransition[i]);
      i++;
    }
    ciTransition[i] = cp->curr;
    CopWait(cp, vp, 0x00); // vpos, hpos is overwritten in Render
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
  short vpos = VPSTART + (framen>>2);
  short ffcross = 0;

  (void) ffcross; (void) vpos; (void) i;
#if __ANIMATE
  for(i = 0; i < NTRANSITIONS+1; ++i) {

    vpos +=  ROWHEIGHT;
    // Calculate where the crossing now occurs
    if(!ffcross && vpos > 0xff){     
      ffcross = 1;
      // Insert safe wait
      ciTransition[i]->wait.vp = 0xFF;
      ciTransition[i]->wait.hp = 0xDF;
      Log("VPOS overflow at wait number %d\n", i);
      i++;
    }
    
    ciTransition[i]->wait.vp = vpos & 0xFF;
    ciTransition[i]->wait.hp = 0x81;
  
  }
#endif
  framen++;
  framen &= 0x3F;
  (void) ciTransition;
  TaskWaitVBlank();
}

EFFECT(Roller, Load, UnLoad, Init, Kill, Render, VBlank);
