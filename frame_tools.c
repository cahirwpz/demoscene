#include <proto/graphics.h>

#include "frame_tools.h"

void RenderFrameNumber(int frameNumber) {
  struct RastPort *rastPort = GetCurrentRastPort();

  SetDrMd(rastPort, JAM1);

  {
    char number[5];
    int i = sizeof(number) - 1;

    number[i--] = '\0';

    while (i >= 0) {
      number[i--] = (frameNumber % 10) + '0';
      frameNumber /= 10;
    }

    Move(rastPort, 2, 8);
    Text(rastPort, number, 4);
  }
}
