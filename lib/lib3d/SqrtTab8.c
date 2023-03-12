#include <3d.h>
#include <linkerset.h>

char SqrtTab8[256];

void InitSqrtTab8(void){
  char *data = SqrtTab8;
  short i = 16;
  short n = 0;
  char k = 0;
  do {
    short j = n;
    do { *data++ = k; } while (--j != -1);
    k += 1;
    n += 2;
  } while (--i > 0);
}

ADD2INIT(InitSqrtTab8, 0);
