#include <stdio.h>

#include "std/memory.h"
#include "system/zip.h"

void ZipList(ZipT *zip) {
  int i;

  printf("entries = %ld\n", zip->num);

  for (i = 0; i < zip->num; i++) {
    printf("%d: name='%s', offset=%ld, comp_size=%ld, orig_size=%ld, crc=%08lx\n",
           i + 1, zip->entry[i]->name, zip->entry[i]->offset,
           zip->entry[i]->comp_size, zip->entry[i]->orig_size,
           zip->entry[i]->crc32);
  }
}

int main(int argc, char **argv) {
  int i;

  for (i = 1; i < argc; i++) {
    ZipT *zip = ZipOpen(argv[i]);
    ZipList(zip);
    {
      uint32_t size;
      void *data;

      if ((data = ZipRead(zip, "gfx/trianglef.c", &size))) {
        fwrite(data, size, 1, stdout);
        MemUnref(data);
      } else {
        puts("No such file!");
      }
    }
    ZipClose(zip);
  }

  return 0;
}
