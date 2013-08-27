#include <string.h>
#include <stdarg.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "gfx/palette.h"

static void DeletePalette(PaletteT *palette) {
  MemUnref(palette->colors);

  if (palette->next)
    MemUnref(palette->next);
}

static void CopyPalette(PaletteT *dst, PaletteT *src) {
  dst->colors = MemClone(src->colors);
  dst->start = src->start;
  dst->count = src->count;
  dst->next = (src->next) ? MemClone(src->next) : NULL;
}

TYPEDECL(PaletteT, (FreeFuncT)DeletePalette, (CopyFuncT)CopyPalette);

PaletteT *NewPalette(size_t count) {
  PaletteT *palette = NewInstance(PaletteT);

  palette->count = count;
  palette->colors = NewTable(RGB, count);

  return palette;
}

typedef struct DiskPalette {
  uint16_t start;
  uint16_t count;
  RGB      colors[0];
} DiskPaletteT;

PaletteT *NewPaletteFromFile(const char *fileName) {
  DiskPaletteT *file = ReadFileSimple(fileName);

  if (file) {
    PaletteT *palette = NewPalette(file->count);

    palette->start = file->start;

    LOG("Palette '%s' has %d colors.", fileName, file->count);

    MemCopy(palette->colors, file->colors, sizeof(RGB) * file->count);

    MemUnref(file);

    return palette;
  }

  return NULL;
}

bool LinkPalettes(PaletteT *palette, ...) {
  int start = 0;
  va_list ap;

  va_start(ap, palette);

  for (;;) {
    PaletteT *next = va_arg(ap, PaletteT *);

    /* Just for safety. */
    UnlinkPalettes(palette);

    palette->start = start;
    palette->next = next;

    start += palette->count;

    if (!next)
      break;

    palette = next;
  }

  va_end(ap);

  return BOOL(start <= 256);
}

void UnlinkPalettes(PaletteT *palette) {
  while (palette) {
    PaletteT *next = palette->next;

    palette->next = NULL;
    palette = next;
  }
}
