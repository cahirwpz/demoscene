#include <proto/exec.h>
#include <proto/graphics.h>

#include <stdarg.h>

#include "std/math.h"
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
  float fps;
  int i;

  for (i = 1; i < FRAMERATE; i++)
    frames[i-1] = frames[i];

  frames[--i] = frameNumber;

  fps = (float)(FRAMERATE * FRAMERATE) / (float)(frames[FRAMERATE - 1] - frames[0]);

  return (fps > FRAMERATE) ? FRAMERATE : fps;
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
  struct RastPort *rastPort = GetCurrentRastPort();
  int fps = (int)(100 * CalculateFramesPerSecond(frameNumber));
  int width = rastPort->BitMap->BytesPerRow * 8;
  char text[6];

  PrintToString(text, sizeof(text), "%2ld.%02ld", fps / 100, fps % 100);
  RenderText(text, width - (2 + 8 * 5), 8);
}
