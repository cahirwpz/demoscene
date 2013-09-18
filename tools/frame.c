#include <proto/exec.h>
#include <proto/graphics.h>

#include <stdarg.h>

#include "std/math.h"
#include "system/vblank.h"
#include "tools/frame.h"

static void RenderText(char *text, int x, int y) {
  struct RastPort *rastPort = GetCurrentRastPort();

  SetDrMd(rastPort, JAM1);
  Move(rastPort, x, y);
  Text(rastPort, text, strlen(text));
}

static void PrintToString(char *buffer, size_t length, const char *format, ...) {
  va_list args;
  int i = 0;

  void OutputToString(char c asm("d0"), char *buffer asm("a3")) {
    if (i < length - 1)
      buffer[i++] = c;
  }

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())OutputToString, buffer);
  va_end(args);

  buffer[i] = '\0';
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
  char text[5];

  PrintToString(text, sizeof(text), "%04ld", frameNumber);
  RenderText(text, 2, 8);
}

void RenderTime(int frameNumber, float beatsPerFrame) {
  char text[20];
  int min = (frameNumber / FRAMERATE) / 60;
  int sec = (frameNumber / FRAMERATE) % 60;
  float beat = truncf((float)frameNumber / beatsPerFrame);;

  PrintToString(text, sizeof(text),
                "%4ldf %ldm%02lds %ldb", frameNumber, min, sec, (int)beat);
  RenderText(text, 2, 8);
}

void RenderFramesPerSecond(int frameNumber) {
  char text[6];
  int fps = (int)(100 * CalculateFramesPerSecond(frameNumber));

  PrintToString(text, sizeof(text), "%2ld.%02ld", fps / 100, fps % 100);
  RenderText(text, 320 - (2 + 8 * 5), 8);
}
