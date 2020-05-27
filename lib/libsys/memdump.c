#include "rawio.h"
#include "common.h"

__regargs void MemDump(void *ptr, int n) {
  char *data = ptr;

  while (n > 0) {
    short m = min(n, 16);
    short i = 0;

    DPutChar('$');
    DPutLong((int)data);
    DPutChar(':');

    while (m--) {
      if ((i++ & 3) == 0)
        DPutChar(' ');

      DPutByte(*data++);

      n--;
    }

    DPutChar('\n');
  }
}
