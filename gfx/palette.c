#include <string.h>

#include "system/memory.h"
#include "system/fileio.h"
#include "system/debug.h"
#include "gfx/palette.h"

PaletteT *NewPalette(size_t num) {
  PaletteT *palette = NEW_S(PaletteT);

  if (palette) {
    palette->num = num;
    palette->colors = NEW_A(ColorT, num);

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

  uint16_t num = data[0];

  PaletteT *palette = NewPalette(num);

  if (palette)
    memcpy(palette->colors, &data[1], num);

  LOG("Palette '%s' has %ld colors.\n",
      fileName, (ULONG)num);

  DELETE(data);

  return palette;
}

void DeletePalette(PaletteT *palette) {
  if (palette)
    DELETE(palette->colors);

  DELETE(palette);
}

