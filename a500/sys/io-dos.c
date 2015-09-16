#include <stdarg.h>
#include <proto/exec.h>
#include <proto/dos.h> 

#include "memory.h"
#include "io.h"

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

static LONG length = 0;
static BPTR console = 0;

static __regargs void WriteToConsole(char *buffer) {
  if (!console)
    console = Output();

  Write(console, buffer, length);

  length = 0;
}

static void OutputToConsole(char c asm("d0"), char *buffer asm("a3")) {
  buffer[length++] = c;

  if (length == BUFLEN)
    WriteToConsole(buffer);
}

void Print(const char *format, ...) {
  char buffer[BUFLEN];
  va_list args;

  va_start(args, format);
  RawDoFmt(format, args, (void (*)())OutputToConsole, buffer);
  va_end(args);

  WriteToConsole(buffer);
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
  if (!file || size == 0)
    return FALSE;

  // Log("[FileRead] $%lx $%lx %ld\n", (LONG)file, (LONG)buf, (LONG)size);

  if (file->flags & IOF_BUFFERED) {
    while (size > 0 && !(file->flags & IOF_ERR)) {
      if (!file->buf.left && !(file->flags & IOF_EOF)) {
        /* Buffer is empty - fill it. */
        if (size >= SECTOR) {
          /* Read some sectors directly into provided buffer. */
          ULONG length = size & ~(SECTOR - 1);
          LONG actual = Read(file->handle, buf, length);

          if (actual < 0) { file->flags |= IOF_ERR; break; }
          if (actual < length) { file->flags |= IOF_EOF; break; }

          file->pos += actual;
          buf += actual; size -= actual;
        } else {
          /* Read one sector into file buffer. */
          LONG actual = Read(file->handle, file->buf.data, SECTOR);

          if (actual < 0) { file->flags |= IOF_ERR; break; }
          if (!actual) { file->flags |= IOF_EOF; break; }

          file->pos += actual;
          file->buf.pos = file->buf.data;
          file->buf.left = actual;
        }
      } else {
        /* Read to the end of buffer or less. */
        ULONG length = min(size, file->buf.left);

        CopyMem(file->buf.pos, buf, length);

        file->buf.pos += length;
        file->buf.left -= length;
        buf += length; size -= length;
        
        /* Did we encounter EOF condition? */
        if (!file->buf.left && (file->pos & (SECTOR - 1))) {
          file->flags |= IOF_EOF;
          break;
        }
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
          result = Seek(file->handle, pos, mode);
          if (result < 0) { file->flags |= IOF_ERR; return FALSE; }

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

STRPTR __cwdpath; /* symbol is defined in common area and can be overridden */
static BPTR oldcwd = NULL;

void InitIoDos() {
  if (__cwdpath) {
    Log("[Init] Current work dir: \"%s\"\n", __cwdpath);
    oldcwd = CurrentDir(Lock(__cwdpath, ACCESS_READ));
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
