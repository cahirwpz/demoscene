#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "types.h"
#include "gfx.h"

typedef struct ConsoleFont {
  u_short stride;
  u_char *data;
} ConsoleFontT;

typedef struct Console {
  BitmapT *bitmap;
  ConsoleFontT *font;
  u_short width, height;
  struct {
    u_short x, y;
  } cursor;
} ConsoleT;

void ConsoleInit(ConsoleT *console, ConsoleFontT *font, BitmapT *bitmap);

__regargs void ConsoleSetCursor(ConsoleT *console, u_short x, u_short y);
__regargs void ConsolePutChar(ConsoleT *console, char c);
__regargs void ConsolePutStr(ConsoleT *console, const char *str);
__regargs void ConsoleDrawChar(ConsoleT *console, u_short x, u_short y, char c);
__regargs void ConsoleDrawCursor(ConsoleT *console);
__regargs void ConsoleDrawBox(ConsoleT *console, u_short x, u_short y, u_short w, u_short h);

void ConsolePrint(ConsoleT *console, const char *format, ...)
  __attribute__ ((format (printf, 2, 3)));

#endif
