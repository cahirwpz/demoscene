#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "std/types.h"
#include "tinf.h"

#define ZIP_FILE_SIG  0x04034b50
#define ZIP_ENTRY_SIG 0x02014b50
#define ZIP_DIR_SIG   0x06054b50

typedef struct DiskFileInfo {
  uint32_t signature;
  uint16_t min_version;
  uint16_t flags;
  uint16_t comp_method;
  uint16_t modify_time;
  uint16_t modify_date;
  uint32_t crc32;
  uint32_t comp_size;
  uint32_t orig_size;
  uint16_t name_len; 
  uint16_t extra_len;
} __attribute__((packed)) DiskFileInfoT;

typedef struct DiskEntry {
  uint32_t signature;
  uint16_t version;
  uint16_t min_version;
  uint16_t flags;
  uint16_t comp_method;
  uint16_t modify_time;
  uint16_t modify_date;
  uint32_t crc32;
  uint32_t comp_size;
  uint32_t orig_size;
  uint16_t name_len; 
  uint16_t extra_len;
  uint16_t comment_len;
  uint16_t disk_num;
  uint16_t internal_attrs;
  uint32_t external_attrs;
  uint32_t offset;
} __attribute__((packed)) DiskEntryT;

typedef struct DiskDir {
  uint32_t signature;
  uint16_t disk_num;
  uint16_t disk_dir_offset;
  uint16_t disk_entries;
  uint16_t entries;
  uint32_t size;
  uint32_t offset;
  uint16_t comment_len;
} __attribute__((packed)) DiskDirT;

typedef struct ZipFile {
  uint32_t comp_size;
  uint32_t orig_size;
  uint32_t offset;
  uint32_t crc32;
  char name[0];
} ZipFileT;

typedef struct Zip {
  FILE *file;
  uint32_t num;
  ZipFileT *entry[0];
} ZipT;

ZipT *ZipOpen(const char *filename) {
  FILE *zipFile;
  ZipT *zip = NULL;

  if ((zipFile = fopen(filename, "r"))) {
    DiskDirT dir;
    int i;

    fseek(zipFile, -sizeof(DiskDirT), SEEK_END);
    fread(&dir, sizeof(DiskDirT), 1, zipFile);
    assert(dir.signature == ZIP_DIR_SIG);

    zip = malloc(sizeof(ZipT) + sizeof(ZipFileT *) * dir.entries);
    zip->num = dir.entries;

    fseek(zipFile, dir.offset, SEEK_SET);

    for (i = 0; i < dir.entries; i++) {
      DiskEntryT entry;
      ZipFileT *file;

      fread(&entry, sizeof(DiskEntryT), 1, zipFile);
      assert(entry.signature == ZIP_ENTRY_SIG);
      assert(entry.comp_method == 0 || entry.comp_method == 8);

      file = malloc(sizeof(ZipFileT) + entry.name_len + 1);
      file->crc32 = entry.crc32;
      file->offset = entry.offset;
      file->comp_size = entry.comp_size;
      file->orig_size = entry.orig_size;
      file->name[entry.name_len] = '\0';
      fread(file->name, entry.name_len, 1, zipFile);
      fseek(zipFile, entry.comment_len + entry.extra_len, SEEK_CUR);

      zip->entry[i] = file;
    }

    for (i = 0; i < dir.entries; i++) {
      DiskFileInfoT info;

      fseek(zipFile, zip->entry[i]->offset, SEEK_SET);
      fread(&info, sizeof(DiskFileInfoT), 1, zipFile);
      assert(info.signature == ZIP_FILE_SIG);

      fseek(zipFile, info.name_len + info.extra_len, SEEK_CUR);
      zip->entry[i]->offset = ftell(zipFile);
    }

    zip->file = zipFile;
  }

  return zip;
}

void *ZipRead(ZipT *zip, const char *path, uint32_t *size) {
  int i;
  ZipFileT *entry = NULL;

  for (i = 0; i < zip->num; i++) {
    if (!strcmp(zip->entry[i]->name, path)) {
      entry = zip->entry[i];
      break;
    }
  }

  if (entry) {
    void *data = malloc(entry->comp_size);

    fread(data, entry->comp_size, 1, zip->file);

    *size = entry->comp_size;

    if (entry->comp_size != entry->orig_size) {
      void *orig_data = malloc(entry->orig_size);
      unsigned int destLen;
      *size = entry->orig_size;
      tinf_uncompress(orig_data, &destLen, data, entry->comp_size);
      free(data);
      data = orig_data;
    }

    assert(tinf_crc32(data, entry->orig_size) == entry->crc32);

    return data;
  }

  return NULL;
}

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

void ZipClose(ZipT *zip) {
  int i;

  for (i = 0; i < zip->num; i++)
    free(zip->entry[i]);

  fclose(zip->file);
  free(zip);
}

int main(int argc, char **argv) {
  int i;

  tinf_init();

  for (i = 1; i < argc; i++) {
    ZipT *zip = ZipOpen(argv[i]);
    ZipList(zip);
    {
      uint32_t size;
      void *data;

      if ((data = ZipRead(zip, "gfx/trianglef.c", &size))) {
        fwrite(data, size, 1, stdout);
        free(data);
      } else {
        puts("No such file!");
      }
    }
    ZipClose(zip);
  }

  return 0;
}
