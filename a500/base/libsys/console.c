#include <graphics/text.h>
#include <proto/graphics.h>

#include "console.h"

void ConsoleInit(ConsoleT *console, BitmapT *bitmap) {
  console->bitmap = bitmap;
  console->width = bitmap->width / 8;
  console->height = bitmap->height / 8;
  console->cursor.x = 0;
  console->cursor.y = 0;

  {
    struct TextAttr textattr = {
      __DECONST(STRPTR, "topaz.font"), 8, FS_NORMAL, FPF_ROMFONT };
    console->font = OpenFont(&textattr);
  }
}

void ConsoleKill(ConsoleT *console) {
  CloseFont(console->font);
}

__regargs void ConsoleSetCursor(ConsoleT *console, u_short x, u_short y) {
  console->cursor.x = x;
  console->cursor.y = y;
}

static __regargs void ConsoleNextLine(ConsoleT *console) {
  console->cursor.x = 0;

  if (++console->cursor.y >= console->height)
    console->cursor.y = 0;
}

static __regargs void ConsoleNextChar(ConsoleT *console) {
  if (++console->cursor.x >= console->width) {
    ConsoleNextLine(console);
  }
}

__regargs void ConsoleDrawChar(ConsoleT *console, u_short x, u_short y, char c) {
  u_char *src = console->font->tf_CharData;
  u_char *dst = console->bitmap->planes[0];
  short swidth = console->font->tf_Modulo;
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

__regargs void ConsoleDrawCursor(ConsoleT *console) {
  u_char *dst = console->bitmap->planes[0];
  short dwidth = console->bitmap->bytesPerRow;
  short i = console->bitmap->width * console->cursor.y + console->cursor.x;
  short h = 7;

  do {
    dst[i] = ~dst[i]; i += dwidth;
  } while (--h >= 0);
}

__regargs void ConsoleDrawBox(ConsoleT *console, u_short x, u_short y,
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

__regargs void ConsolePutChar(ConsoleT *console, char c) {
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

__regargs void ConsolePutStr(ConsoleT *console, const char *str)
{
  char c;

  while ((c = *str++))
    ConsolePutChar(console, c);
}

static __regargs void OutputToConsole(char c, ConsoleT *console) {
  ConsolePutChar(console, c);
}

void ConsolePrint(ConsoleT *console, const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf(format, (kvprintf_fn_t *)OutputToConsole, console, args);
  va_end(args);
}
