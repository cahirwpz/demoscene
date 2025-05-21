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

// 16 transitions = 1 row of texture tiles
#define NTRANSITIONS (7*16)
#define VPSTART 0xC8
#define ROWHEIGHT 1

// Debug / work in progress switches
// Disable animation (do not rewrite copperlist in Render)
#define __ANIMATE 1
// Handle setting background color when outside bitplanes
#define __HANDLEBG 1
// Narrow the area where we set color 0 to pallete
#define LFRAME ((0x3E + 2) << 1)
#define RFRAME ((0xDF - 2) << 1)


//static CopInsT *ciTransition[NTRANSITIONS*3];
static CopInsT *ciColor[NTRANSITIONS];

#include "data/roller-bg.c"
#include "data/magland16.c"

typedef u_short palentry_t;

static CopListT *MakeCopperList(CopListT *cp) {
  palentry_t *p = texture_pal_colors;
  short i, j, k;
  short vp = VPSTART;
  short ffcross = 0;
  (void) i; (void) ffcross; (void) ciColor; (void) vp; (void) j; (void) p;
  CopSetupBitplanes(cp, &roller_bp, S_DEPTH);
  CopSetColor(cp, 0, 0x033);

  k = 0;

  /////CopWait(cp, VP(vp), HP(0x40));
  for(i = 0; i < NTRANSITIONS; ) {

    CopWaitMask(cp, VP(vp), HP(0x00), 0xFF, 0xFF);
    CopSetColor(cp, 0, 0xF00);
    CopWaitMask(cp, VP(vp), HP(LFRAME), 0x00, 0xFF);

    // Colors are overwritten in Render anyway
    ciColor[i++] = CopSetColor(cp, 0, 0x000);
    for(j = 1; j < 16; j++){
      CopSetColor(cp, j, j << 4);
    }
    
    CopWait(cp, VP(vp), HP(RFRAME)); // This one cannot be masked
    CopSetColor(cp, 0, 0x00F);

    vp += 1;

  }

  return CopListFinish(cp);
}

static void Init(void) {
  // TODO: calculate copper list length
  CopListT *cp = NewCopList(0x3000);
  SetupPlayfield(MODE_LORES, S_DEPTH, X(0), Y(0), S_WIDTH, S_HEIGHT);

  cp = MakeCopperList(cp);
  CopListActivate(cp);

  // EnableDMA(DMAF_RASTER | DMAF_SPRITE | DMAF_BLITTER | DMAF_BLITHOG);
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

static void Render(void) {
  // Patch the coppper instructions in memory
  static short framen = 0;
  short i = 0;
  u_char *pixel = 0; // pointer to texture pixel
  (void) i; (void) pixel;
  
#if __ANIMATE 
  for(i = 0; i < NTRANSITIONS;) {
    // TODO: lines closer to viewer should be taller to match perspective
      
    //pixel = &texture_bp_pixels[(i*16 + framen) & 0xFF]; // funky, broken side movement
    pixel = &texture_bp_pixels[(i - framen) << 4 & 0xFF];

    // funroll loops :)
    ciColor[i][0].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][1].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][2].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][3].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][4].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][5].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][6].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][7].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][8].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][9].move.data  = texture_pal_colors[*pixel++];
    ciColor[i][10].move.data = texture_pal_colors[*pixel++];
    ciColor[i][11].move.data = texture_pal_colors[*pixel++];
    ciColor[i][12].move.data = texture_pal_colors[*pixel++];
    ciColor[i][13].move.data = texture_pal_colors[*pixel++];
    ciColor[i][14].move.data = texture_pal_colors[*pixel++];
    ciColor[i][15].move.data = texture_pal_colors[*pixel++];
  
    i++;
  }
#endif
  framen++;
  framen = framen & 0xF;
  TaskWaitVBlank();
}

EFFECT(Roller, NULL, NULL, Init, Kill, Render, NULL);
