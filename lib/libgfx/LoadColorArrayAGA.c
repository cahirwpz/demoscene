#include <custom.h>
#include <palette.h>

void LoadColorArrayAGA(const rgb *colors, short count, int start) {
  short n = min((short)count, (short)(256 - start)) - 1;

  do {
    SetColorAGA(start++, *colors++);
  } while (--n != -1);
}
