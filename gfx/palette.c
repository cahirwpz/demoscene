#include <string.h>

#include "system/memory.h"
#include "system/fileio.h"
#include "system/debug.h"
#include "gfx/palette.h"

PaletteT *NewPalette(size_t count) {
  PaletteT *palette = NEW_S(PaletteT);

  if (palette) {
    palette->count = count;
    palette->colors = NEW_A(ColorT, count);

    if (palette->colors)
      return palette;

    DeletePalette(palette);
  }

  return NULL;
}

PaletteT *NewPaletteFromFile(const char *fileName, uint32_t memFlags) {
  uint16_t *data = ReadFileSimple(fileName, memFlags);

  if (!data)
    return NULL;

  uint16_t count = data[0];

  PaletteT *palette = NewPalette(count);

  if (palette) {
    uint8_t *raw = &data[1];

    int i;

    for (i = 0; i < count; i++) {
      palette->colors[i].r = raw[i*3];
      palette->colors[i].g = raw[i*3+1];
      palette->colors[i].b = raw[i*3+2];
    }
  }

  LOG("Palette '%s' has %ld colors.\n", fileName, (ULONG)count);

  DELETE(data);

  return palette;
}

void DeletePalette(PaletteT *palette) {
  if (palette)
    DELETE(palette->colors);

  DELETE(palette);
}

