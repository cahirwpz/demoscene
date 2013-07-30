#include <dos/dos.h>
#include <proto/dos.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/zip.h"
#include "tinf/tinf.h"

#define ZIP_FILE_SIG  0x04034b50
#define ZIP_ENTRY_SIG 0x02014b50
#define ZIP_DIR_SIG   0x06054b50

static uint16_t le16toh(uint16_t little16) {
  uint8_t *v = (uint8_t *)&little16;
  return (v[1] << 8) | v[0];
}

static uint32_t le32toh(uint32_t little32) {
  uint8_t *v = (uint8_t *)&little32;
  return (v[3] << 24) | (v[2] << 16) | (v[1] << 8) | v[0];
}

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

static void ZipClose(ZipT *zip) {
  int i;

  for (i = 0; i < zip->num; i++)
    MemUnref(zip->entry[i]);

  MemUnref(zip->entry);
  Close(zip->fh);
}

TYPEDECL(ZipT, (FreeFuncT)ZipClose);

ZipT *ZipOpen(const char *filename) {
  BPTR fh;
  ZipT *zip = NULL;

  tinf_init();

  if ((fh = Open(filename, MODE_OLDFILE))) {
    DiskDirT dir;
    int i;

    Seek(fh, -sizeof(DiskDirT), OFFSET_END);
    Read(fh, &dir, sizeof(DiskDirT));
    ASSERT(le32toh(dir.signature) == ZIP_DIR_SIG, "Wrong signature!");

    zip = NewInstance(ZipT);
    zip->entry = NewTable(ZipFileT *, le16toh(dir.entries));
    zip->num = le16toh(dir.entries);

    Seek(fh, le32toh(dir.offset), OFFSET_BEGINNING);

    for (i = 0; i < zip->num; i++) {
      DiskEntryT entry;
      ZipFileT *file;
      int name_len;

      Read(fh, &entry, sizeof(DiskEntryT));
      ASSERT(le32toh(entry.signature) == ZIP_ENTRY_SIG, "Wrong signature!");
      ASSERT(le16toh(entry.comp_method) == 0 ||
             le16toh(entry.comp_method) == 8, "Unknown compression method!");

      name_len = le16toh(entry.name_len);

      file = MemNew(sizeof(ZipFileT) + name_len + 1);
      file->crc32 = le32toh(entry.crc32);
      file->offset = le32toh(entry.offset);
      file->comp_size = le32toh(entry.comp_size);
      file->orig_size = le32toh(entry.orig_size);
      file->name[name_len] = '\0';
      Read(fh, file->name, name_len);
      Seek(fh, le16toh(entry.comment_len) + le16toh(entry.extra_len),
           OFFSET_CURRENT);

      zip->entry[i] = file;
    }

    for (i = 0; i < zip->num; i++) {
      DiskFileInfoT info;

      Seek(fh, zip->entry[i]->offset, OFFSET_BEGINNING);
      Read(fh, &info, sizeof(DiskFileInfoT));
      ASSERT(le32toh(info.signature) == ZIP_FILE_SIG, "Wrong signature!");

      Seek(fh, le16toh(info.name_len) + le16toh(info.extra_len),
           OFFSET_CURRENT);
      zip->entry[i]->offset = Seek(fh, 0, OFFSET_CURRENT);
    }

    zip->fh = fh;
  }

  return zip;
}

void *ZipRead(ZipT *zip, const char *path, uint32_t *sizeptr) {
  int i;
  ZipFileT *entry = NULL;

  for (i = 0; i < zip->num; i++) {
    if (!strcmp(zip->entry[i]->name, path)) {
      entry = zip->entry[i];
      break;
    }
  }

  if (entry) {
    void *data = MemNew(entry->comp_size);
    unsigned int size;

    Seek(zip->fh, entry->offset, OFFSET_BEGINNING);
    Read(zip->fh, data, entry->comp_size);

    size = entry->comp_size;

    if (entry->comp_size != entry->orig_size) {
      void *orig_data = MemNew(entry->orig_size);
      ASSERT(tinf_uncompress(orig_data, &size, data, entry->comp_size) == TINF_OK,
             "Decompression failed!");
      MemUnref(data);
      data = orig_data;
    }

    *sizeptr = size;

    {
      uint32_t crc = tinf_crc32(0, data, size);
      ASSERT(crc == entry->crc32,
             "Bad CRC checksum (orig: $%8lx) vs. (curr: $%8lx)!", entry->crc32, crc);
    }

    return data;
  }

  return NULL;
}
