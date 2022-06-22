#include <debug.h>
#include <common.h>
#include <string.h>
#include <types.h>
#include <system/errno.h>
#include <system/file.h>
#include <system/filesys.h>
#include <system/floppy.h>
#include <system/memory.h>

#define IOF_EOF 0x0002
#define IOF_ERR 0x8000

static const FileEntryT *NextFileEntry(const FileEntryT *fe) {
  return (void *)fe + fe->reclen;
}

static FileT *FileSysDev;
/* Finished by NUL character (reclen = 0). */
static FileEntryT *FileSysRootDir;

struct File {
  FileOpsT *ops;
  int pos;
  u_short flags;

  u_int start;
  u_int size;
};

static int FsRead(FileT *f, void *buf, u_int nbyte);
static int FsSeek(FileT *f, int offset, int whence);
static void FsClose(FileT *f);

static FileOpsT FsOps = {
  .read = FsRead,
  .write = NoWrite,
  .seek = FsSeek,
  .close = FsClose
};

static const FileEntryT *LookupFile(const char *path) {
  const FileEntryT *fe = FileSysRootDir;

  if (fe == NULL)
    return NULL;

  do {
    if (strcmp(path, fe->name) == 0)
      return fe;
    fe = NextFileEntry(fe);
  } while (fe->reclen);

  return NULL;
}

bool FileSysList(const FileEntryT **fep) {
  const FileEntryT *fe = *fep;

  fe = fe ? NextFileEntry(fe) : FileSysRootDir;

  if (!fe->reclen)
    return false;

  *fep = fe;
  return true;
}

FileT *OpenFileEntry(const FileEntryT *fe) {
  FileT *f = MemAlloc(sizeof(FileT), MEMF_PUBLIC|MEMF_CLEAR);

  f->ops = &FsOps;
  f->start = (fe->start + 2) * SECTOR_SIZE;
  f->size = fe->size;

  Debug("%s: %d+%d", path, f->start, f->size);
  return f;
}

FileT *OpenFile(const char *path asm("a0")) {
  const FileEntryT *fe;

  if ((fe = LookupFile(path)))
    return OpenFileEntry(fe);

  return NULL;
}

static void FsClose(FileT *f) {
  MemFree(f);
}

static int FsRead(FileT *f, void *buf, u_int nbyte) {
  u_int left = nbyte;
  int res;

  Assume(f != NULL);

  if (f->flags & IOF_ERR)
    return EIO;

  Debug("$%p $%p %d+%d", f, buf, f->pos, nbyte);

  left = min(left, f->size - f->pos);

  (void)FileSeek(FileSysDev, f->pos + f->start, SEEK_SET);

  if ((res = FileRead(FileSysDev, buf, left)) < 0)
    return res;

  f->pos += res;

  return res;
}

static int FsSeek(FileT *f, int offset, int whence) {
  if (f->flags & IOF_ERR)
    return EIO;
  
  Debug("$%p %d %d", f, offset, whence);

  f->flags &= ~IOF_EOF;

  if (whence == SEEK_CUR) {
    offset += f->pos;
    whence = SEEK_SET;
  }

  if (whence == SEEK_END) {
    offset = f->size - offset;
    whence = SEEK_SET;
  }

  if (whence == SEEK_SET) {
    /* New position is not within file. */
    if ((offset < 0) || (offset > (int)f->size))
      return EINVAL;

    f->pos = offset;
    return offset;
  }

  return EINVAL;
}

#define ONSTACK(x) (&(x)), sizeof((x))

void InitFileSys(FileT *dev) {
  u_short rootDirLen;

  Assume(dev != NULL);

  FileSysDev = dev;

  /* read directory size */
  FileSeek(dev, SECTOR_SIZE * 2, SEEK_SET);
  FileRead(dev, ONSTACK(rootDirLen));

  Log("[FileSys] Reading directory of %d bytes.\n", rootDirLen);

  /* read directory entries */
  FileSysRootDir = MemAlloc(rootDirLen + 1, MEMF_PUBLIC|MEMF_CLEAR);
  FileRead(dev, FileSysRootDir, rootDirLen);

  /* associate names with file entries */
  {
    const FileEntryT *fe = FileSysRootDir;
    do {
      Log("[FileSys] Sector %d: %s file '%s' of %d bytes.\n",
          fe->start, fe->type ? "executable" : "regular", fe->name, fe->size);
      fe = NextFileEntry(fe);
    } while (fe->reclen);
  }
}

void KillFileSys(void) {
  if (FileSysRootDir) {
    FileClose(FileSysDev);
    MemFree(FileSysRootDir);
    FileSysDev = NULL;
    FileSysRootDir = NULL;
  }
}

#if 0
int GetFileSize(const char *path) {
  FileEntryT *entry;
  if ((entry = LookupFile(path)))
    return entry->size;
  return ENOENT;
}

int GetCursorPos(FileT *f) {
  Assume(f != NULL);
  if (f->flags & IOF_ERR)
    return EIO;
  return f->pos;
}

void *LoadFile(const char *path, u_int memoryFlags) {
  char *data = NULL;
  int size = GetFileSize(path);

  if (size > 0 && (data = MemAlloc(size + 1, memoryFlags))) {
    FileT *f = OpenFile(path);

    if (!FileRead(f, data, size)) {
      MemFree(data);
      data = NULL;
    } else {
      /* Add extra byte and mark the end of file by zero. */
      data[size] = 0;
    }

    FileClose(f);
  }

  return data;
}
#endif
