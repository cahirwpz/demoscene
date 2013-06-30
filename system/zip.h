#ifndef __SYSTEM_ZIP_H__
#define __SYSTEM_ZIP_H__

#include "std/types.h"

typedef struct ZipFile {
  uint32_t comp_size;
  uint32_t orig_size;
  uint32_t offset;
  uint32_t crc32;
  char name[0];
} ZipFileT;

typedef struct Zip {
  uint32_t fh;
  uint32_t num;
  ZipFileT *entry[0];
} ZipT;

ZipT *ZipOpen(const char *filename);
void *ZipRead(ZipT *zip, const char *path, uint32_t *sizeptr);
void ZipClose(ZipT *zip);

#endif
