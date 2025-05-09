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
#define NTRANSITIONS (8*16)
#define VPSTART 0xC8    // (0xc8 - (2*54))
#define ROWHEIGHT 1

// Debug / work in progress switches
// Disable animation (do not rewrite copperlist in Render)
#define __ANIMATE 1
// Handle setting background color when outside bitplanes
#define __HANDLEBG 1
// Narrow the area where we set color 0 to pallete
#define LFRAME (0x3E + 2)
#define RFRAME (0xDF - 2)


static CopInsT *ciTransition[NTRANSITIONS*3];

#include "data/roller-bg.c"
#include "data/magland16.c"

typedef u_short palentry_t;

static CopListT *MakeCopperList(CopListT *cp) {
  palentry_t *p = texture_pal_colors;
  short i, j, k;
  short vp = VPSTART;
  short ffcross = 0;
  (void) i; (void) ffcross; (void) ciTransition; (void) vp;
  CopSetupBitplanes(cp, &roller_bp, S_DEPTH);
  CopSetColor(cp, 0, 0x000);

 

  /*

    Inserting CopWaitMask for the right edge of the screen breaks current vp overflow fix
    the VP overflow CopWait should be the last one in the line

    So, like this:
    
    *ciTran++ = CopWait(cp, vp, 0);   // [1]
    color0 = RED
    CopWaitMask( HP = LFRAME)
    [ set 16 colors ]
    CopWaitMask( HP = RFRAME)
    color 0 = RED

    *ciTran++ = CopWait(cp, vp, 0);  // [2]

    The copWait [2] will be written to vp=FF,hp=DF when vp = 0xFF
    and will be the same as [1] in other cases

   */
  // cp->curr points to the copper instruction that's about to be inserted
  k = 0;
  for(i = 0; i < NTRANSITIONS*3;){
    
    vp += 1;
    ciTransition[i++] = cp->curr;
    CopWait(cp, VP(vp), HP(0));
    
#if __HANDLEBG     
    CopSetColor(cp, 0, 0xF00); 
    CopWaitMask(cp, VP(vp), HP(LFRAME), 0x00, 0xFF); // vpos, hpos is overwritten in Rende
#endif
    // Set colors from texture
    // XXX: change texture to cmap4.
    for(j = 0; j < 16; j++){
      CopSetColor(cp, j, p[texture_bp_pixels[(k*16 + j) % (texture_bp_width * texture_bp_height)]]);
    }
#if __HANDLEBG    
    CopWaitMask(cp, VP(vp), HP(RFRAME), 0x00, 0xFF); // vpos, hpos is overwritten in Render
    CopSetColor(cp, 0, 0xF00);
#endif
    ciTransition[i++] = cp->curr;
    CopWait(cp, VP(vp), HP(0));
    ciTransition[i++] = cp->curr;
    CopWait(cp, VP(vp), HP(0));
    k++;
  }

  return CopListFinish(cp);
}

static void Load(void) {
  
}

static void UnLoad(void) {
  
}

static void Init(void) {
  //TimeWarp(roller_start);
  //TODO: calculate copper list length
  CopListT *cp = NewCopList(0x3000);
  SetupPlayfield(MODE_LORES, S_DEPTH, X(0), Y(0), S_WIDTH, S_HEIGHT);

  cp = MakeCopperList(cp);
  CopListActivate(cp);

  //EnableDMA(DMAF_RASTER | DMAF_SPRITE | DMAF_BLITTER | DMAF_BLITHOG);
  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
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
  short vpos = VPSTART + (framen>>1);
  short ffcross = 0;
  (void) ffcross;
  (void) vpos;
  (void) i;

#if __ANIMATE
  
  for(i = 0; i < NTRANSITIONS*3; ++i) {
    // TODO: lines closer to viewer should be taller to keep perspective
    vpos +=  1;
    
#if 1
    // Calculate where the crossing now occurs
    if(!ffcross && vpos > 0xff){     
      ffcross = 1;

      ciTransition[i]->wait.vp = vpos & 0xFF;
      ciTransition[i]->wait.hp = 1;
      i++;
      // Insert safe wait
      ciTransition[i]->wait.vp = 0xFF;
      ciTransition[i]->wait.hp = 0xDF;
      
      i++;
      ciTransition[i]->wait.vp = vpos & 0xFF;
      ciTransition[i]->wait.hp = 1;
      
      
      Log("VPOS overflow at wait number %d\n", i);
    } else {
      ciTransition[i]->wait.vp = vpos & 0xFF;
      ciTransition[i]->wait.hp =  1;
      i++;
      ciTransition[i]->wait.vp = vpos & 0xFF;
      ciTransition[i]->wait.hp =  1;
      i++;
      ciTransition[i]->wait.vp = vpos & 0xFF;
      ciTransition[i]->wait.hp =  1;

    }
#endif
    
  }
#endif
  framen++;
  framen = framen % (ROWHEIGHT * texture_bp_height * 2);
  TaskWaitVBlank();
}

EFFECT(Roller, Load, UnLoad, Init, Kill, Render, VBlank);
