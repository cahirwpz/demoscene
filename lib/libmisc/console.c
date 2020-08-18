#include <stdio.h>
#include "console.h"

void ConsoleInit(ConsoleT *console, ConsoleFontT *font, BitmapT *bitmap) {
  console->bitmap = bitmap;
  console->font = font;
  console->width = bitmap->width / 8;
  console->height = bitmap->height / 8;
  console->cursor.x = 0;
  console->cursor.y = 0;
}

void ConsoleSetCursor(ConsoleT *console, u_short x, u_short y) {
  console->cursor.x = x;
  console->cursor.y = y;
}

static void ConsoleNextLine(ConsoleT *console) {
  console->cursor.x = 0;

  if (++console->cursor.y >= console->height)
    console->cursor.y = 0;
}

static void ConsoleNextChar(ConsoleT *console) {
  if (++console->cursor.x >= console->width) {
    ConsoleNextLine(console);
  }
}

void ConsoleDrawChar(ConsoleT *console, u_short x, u_short y, char c) {
  u_char *src = console->font->data;
  u_char *dst = console->bitmap->planes[0];
  short swidth = console->font->stride;
  short dwidth = console->bitmap->bytesPerRow;
  short h = 7;

  c -= 32;
  src += (short)c;
  dst += (short)console->bitmap->width * (short)y;
  dst += (short)x;

  do {
    *dst = *src; src += swidth; dst += dwidth;
  } while (--h >= 0);
}

void ConsoleDrawCursor(ConsoleT *console) {
  u_char *dst = console->bitmap->planes[0];
  short dwidth = console->bitmap->bytesPerRow;
  short i = console->bitmap->width * console->cursor.y + console->cursor.x;
  short h = 7;

  do {
    dst[i] = ~dst[i]; i += dwidth;
  } while (--h >= 0);
}

void ConsoleDrawBox(ConsoleT *console, u_short x, u_short y,
                    u_short w, u_short h)
{
  int i;

  w += x - 1;
  h += y - 1;

  ConsoleDrawChar(console, x, y, '+');
  ConsoleDrawChar(console, x, h, '+');
  ConsoleDrawChar(console, w, y, '+');
  ConsoleDrawChar(console, w, h, '+');

  for (i = x + 1; i < w; i++) {
    ConsoleDrawChar(console, i, y, '-');
    ConsoleDrawChar(console, i, h, '-');
  }

  for (i = y + 1; i < h; i++) {
    ConsoleDrawChar(console, x, i, '|');
    ConsoleDrawChar(console, w, i, '|');
  }
}

void ConsolePutChar(ConsoleT *console, char c) {
  switch (c) {
    case '\r':
      break;
    case '\n':
      ConsoleNextLine(console);
      break;
    default:
      if (c < 32)
        return;
      ConsoleDrawChar(console, console->cursor.x, console->cursor.y, c);
      ConsoleNextChar(console);
      break;
  }
}

void ConsolePutStr(ConsoleT *console, const char *str)
{
  char c;

  while ((c = *str++))
    ConsolePutChar(console, c);
}

void ConsolePrint(ConsoleT *console, const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf((kvprintf_fn_t *)ConsolePutChar, console, format, args);
  va_end(args);
}
