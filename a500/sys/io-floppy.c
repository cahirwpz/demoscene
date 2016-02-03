#include <devices/trackdisk.h>
#include <exec/io.h>
#include <proto/alib.h>
#include <proto/exec.h>

#include "config.h"
#include "memory.h"
#include "io.h"

#if USE_IO_FLOPPY

#define TD_TRACK (TD_SECTOR * NUMSECS)
#define TD_DISK (TD_TRACK * 160)

#define IOF_EOF 0x0002
#define IOF_ERR 0x8000

static struct MsgPort *trackMP;
static struct IOExtTD *trackIO;

typedef struct {
  ULONG offset;
  ULONG length;
  STRPTR name;
} FileEntryT;

typedef struct {
  STRPTR names;
  FileEntryT file[0];
} RootDirT;

RootDirT *rootDir = NULL;

struct File {
  ULONG offset;
  ULONG length;

  UWORD flags;
  LONG pos;

  struct {
    LONG left;
    UBYTE *pos;
    UBYTE *track;
  } buf;
};

ALIAS(Print, Log);

/*
 * Transfer data from the track buffer to a supplied buffer.
 *
 * If the desired sector is already in the track buffer, no disk activity is
 * initiated. If the desired sector is not in the buffer, the track containing
 * that sector is automatically read in.
 *
 * Under versions of Kickstart earlier than V36, the io_Data has to point to a
 * buffer in chip memory.
 */
static inline BOOL ReadTrack(APTR data, LONG offset) {
  trackIO->iotd_Req.io_Flags = IOF_QUICK;
  trackIO->iotd_Req.io_Length = TD_TRACK;
  trackIO->iotd_Req.io_Data = data;
  trackIO->iotd_Req.io_Offset = offset;
  trackIO->iotd_Req.io_Command = CMD_READ;
  return !DoIO((struct IORequest *)trackIO);
}

static __regargs BOOL FillBuffer(FileT *file) {
  LONG abspos = file->offset + file->pos;
  LONG waste = mod16(abspos, TD_TRACK);

  if (file->pos == file->length) {
    file->flags |= IOF_EOF;
    return FALSE;
  }

  if (!ReadTrack(file->buf.track, abspos - waste)) {
    file->flags |= IOF_ERR;
    return FALSE;
  }

  file->buf.pos = file->buf.track + waste;
  file->buf.left = min(file->length - file->pos, TD_TRACK - waste);
  file->pos += file->buf.left;
  return TRUE;
}

static __regargs FileT *NewFile(LONG length, LONG offset) {
  FileT *file = MemAlloc(sizeof(FileT), MEMF_PUBLIC|MEMF_CLEAR);

  file->length = length;
  file->offset = offset;
  file->buf.track = MemAlloc(TD_TRACK, MEMF_CHIP);
  file->buf.pos = file->buf.track;

  return file;
}

static FileEntryT *LookupFile(CONST STRPTR path asm("d1")) {
  FileEntryT *entry = rootDir->file;

  do {
    if (strcmp(path, entry->name) == 0)
      return entry;
  } while (++entry);

  return NULL;
}

FileT *OpenFile(CONST STRPTR path asm("d1"), UWORD flags asm("d0")) {
  FileEntryT *entry;
  if ((entry = LookupFile(path))) {
    Log("Found '%s', length: %ld, offset: %ld\n",
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

BOOL FileRead(FileT *file asm("a0"), APTR buf asm("d2"), ULONG size asm("d3")) {
  if (!file || size == 0 || (file->flags & IOF_ERR))
    return FALSE;

  // Log("[FileRead] $%lx $%lx %ld\n", (LONG)file, (LONG)buf, (LONG)size);

  while (size > 0) {
    if (!file->buf.left && !FillBuffer(file))
      break;

    {
      /* Read to the end of buffer or less. */
      LONG length = min(size, file->buf.left);

      CopyMem(file->buf.pos, buf, length);

      file->buf.pos += length;
      file->buf.left -= length;
      buf += length; size -= length;
    }
  }

  return size == 0; /* have we read everything? */
}

BOOL FileSeek(FileT *file asm("a0"), LONG pos asm("d2"), LONG mode asm("d3")) {
  if (!file || (file->flags & IOF_ERR))
    return FALSE;
  
  // Log("[FileSeek] $%lx %ld %ld\n", (LONG)file, (LONG)pos, (LONG)mode);

  file->flags &= ~IOF_EOF;

  if (mode == SEEK_CUR) {
    pos += file->pos - file->buf.left;
    mode = SEEK_SET;
  }

  if (mode == SEEK_SET) {
    LONG bufsize = file->buf.pos - file->buf.track + (LONG)file->buf.left;
    LONG bufstart = file->pos - bufsize;
    LONG bufend = file->pos;

    /* New position is within buffer boundaries. */
    if ((pos >= bufstart) && (pos < bufend)) {
      file->buf.pos = file->buf.track + pos - bufstart;
      file->buf.left = file->pos - pos;
      return TRUE;
    }

    /* New position is not within file. */
    if ((pos < 0) || (pos > file->length))
      return FALSE;

    file->pos = pos;
    return FillBuffer(file);
  }

  return FALSE;
}

LONG GetFileSize(CONST STRPTR path asm("d1")) {
  FileEntryT *entry;
  if ((entry = LookupFile(path)))
    return entry->length;
  return -1;
}

LONG GetCursorPos(FileT *file asm("a0")) {
  if (file && !(file->flags & IOF_ERR))
    return file->pos - file->buf.left;
  return -1;
}

#define ONSTACK(x) (&(x)), sizeof((x))

typedef struct {
  ULONG offset;
  ULONG length;
} DirEntT;

typedef struct {
  UWORD files;
  UWORD names;
  DirEntT dirent[0];
} DirT;

static void ReadDir() {
  DirT dir;
  FileT *fh;
  ULONG dirLen, rootDirLen;

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
    WORD n = dir.files;
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
    STRPTR name = rootDir->names;
    FileEntryT *entry = rootDir->file;

    while (entry->length) {
      entry->name = name;
      while (*name++); /* move to next string */
      entry++;
    }
  }
}

void InitIoFloppy() {
  if (!(trackMP = CreatePort(NULL, 0L)))
    exit();
  if (!(trackIO = (struct IOExtTD *)
        CreateExtIO(trackMP, sizeof(struct IOExtTD))))
    exit();
  if (OpenDevice(TD_NAME, 0L, (struct IORequest *)trackIO, 0))
    exit();
  ReadDir();
}

void KillIoFloppy() {
  if (rootDir) {
    MemFree(rootDir->names);
    MemFree(rootDir);
    rootDir = NULL;
  }
  if (trackIO)
    CloseDevice((struct IORequest *)trackIO);
  if (trackMP)
    DeletePort(trackMP);
}

ADD2INIT(InitIoFloppy, -10);
ADD2EXIT(KillIoFloppy, -10);

#endif
