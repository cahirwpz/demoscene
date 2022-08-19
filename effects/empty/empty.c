#include <effect.h>

/* Search following header files for useful procedures. */
#include <custom.h>     /* custom registers definition and misc functions */   
#include <blitter.h>    /* blitter handling routines */
#include <color.h>      /* operations on RGB colors */
#include <copper.h>     /* copper list construction */
#include <bitmap.h>     /* bitmap structure */
#include <palette.h>    /* palette structure */
#include <pixmap.h>     /* pixel map (chunky) structure */
#include <sprite.h>     /* sprite structure and copper list helpers */
#include <system/interrupt.h> /* register & unregister an interrupt handler */
#include <system/memory.h>    /* dynamic memory allocator */

static void Init(void) {
}

static void Kill(void) {
}

extern void OptimizedFunction(void);

static void Render(void) {
  static u_short c = 0;
  custom->color[0] = c++;
  OptimizedFunction();
  TaskWaitVBlank();
}

EFFECT(Empty, NULL, NULL, Init, Kill, Render);
