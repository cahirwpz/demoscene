#include <proto/dos.h> 

#include "config.h"
#include "memory.h"
#include "io.h"

#if USE_IO_DOS

#define SECTOR 512

#define IOF_EOF 0x0002
#define IOF_ERR 0x8000

struct File {
  BPTR handle;
  u_short flags;

  int pos;
  
  struct {
    int left;
    u_char *pos;
    u_char data[0];
  } buf;
};

FileT *OpenFile(const char *path asm("d1"), u_short flags asm("d0")) {
  FileT *file = MemAlloc(sizeof(FileT) + ((flags & IOF_BUFFERED) ? SECTOR : 0),
                         MEMF_PUBLIC|MEMF_CLEAR);

  if (!(file->handle = Open(path, MODE_OLDFILE)))
    Panic("[IO] File '%s' is missing!\n", path);

  file->flags = flags;
  file->buf.pos = file->buf.data;
  return file;
}

void CloseFile(FileT *file asm("a0")) {
  if (file) {
    Close(file->handle);
    MemFree(file);
  }
}

bool FileRead(FileT *file asm("a0"), void *buf asm("d2"), u_int size asm("d3")) {
  if (!file || size == 0 || (file->flags & IOF_ERR))
    return false;

  // Log("[FileRead] $%p $%p %d\n", file, buf, size);

  if (file->flags & IOF_BUFFERED) {
    while (size > 0) {
      if (file->buf.left == 0) {
        if (file->flags & IOF_EOF) break;
        /* Buffer is empty - fill it. */
        if (size >= SECTOR) {
          /* Read some sectors directly into provided buffer. */
          u_int length = size & ~(SECTOR - 1);
          int actual = Read(file->handle, buf, length);

          if (actual < 0) { file->flags |= IOF_ERR; break; }
          if (actual < length) { file->flags |= IOF_EOF; }

          file->pos += actual;
          buf += actual; size -= actual;
        } else {
          /* Read one sector into file buffer. */
          int actual = Read(file->handle, file->buf.data, SECTOR);

          if (actual < 0) { file->flags |= IOF_ERR; break; }
          if (actual < SECTOR) { file->flags |= IOF_EOF; }

          file->pos += actual;
          file->buf.pos = file->buf.data;
          file->buf.left = actual;
        }
      } else {
        /* Read to the end of buffer or less. */
        int length = min(size, file->buf.left);

        memcpy(buf, file->buf.pos, length);

        file->buf.pos += length;
        file->buf.left -= length;
        buf += length; size -= length;
      }
    }
  } else {
    int actual = Read(file->handle, buf, size);

    if (actual < 0) file->flags |= IOF_ERR;
    if (!actual) file->flags |= IOF_EOF;

    file->pos += actual;

    size -= actual;
  }

  return size == 0; /* have we read everything? */
}

bool FileSeek(FileT *file asm("a0"), int pos asm("d2"), int mode asm("d3")) {
  if (file && !(file->flags & IOF_ERR)) {
    // Log("[FileSeek] $%p %d %d\n", file, pos, mode);
    
    file->flags &= ~IOF_EOF;

    if (file->flags & IOF_BUFFERED) {
      if (mode == SEEK_CUR) {
        pos += file->pos - file->buf.left;
        mode = SEEK_SET;
      }

      if (mode == SEEK_SET) {
        int bufsize = file->buf.pos - file->buf.data + (int)file->buf.left;
        int bufstart = file->pos - bufsize;
        int bufend = file->pos;

        if ((pos >= bufstart) && (pos < bufend)) {
          /* New position is within buffer boundaries. */
          file->buf.pos = file->buf.data + pos - bufstart;
          file->buf.left = file->pos - pos;
        } else {
          /* Read buffer from new position (aligned to sector size). */
          int result;

          /* Go to given position and fail if not possible. */
          if (Seek(file->handle, pos, mode) != 0)
            return false;

          /* Go back to the beginning of current sector. */
          (void)Seek(file->handle, pos & ~(SECTOR - 1), mode);

          /* Read one sector into file buffer. */
          result = Read(file->handle, file->buf.data, SECTOR);

          if (result < 0) { file->flags |= IOF_ERR; return false; }
          if (!result) file->flags |= IOF_EOF;

          file->pos = (pos & ~(SECTOR - 1)) + result;
          file->buf.pos = file->buf.data;
          file->buf.left = result;

          /* Move cursor within the buffer. */
          pos &= (SECTOR - 1);

          file->buf.pos += pos;
          file->buf.left -= pos;

          /* Did we encounter EOF condition? */
          if (!file->buf.left && (result < SECTOR))
            file->flags |= IOF_EOF;

          return true;
        }
      }
    } else {
      return Seek(file->handle, pos, mode) >= 0;
    }
  }

  return false;
}

int GetFileSize(const char *path asm("d1")) {
  int size = -1;
  BPTR fh;

  if ((fh = Lock(path, ACCESS_READ))) {
    struct FileInfoBlock fib;

    if (Examine(fh, &fib))
      size = fib.fib_Size;

    UnLock(fh);
  }

  return size;
}

int GetCursorPos(FileT *file asm("a0")) {
  if (file && !(file->flags & IOF_ERR)) {
    if (file->flags & IOF_BUFFERED)
      return file->pos - file->buf.left;
    else
      return file->pos;
  }
  return -1;
}

/* symbol is defined in common area and can be overridden */
const char *__cwdpath;
static BPTR oldcwd = NULL;

void InitIoDos() {
  if (__cwdpath) {
    BPTR lock;
    Log("[Init] Current work dir: \"%s\"\n", __cwdpath);
    if ((lock = Lock(__cwdpath, ACCESS_READ))) {
      oldcwd = CurrentDir(lock);
    } else {
      Log("Lock() failed with %d\n", IoErr());
      exit();
    }
  }
}

ADD2INIT(InitIoDos, -55);

void KillIoDos() {
  if (oldcwd) {
    Log("[Quit] Restoring original work dir.\n");
    UnLock(CurrentDir(oldcwd));
  }
}

ADD2EXIT(KillIoDos, -55);

#endif
