#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <exec/types.h>
#include <graphics/text.h>

#include "gfx.h"

typedef struct TextFont TextFontT;

typedef struct Console {
  BitmapT *bitmap;
  TextFontT *font;
  UWORD width, height;
  struct {
    UWORD x, y;
  } cursor;
} ConsoleT;

void ConsoleInit(ConsoleT *console, BitmapT *bitmap, TextFontT *font);
__regargs void ConsoleSetCursor(ConsoleT *console, UWORD x, UWORD y);
__regargs void ConsolePutChar(ConsoleT *console, char c);
__regargs void ConsolePutStr(ConsoleT *console, const char *str);
__regargs void ConsoleDrawChar(ConsoleT *console, UWORD x, UWORD y, char c);
__regargs void ConsoleDrawBox(ConsoleT *console, UWORD x, UWORD y, UWORD w, UWORD h);
void ConsolePrint(ConsoleT *console, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

#endif
