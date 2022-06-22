#ifndef __SYSTEM_FILESYS_H__
#define __SYSTEM_FILESYS_H__

#include <types.h>
#include <system/syscall.h>

struct File;

#ifdef _SYSTEM

#define FE_REGULAR 0
#define FE_EXEC 1

/* On disk directory entries are always aligned to 2-byte boundary. */
typedef struct FileEntry {
  u_char   reclen;   /* total size of this record in bytes */
  u_char   type;     /* type of file (1: executable, 0: regular) */
  u_short  start;    /* sector where the file begins (0..1759) */
  u_int    size;     /* file size in bytes (up to 1MiB) */
  char     name[0];  /* name of the file (NUL terminated) */
} FileEntryT;

void InitFileSys(struct File *dev);
void KillFileSys(void);
bool FileSysList(const FileEntryT **fep);
struct File *OpenFileEntry(const FileEntryT *fe);
#endif

SYSCALL1(OpenFile, struct File *, const char *, path, a0);

#endif /* !__SYSTEM_FILESYS_H__ */
