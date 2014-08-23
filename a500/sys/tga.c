#include <proto/dos.h> 

#include "memory.h"
#include "pixmap.h"
#include "tga.h"

#define TGA_NOIMAGE   0
#define TGA_CMAP      1
#define TGA_RGB       2
#define TGA_GRAY      3
#define TGA_RLE_CMAP  9
#define TGA_RLE_RGB   10
#define TGA_RLE_GRAY  11

typedef struct TgaHeader {
  UBYTE idLength;
  UBYTE cmapPresent;
  UBYTE imageType;
  UWORD cmapFirst;
  UWORD cmapLength;
  UBYTE cmapEntrySize;
  UWORD xOrigin;
  UWORD yOrigin;
  UWORD width;
  UWORD height;
  UBYTE depth;
  UBYTE descriptor;
} __attribute__((packed)) TgaHeaderT;

__regargs PixmapT *LoadTGA(const char *filename, PixmapTypeT type) {
  BPTR fh = Open(filename, MODE_OLDFILE);
  PixmapT *pixmap = NULL;
  UBYTE *data = NULL;
  TgaHeaderT tga;

  if (!fh) {
    Log("File '%s' missing.\n", filename);
    goto error;
  }

  if (Read(fh, &tga, sizeof(TgaHeaderT)) != sizeof(TgaHeaderT))
    goto error;

  tga.cmapFirst = swap8(tga.cmapFirst);
  tga.cmapLength = swap8(tga.cmapLength);
  tga.xOrigin = swap8(tga.xOrigin);
  tga.yOrigin = swap8(tga.yOrigin);
  tga.width = swap8(tga.width);
  tga.height = swap8(tga.height);

  (void)Seek(fh, tga.idLength, OFFSET_CURRENT);

  /* TODO: RLE compression is not handled. */
  if (type == PM_GRAY && tga.imageType == TGA_GRAY && tga.depth == 8)
  {
    ULONG imgSize = tga.width * tga.height;

    pixmap = NewPixmap(tga.width, tga.height, PM_GRAY, MEMF_PUBLIC);

    if (Read(fh, pixmap->pixels, imgSize) != imgSize)
      goto error;
  }
  else if (type == PM_CMAP && tga.imageType == TGA_CMAP && tga.depth == 8 &&
           tga.cmapEntrySize == 24)
  {
    ULONG imgSize = tga.width * tga.height;
    ULONG palSize = tga.cmapLength * 3;

    pixmap = NewPixmap(tga.width, tga.height, PM_CMAP, MEMF_PUBLIC);
    pixmap->palette = NewPalette(tga.cmapLength);

    data = AllocAutoMem(palSize, MEMF_PUBLIC);

    if (Read(fh, data, palSize) != palSize)
      goto error;

    /* TGA palette is defined as BGR value. */
    {
      ColorT *palette = pixmap->palette->colors;
      UBYTE *colors = data;
      WORD n = tga.cmapLength;

      do {
        palette->b = *colors++;
        palette->g = *colors++;
        palette->r = *colors++;
        palette++;
      } while (--n);
    }

    FreeAutoMem(data);

    data = AllocAutoMem(imgSize, MEMF_PUBLIC);

    if (Read(fh, data, imgSize) != imgSize)
      goto error;

    /*
     * Assume descriptor field is 0 - i.e. image starts from bottom left.
     */
    {
      UBYTE *pixels = pixmap->pixels;
      WORD y = tga.height;
      
      do {
        UBYTE *colors = data + tga.width * (y - 1);
        WORD x = tga.width;

        do {
          *pixels++ = *colors++;
        } while (--x);
      } while (--y);
    }

    FreeAutoMem(data);
  } 
  else if (type == PM_RGB4 && tga.imageType == TGA_RGB && tga.depth) 
  {
    ULONG imgSize = tga.width * tga.height * 3;

    pixmap = NewPixmap(tga.width, tga.height, PM_RGB4, MEMF_PUBLIC);

    data = AllocAutoMem(imgSize, MEMF_PUBLIC);

    if (Read(fh, data, imgSize) != imgSize)
      goto error;

    /*
     * TGA true-color pixels are defined as BGR value.
     * Assume descriptor field is 0 - i.e. image starts from bottom left.
     */
    {
      UWORD *pixels = pixmap->pixels;
      UWORD rowSize = tga.width * 3;
      WORD y = tga.height;
      
      do {
        UBYTE *colors = data + rowSize * (y - 1);
        WORD x = tga.width;

        do {
          UBYTE b = *colors++;
          UBYTE g = *colors++;
          UBYTE r = *colors++;
          *pixels++ = ((r & 0xf0) << 4) | (g & 0xf0) | ((b & 0xf0) >> 4);
        } while (--x);
      } while (--y);
    }
    FreeAutoMem(data);
  }

  goto end;

error:
  if (data)
    FreeAutoMem(data);
  if (pixmap) {
    if (pixmap->palette)
      DeletePalette(pixmap->palette);
    DeletePixmap(pixmap);
  }

end:
  if (fh)
    Close(fh);
  return pixmap;
}
