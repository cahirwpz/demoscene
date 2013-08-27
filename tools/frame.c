#include <proto/graphics.h>

#include "system/vblank.h"
#include "tools/frame.h"

static void RenderText(char *text, int x, int y) {
  struct RastPort *rastPort = GetCurrentRastPort();

  SetDrMd(rastPort, JAM1);
  Move(rastPort, x, y);
  Text(rastPort, text, strlen(text));
}

static float CalculateFramesPerSecond(int frameNumber) {
  static int frames[FRAMERATE];

  int i;

  for (i = 0; i < FRAMERATE - 1; i++)
    frames[i] = frames[i + 1];

  frames[i--] = frameNumber;

  while ((i > 0) && (frameNumber - frames[i] < FRAMERATE))
    i--;

  return (float)(FRAMERATE * FRAMERATE) / (float)(frameNumber - frames[i]);
}

void RenderFrameNumber(int frameNumber) {
  char number[5];
  int i = sizeof(number) - 1;

  number[i--] = '\0';

  while (i >= 0) {
    number[i--] = (frameNumber % 10) + '0';
    frameNumber /= 10;
  }

  RenderText(number, 2, 8);
}

void RenderFramesPerSecond(int frameNumber) {
  int fps = (int)(100 * CalculateFramesPerSecond(frameNumber));

  char number[6];
  int i = sizeof(number) - 1;

  number[i--] = '\0';

  while (i >= 3) {
    number[i--] = (fps % 10) + '0';
    fps /= 10;
  }

  number[i--] = '.';

  while (i >= 0) {
    number[i--] = (fps % 10) + '0';
    fps /= 10;
  }

  RenderText(number, 320 - (2 + 8 * 5), 8);
}
