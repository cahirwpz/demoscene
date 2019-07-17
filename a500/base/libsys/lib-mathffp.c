#include <common.h>

static char __mathffpname[] = "mathffp.library";
void *MathBase[2] = { NULL, __mathffpname };
ADD2LIB(MathBase);
