#include <stdarg.h>
#include <proto/exec.h>
#include <proto/dos.h> 

#include "memory.h"
#include "io.h"

#ifdef IO_DOS

#define SECTOR 512

#define IOF_EOF 0x0002
#define IOF_ERR 0x8000

struct File {
  BPTR handle;
  UWORD flags;

  LONG pos;
  
  struct {
    LONG left;
    UBYTE *pos;
    UBYTE data[0];
  } buf;
};

#define BUFLEN 80

struct {
  BPTR fh;
  LONG length;
  char data[BUFLEN];
} console;

static void ConsoleWrite(char c asm("d0")) {
  console.data[console.length++] = c;

  if (console.length == BUFLEN) {
    Write(console.fh, console.data, console.length);
    console.length = 0;
  }
}

void Print(const char *format, ...) {
  va_list args;

  console.length = 0;

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())ConsoleWrite, NULL);
  va_end(args);

  Write(console.fh, console.data, console.length);
}

FileT *OpenFile(CONST STRPTR path asm("d1"), UWORD flags asm("d0")) {
  FileT *file = MemAlloc(sizeof(FileT) + ((flags & IOF_BUFFERED) ? SECTOR : 0),
                         MEMF_PUBLIC|MEMF_CLEAR);

  if ((file->handle = Open(path, MODE_OLDFILE))) {
    file->flags = flags;
    file->buf.pos = file->buf.data;
    return file;
  }

  MemFree(file);
  return NULL;
}

void CloseFile(FileT *file asm("a0")) {
  if (file) {
    Close(file->handle);
    MemFree(file);
  }
}

BOOL FileRead(FileT *file asm("a0"), APTR buf asm("d2"), ULONG size asm("d3")) {
  if (!file || size == 0 || (file->flags & IOF_ERR))
    return FALSE;

  // Log("[FileRead] $%lx $%lx %ld\n", (LONG)file, (LONG)buf, (LONG)size);

  if (file->flags & IOF_BUFFERED) {
    while (size > 0) {
      if (file->buf.left == 0) {
        if (file->flags & IOF_EOF) break;
        /* Buffer is empty - fill it. */
        if (size >= SECTOR) {
          /* Read some sectors directly into provided buffer. */
          ULONG length = size & ~(SECTOR - 1);
          LONG actual = Read(file->handle, buf, length);

          if (actual < 0) { file->flags |= IOF_ERR; break; }
          if (actual < length) { file->flags |= IOF_EOF; }

          file->pos += actual;
          buf += actual; size -= actual;
        } else {
          /* Read one sector into file buffer. */
          LONG actual = Read(file->handle, file->buf.data, SECTOR);

          if (actual < 0) { file->flags |= IOF_ERR; break; }
          if (actual < SECTOR) { file->flags |= IOF_EOF; }

          file->pos += actual;
          file->buf.pos = file->buf.data;
          file->buf.left = actual;
        }
      } else {
        /* Read to the end of buffer or less. */
        LONG length = min(size, file->buf.left);

        CopyMem(file->buf.pos, buf, length);

        file->buf.pos += length;
        file->buf.left -= length;
        buf += length; size -= length;
      }
    }
  } else {
    LONG actual = Read(file->handle, buf, size);

    if (actual < 0) file->flags |= IOF_ERR;
    if (!actual) file->flags |= IOF_EOF;

    file->pos += actual;

    size -= actual;
  }

  return size == 0; /* have we read everything? */
}

BOOL FileSeek(FileT *file asm("a0"), LONG pos asm("d2"), LONG mode asm("d3")) {
  if (file && !(file->flags & IOF_ERR)) {
    // Log("[FileSeek] $%lx %ld %ld\n", (LONG)file, (LONG)pos, (LONG)mode);
    
    file->flags &= ~IOF_EOF;

    if (file->flags & IOF_BUFFERED) {
      if (mode == SEEK_CUR) {
        pos += file->pos - file->buf.left;
        mode = SEEK_SET;
      }

      if (mode == SEEK_SET) {
        LONG bufsize = file->buf.pos - file->buf.data + (LONG)file->buf.left;
        LONG bufstart = file->pos - bufsize;
        LONG bufend = file->pos;

        if ((pos >= bufstart) && (pos < bufend)) {
          /* New position is within buffer boundaries. */
          file->buf.pos = file->buf.data + pos - bufstart;
          file->buf.left = file->pos - pos;
        } else {
          /* Read buffer from new position (aligned to sector size). */
          LONG result;

          /* Go to given position and fail if not possible. */
          if (Seek(file->handle, pos, mode) != 0)
            return FALSE;

          /* Go back to the beginning of current sector. */
          (void)Seek(file->handle, pos & ~(SECTOR - 1), mode);

          /* Read one sector into file buffer. */
          result = Read(file->handle, file->buf.data, SECTOR);

          if (result < 0) { file->flags |= IOF_ERR; return FALSE; }
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

          return TRUE;
        }
      }
    } else {
      return Seek(file->handle, pos, mode) >= 0;
    }
  }

  return FALSE;
}

LONG GetFileSize(CONST STRPTR path asm("d1")) {
  LONG size = -1;
  BPTR fh;

  if ((fh = Lock(path, ACCESS_READ))) {
    struct FileInfoBlock fib;

    if (Examine(fh, &fib))
      size = fib.fib_Size;

    UnLock(fh);
  }

  return size;
}

LONG GetCursorPos(FileT *file asm("a0")) {
  if (file && !(file->flags & IOF_ERR)) {
    if (file->flags & IOF_BUFFERED)
      return file->pos - file->buf.left;
    else
      return file->pos;
  }
  return -1;
}

STRPTR __cwdpath; /* symbol is defined in common area and can be overridden */
static BPTR oldcwd = NULL;

void InitIoDos() {
  console.fh = Output();

  if (__cwdpath) {
    BPTR lock;
    Log("[Init] Current work dir: \"%s\"\n", __cwdpath);
    if ((lock = Lock(__cwdpath, ACCESS_READ))) {
      oldcwd = CurrentDir(lock);
    } else {
      Log("Lock() failed with %ld\n", IoErr());
      exit();
    }
  }
}

void KillIoDos() {
  if (oldcwd) {
    Log("[Quit] Restoring original work dir.\n");
    UnLock(CurrentDir(oldcwd));
  }
}

ADD2INIT(InitIoDos, -10);
ADD2EXIT(KillIoDos, -10);

#endif
