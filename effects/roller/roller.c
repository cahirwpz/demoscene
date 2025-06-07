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
#define NTRANSITIONS (7 * 16)
#define VPSTART 0xBC
#define ROWHEIGHT 1

// Debug / work in progress switches
// Disable animation (do not rewrite copperlist in Render)
#define __ANIMATE 1
// Handle setting background color when outside bitplanes
#define __HANDLEBG 1
// Narrow the area where we set color 0 to pallete
#define LFRAME ((0x3E + 2) << 1)
#define RFRAME ((0xDF - 2) << 1)

static CopInsT *ciColor[NTRANSITIONS];

// maximum 58 - otherwise we drop FPS
#define LHSIZE 58
static char lineheights[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                             1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2,
                             1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3,
                             3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};

#include "data/roller-bg.c"
#include "data/magland16.c"

typedef u_short palentry_t;

static CopListT *MakeCopperList(CopListT *cp) {
  short i, j;
  short vp = VPSTART;
  CopSetupBitplanes(cp, &roller_bp, S_DEPTH);
  CopSetColor(cp, 0, 0x000);

  for (i = 0; i < NTRANSITIONS;) {
    CopWaitMask(cp, VP(vp), HP(0x00), 0xFF, 0xFF);
    CopSetColor(cp, 0, 0x000);
    CopWaitMask(cp, VP(vp), HP(LFRAME), 0x00, 0xFF);

    // Colors are overwritten in Render anyway
    ciColor[i++] = CopSetColor(cp, 0, 0x000);
    for (j = 1; j < 16; j++)
      CopSetColor(cp, j, 0x000);

    CopWait(cp, VP(vp), HP(RFRAME)); // This one cannot be masked
    CopSetColor(cp, 0, 0x000);

    vp += 1;
  }

  return CopListFinish(cp);
}

static void Init(void) {
  // TODO: calculate copper list length
  CopListT *cp = NewCopList(0x1000);
  SetupPlayfield(MODE_LORES, S_DEPTH, X(0), Y(0), S_WIDTH, S_HEIGHT);

  cp = MakeCopperList(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  CopperStop();
  BlitterStop();

  DeleteCopList(cp);
}

// Patch the coppper instructions in memory
static void Animate(short framen) {
  char *lp = lineheights;
  short lh = lineheights[0];
  short i, j;
 
  for (i = 0, j = 0; j <= LHSIZE; i++) {
    // Make lines closer to viewer should be taller to match perspective
    short index = ((j - framen) << 4) & 0x1FF;
    // texture right now has size 16*54 = 864, but the % operation kills perf.
    u_char *pixel = &texture_bp_pixels[index];
    CopInsT *ci = ciColor[i];

    // funroll loops :)
    CopInsSet16(&ci[0], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[1], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[2], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[3], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[4], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[5], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[6], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[7], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[8], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[9], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[10], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[11], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[12], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[13], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[14], texture_pal_colors[*pixel++]);
    CopInsSet16(&ci[15], texture_pal_colors[*pixel++]);

    // Next line?
    lh--;
    if (lh == 0) {
      lh = *lp;
      lp++;
      j++;
    }
  }
}

PROFILE(Roller);

static void Render(void) {
  static short framen = 0;

  ProfilerStart(Roller);
  {
#if __ANIMATE
    Animate(framen);
    framen++;
    framen &= 0x3F;
#endif
  }
  ProfilerStop(Roller);
  
  TaskWaitVBlank();
}

EFFECT(Roller, NULL, NULL, Init, Kill, Render, NULL);
