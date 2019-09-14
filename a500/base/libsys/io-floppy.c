#include "config.h"
#include "memory.h"
#include "io.h"
#include "floppy.h"

#if USE_IO_FLOPPY

#define TD_TRACK (TD_SECTOR * NUMSECS)
#define TD_DISK (TD_TRACK * 160)

#define IOF_EOF 0x0002
#define IOF_ERR 0x8000

typedef struct {
  u_int offset;
  u_int length;
  char *name;
} FileEntryT;

typedef struct {
  char *names;
  FileEntryT file[0];
} RootDirT;

static RootDirT *rootDir = NULL;

struct File {
  u_int offset;
  u_int length;

  u_short flags;
  u_int pos;

  struct {
    u_int left;
    u_char *pos;
    u_char *track;
  } buf;
};

ALIAS(Print, Log);

static inline bool ReadTrack(void *data, int offset) {
  FloppyTrackRead(div16(offset, TD_TRACK));
  FloppyTrackDecode(data);
  return true;
}

static __regargs bool FillBuffer(FileT *file) {
  u_int abspos = file->offset + file->pos;
  u_int waste = mod16(abspos, TD_TRACK);

  if (file->pos == file->length) {
    file->flags |= IOF_EOF;
    return false;
  }

  if (!ReadTrack(file->buf.track, abspos - waste)) {
    file->flags |= IOF_ERR;
    return false;
  }

  file->buf.pos = file->buf.track + waste;
  file->buf.left = min(file->length - file->pos, TD_TRACK - waste);
  file->pos += file->buf.left;
  return true;
}

static __regargs FileT *NewFile(int length, int offset) {
  FileT *file = MemAlloc(sizeof(FileT), MEMF_PUBLIC|MEMF_CLEAR);

  file->length = length;
  file->offset = offset;
  file->buf.track = MemAlloc(TD_TRACK, MEMF_PUBLIC);
  file->buf.pos = file->buf.track;

  return file;
}

static FileEntryT *LookupFile(const char *path asm("d1")) {
  FileEntryT *entry = rootDir->file;

  for (entry = rootDir->file; entry->name; entry++)
    if (strcmp(path, entry->name) == 0)
      return entry;

  return NULL;
}

FileT *OpenFile(const char *path asm("d1"), u_short flags asm("d0") __unused) {
  FileEntryT *entry;
  if ((entry = LookupFile(path))) {
    Log("Found '%s', length: %d, offset: %d\n",
        path, entry->length, entry->offset);
    return NewFile(entry->length, entry->offset);
  }
  return NULL;
}

void CloseFile(FileT *file asm("a0")) {
  if (file) {
    MemFree(file->buf.track);
    MemFree(file);
  }
}

bool FileRead(FileT *file asm("a0"), void *buf asm("d2"), u_int size asm("d3")) {
  if (!file || size == 0 || (file->flags & IOF_ERR))
    return false;

  // Log("[FileRead] $%p $%p %d\n", file, buf, size);

  while (size > 0) {
    if (!file->buf.left && !FillBuffer(file))
      break;

    {
      /* Read to the end of buffer or less. */
      int length = min(size, file->buf.left);

      memcpy(buf, file->buf.pos, length);

      file->buf.pos += length;
      file->buf.left -= length;
      buf += length; size -= length;
    }
  }

  return size == 0; /* have we read everything? */
}

bool FileSeek(FileT *file asm("a0"), int pos asm("d2"), int mode asm("d3")) {
  if (!file || (file->flags & IOF_ERR))
    return false;
  
  // Log("[FileSeek] $%p %d %d\n", file, pos, mode);

  file->flags &= ~IOF_EOF;

  if (mode == SEEK_CUR) {
    pos += file->pos - file->buf.left;
    mode = SEEK_SET;
  }

  if (mode == SEEK_SET) {
    int bufsize = file->buf.pos - file->buf.track + (int)file->buf.left;
    int bufstart = file->pos - bufsize;
    int bufend = file->pos;

    /* New position is within buffer boundaries. */
    if ((pos >= bufstart) && (pos < bufend)) {
      file->buf.pos = file->buf.track + pos - bufstart;
      file->buf.left = file->pos - pos;
      return true;
    }

    /* New position is not within file. */
    if ((pos < 0) || (pos > (int)file->length))
      return false;

    file->pos = pos;
    return FillBuffer(file);
  }

  return false;
}

int GetFileSize(const char *path asm("d1")) {
  FileEntryT *entry;
  if ((entry = LookupFile(path)))
    return entry->length;
  return -1;
}

int GetCursorPos(FileT *file asm("a0")) {
  if (file && !(file->flags & IOF_ERR))
    return file->pos - file->buf.left;
  return -1;
}

#define ONSTACK(x) (&(x)), sizeof((x))

typedef struct {
  u_int offset;
  u_int length;
} DirEntT;

typedef struct {
  u_short files;
  u_short names;
  DirEntT dirent[0];
} DirT;

static void ReadDir(void) {
  DirT dir;
  FileT *fh;
  u_int dirLen, rootDirLen;

  /* Create a file that represent whole floppy disk without boot sector. */
  fh = NewFile(TD_DISK - TD_SECTOR * 2, TD_SECTOR * 2);

  /* read directory header */
  FileRead(fh, ONSTACK(dir));

  dirLen = sizeof(DirEntT) * dir.files + sizeof(DirT);
  rootDirLen = sizeof(FileEntryT) * (dir.files + 1) + sizeof(RootDirT);

  rootDir = MemAlloc(rootDirLen, MEMF_PUBLIC|MEMF_CLEAR);
  rootDir->names = MemAlloc(dir.names, MEMF_PUBLIC);

  /* read directory entries */
  {
    short n = dir.files;
    FileEntryT *entry = rootDir->file;

    do {
      FileRead(fh, entry++, sizeof(DirEntT));
    } while (--n);
  }

  /* read filenames */
  FileSeek(fh, align(dirLen, TD_SECTOR), SEEK_SET);
  FileRead(fh, rootDir->names, dir.names);
  CloseFile(fh);

  /* associate names with file entries */
  {
    char *name = rootDir->names;
    FileEntryT *entry = rootDir->file;

    while (entry->length) {
      entry->name = name;
      while (*name++); /* move to next string */
      entry++;
    }
  }
}

void InitIoFloppy(void) {
  InitFloppy();
  ReadDir();
}

void KillIoFloppy(void) {
  if (rootDir) {
    MemFree(rootDir->names);
    MemFree(rootDir);
    rootDir = NULL;
  }
  KillFloppy();
}

ADD2INIT(InitIoFloppy, -10);
ADD2EXIT(KillIoFloppy, -10);

#endif
