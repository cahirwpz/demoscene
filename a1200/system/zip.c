#include "std/debug.h"
#include "std/memory.h"
#include "system/zip.h"
#include "tinf/tinf.h"

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

static void ZipClose(ZipT *zip) {
  int i;

  for (i = 0; i < zip->num; i++)
    MemUnref(zip->entry[i]);

  MemUnref(zip->entry);
  IoClose(zip->fh);
}

TYPEDECL(ZipT, (FreeFuncT)ZipClose);

ZipT *ZipOpen(const char *filename) {
  RwOpsT *fh;
  ZipT *zip = NULL;

  if ((fh = RwOpsFromFile(filename, "r"))) {
    DiskDirT dir;
    int i;

    IoSeek(fh, -sizeof(DiskDirT), IO_SEEK_END);
    IoRead(fh, &dir, sizeof(DiskDirT));
    ASSERT(bswap32(dir.signature) == ZIP_DIR_SIG, "Wrong signature!");

    zip = NewInstance(ZipT);
    zip->entry = NewTable(ZipFileT *, bswap16(dir.entries));
    zip->num = bswap16(dir.entries);

    IoSeek(fh, bswap32(dir.offset), IO_SEEK_SET);

    for (i = 0; i < zip->num; i++) {
      DiskEntryT entry;
      ZipFileT *file;
      int name_len;

      IoRead(fh, &entry, sizeof(DiskEntryT));
      ASSERT(bswap32(entry.signature) == ZIP_ENTRY_SIG, "Wrong signature!");
      ASSERT(bswap16(entry.comp_method) == 0 ||
             bswap16(entry.comp_method) == 8, "Unknown compression method!");

      name_len = bswap16(entry.name_len);

      file = MemNew(sizeof(ZipFileT) + name_len + 1);
      file->crc32 = bswap32(entry.crc32);
      file->offset = bswap32(entry.offset);
      file->comp_size = bswap32(entry.comp_size);
      file->orig_size = bswap32(entry.orig_size);
      file->name[name_len] = '\0';
      IoRead(fh, file->name, name_len);
      IoSeek(fh, bswap16(entry.comment_len) + bswap16(entry.extra_len),
             IO_SEEK_CUR);

      zip->entry[i] = file;
    }

    for (i = 0; i < zip->num; i++) {
      DiskFileInfoT info;

      IoSeek(fh, zip->entry[i]->offset, IO_SEEK_SET);
      IoRead(fh, &info, sizeof(DiskFileInfoT));
      ASSERT(bswap32(info.signature) == ZIP_FILE_SIG, "Wrong signature!");

      IoSeek(fh, bswap16(info.name_len) + bswap16(info.extra_len),
             IO_SEEK_CUR);
      zip->entry[i]->offset = IoTell(fh);
    }

    zip->fh = fh;
  }

  return zip;
}

RwOpsT *ZipRead(ZipT *zip, const char *path) {
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

    IoSeek(zip->fh, entry->offset, IO_SEEK_SET);
    IoRead(zip->fh, data, entry->comp_size);

    size = entry->comp_size;

    if (entry->comp_size != entry->orig_size) {
      void *orig_data = MemNew(entry->orig_size);
      ASSERT(tinf_uncompress(orig_data, &size, data, entry->comp_size) == TINF_OK,
             "Decompression failed!");
      MemUnref(data);
      data = orig_data;
    }

    {
#ifndef NDEBUG
      uint32_t crc = tinf_crc32(0, data, size);
#endif
      ASSERT(crc == entry->crc32,
             "Bad CRC checksum (orig: $%8x) vs. (curr: $%8x)!", entry->crc32, crc);
    }

    return RwOpsFromMemory(data, size);
  }

  return NULL;
}
