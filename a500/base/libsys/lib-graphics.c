#include <common.h>

static char __graphicsname[] = "graphics.library";
void *GfxBase[2] = { NULL, __graphicsname };
ADD2LIB(GfxBase);
