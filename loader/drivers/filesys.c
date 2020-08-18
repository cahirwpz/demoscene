#include <string.h>
#include <types.h>
#include "debug.h"
#include "memory.h"
#include "filesys.h"
#include "floppy.h"

#define TD_TRACK (TD_SECTOR * NSECTORS)
#define TD_DISK (TD_TRACK * NTRACKS)

#define IOF_EOF 0x0002
#define IOF_ERR 0x8000

/* On disk directory entries are always aligned to 2-byte boundary. */
typedef struct FileEntry {
  u_char   reclen;   /* total size of this record in bytes */
  u_char   type;     /* type of file (1: executable, 0: regular) */
  u_short  start;    /* sector where the file begins (0..1759) */
  u_int    size;     /* file size in bytes (up to 1MiB) */
  char     name[0];  /* name of the file (NUL terminated) */
} FileEntryT;

static FileEntryT *NextFileEntry(FileEntryT *fe) {
  return (void *)fe + fe->reclen;
}

/* Finished by NUL character (reclen = 0). */
static FileEntryT *rootDir = NULL;

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

static inline bool ReadTrack(void *data, int offset) {
  int trknum = div16(offset, TD_TRACK);
  Log("[Floppy] Read track %d.\n", trknum);
  FloppyTrackRead(trknum);
  FloppyTrackDecode(data);
  return true;
}

static bool FillBuffer(FileT *file) {
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

static FileT *NewFile(int length, int offset) {
  FileT *file = MemAlloc(sizeof(FileT), MEMF_PUBLIC|MEMF_CLEAR);

  file->length = length;
  file->offset = offset;
  file->buf.track = MemAlloc(TD_TRACK, MEMF_PUBLIC);
  file->buf.pos = file->buf.track;

  return file;
}

static FileEntryT *LookupFile(const char *path) {
  FileEntryT *fe = rootDir;

  do {
    if (strcmp(path, fe->name) == 0)
      return fe;
    fe = NextFileEntry(fe);
  } while (fe->reclen);

  return NULL;
}

FileT *OpenFile(const char *path, __unused u_short flags) {
  FileEntryT *entry;
  if ((entry = LookupFile(path)))
    return NewFile(entry->size, entry->start * TD_SECTOR);
  return NULL;
}

void CloseFile(FileT *file) {
  if (file) {
    MemFree(file->buf.track);
    MemFree(file);
  }
}

bool FileRead(FileT *file, void *buf, u_int size) {
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

bool FileSeek(FileT *file, int pos, int mode) {
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

int GetFileSize(const char *path) {
  FileEntryT *entry;
  if ((entry = LookupFile(path)))
    return entry->size;
  return -1;
}

int GetCursorPos(FileT *file) {
  if (file && !(file->flags & IOF_ERR))
    return file->pos - file->buf.left;
  return -1;
}

#define ONSTACK(x) (&(x)), sizeof((x))

void InitFileSys(void) {
  FileT *fh;
  u_short rootDirLen;

  /* Create a file that represent whole floppy disk without boot sector. */
  fh = NewFile(TD_DISK - TD_SECTOR * 2, TD_SECTOR * 2);

  /* read directory size */
  FileRead(fh, ONSTACK(rootDirLen));

  Log("[FileSys] Reading directory of %d bytes.\n", rootDirLen);

  /* read directory entries */
  rootDir = MemAlloc(rootDirLen + 1, MEMF_PUBLIC|MEMF_CLEAR);
  FileRead(fh, rootDir, rootDirLen);
  CloseFile(fh);

  /* associate names with file entries */
  {
    FileEntryT *fe = rootDir;
    do {
      Log("[FileSys] Sector %d: %s file '%s' of %d bytes.\n",
          fe->start, fe->type ? "executable" : "regular", fe->name, fe->size);
      fe = NextFileEntry(fe);
    } while (fe->reclen);
  }
}

void KillFileSys(void) {
  if (rootDir) {
    MemFree(rootDir);
    rootDir = NULL;
  }
}
