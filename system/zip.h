#ifndef __SYSTEM_ZIP_H__
#define __SYSTEM_ZIP_H__

#include "system/rwops.h"

typedef struct ZipFile {
  uint32_t comp_size;
  uint32_t orig_size;
  uint32_t offset;
  uint32_t crc32;
  char name[0];
} ZipFileT;

typedef struct Zip {
  RwOpsT *fh;
  uint32_t num;
  ZipFileT **entry;
} ZipT;

ZipT *ZipOpen(const char *filename);
RwOpsT *ZipRead(ZipT *zip, const char *path);

#endif
