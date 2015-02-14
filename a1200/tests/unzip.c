#include <stdio.h>

#include "std/memory.h"
#include "system/zip.h"

void ZipList(ZipT *zip) {
  int i;

  printf("entries = %d\n", zip->num);

  for (i = 0; i < zip->num; i++) {
    printf("%d: name='%s', offset=%d, comp_size=%d, orig_size=%d, crc=%08x\n",
           i + 1, zip->entry[i]->name, zip->entry[i]->offset,
           zip->entry[i]->comp_size, zip->entry[i]->orig_size,
           zip->entry[i]->crc32);
  }
}

static void Usage(const char *program) {
  printf("Usage: %s archive.zip [path1 path2 ...]\n", program);
  exit(1);
}

int main(int argc, char **argv) {
  ZipT *zip;
  int i;

  if (argc < 2)
    Usage(argv[0]);

  if ((zip = ZipOpen(argv[1]))) {
    if (argc == 2) {
      ZipList(zip);
    } else {
      for (i = 2; i < argc; i++) {
        RwOpsT *file;

        if ((file = ZipRead(zip, argv[i]))) {
          int size = IoSize(file);
          uint8_t *data = MemNew(size);

          IoRead(file, data, size);
          IoClose(file);
          fwrite(data, size, 1, stdout);
          MemUnref(data);
        } else {
          printf("%s: no such file!", argv[i]);
        }
      }
    }

    MemUnref(zip);
  }

  return 0;
}
