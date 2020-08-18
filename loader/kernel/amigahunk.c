#include <string.h>
#include "amigahunk.h"
#include "debug.h"
#include "filesys.h"
#include "memory.h"

#define FileReadVar(fh, x) FileRead((fh), &(x), sizeof(x))

#define HUNK_CODE 1001
#define HUNK_DATA 1002
#define HUNK_BSS 1003
#define HUNK_RELOC32 1004
#define HUNK_SYMBOL 1008
#define HUNK_DEBUG 1009
#define HUNK_END 1010
#define HUNK_HEADER 1011

#define HUNKF_CHIP __BIT(30)
#define HUNKF_FAST __BIT(31)

struct Hunk {
  u_int size;
  struct Hunk *next;
  char data[0];
};

static bool AllocHunks(FileT *fh, HunkT **hunkArray, int hunkCount) {
  HunkT *hunk, *prev;

  for (prev = NULL; hunkCount--; prev = hunk) {
    u_int memFlags = MEMF_CLEAR;
    u_int n;

    /* size specifiers including memory attribute flags */
    FileReadVar(fh, n);

    if (n & HUNKF_CHIP)
      memFlags |= MEMF_CHIP;
    else if (n & HUNKF_FAST)
      memFlags |= MEMF_FAST;
    else
      memFlags |= MEMF_PUBLIC;

    n *= sizeof(int);

    if (!(hunk = MemAlloc(sizeof(HunkT) + n, memFlags)))
      return false;

    hunk->size = n;
    hunk->next = NULL;

    if (prev)
      prev->next = hunk;

    *hunkArray++ = hunk;
  }

  return true;
}

static bool LoadHunks(FileT *fh, HunkT **hunkArray) {
  HunkT *hunk = *hunkArray;
  int n, hunkCode;
  bool hunkRoot = true;

  while (FileReadVar(fh, hunkCode)) {
    short hunkId = hunkCode; /* hunkId = 10xx, so we can ignore upper bits */

    if (hunkId == HUNK_CODE || hunkId == HUNK_DATA || hunkId == HUNK_BSS) {
      hunkRoot = true;
      FileReadVar(fh, n);
      if (hunkId != HUNK_BSS)
        FileRead(fh, hunk->data, n * sizeof(int));
      else
        memset(hunk->data, 0, n * sizeof(int));
      {
        const char *hunkType;

        if (hunkId == HUNK_CODE)
          hunkType = "CODE";
        else if (hunkId == HUNK_DATA)
          hunkType = "DATA";
        else
          hunkType = " BSS";

        Log("%s: %p - %p\n", hunkType, hunk->data, hunk->data + hunk->size);
      }
    } else if (hunkId == HUNK_DEBUG) {
      FileReadVar(fh, n);
      FileSeek(fh, n * sizeof(int), SEEK_CUR);
    } else if (hunkId == HUNK_RELOC32) {
      for (;;) {
        int hunkRef, hunkNum;
        FileReadVar(fh, n);
        if (n == 0)
          break;
        FileReadVar(fh, hunkNum);
        hunkRef = (int)hunkArray[hunkNum]->data;
        while (n--) {
          int reloc;
          FileReadVar(fh, reloc);
          ((int *)hunk->data)[reloc] += hunkRef;
        }
      }
    } else if (hunkId == HUNK_SYMBOL) {
      for (;;) {
        FileReadVar(fh, n);
        if (n == 0)
          break;
        FileSeek(fh, (n + 1) * sizeof(int), SEEK_CUR);
      }
    } else if (hunkId == HUNK_END) {
      if (hunkRoot) {
        hunkRoot = false;
        hunk++;
      }
    } else {
      Log("Unknown hunk $%08lx!\n", (long)hunkCode);
      return false;
    }
  }

  return true;
}

HunkT *LoadHunkList(FileT *fh) {
  HunkT **hunkArray;
  int hunkId, hunkCount;

  FileReadVar(fh, hunkId);
  hunkId &= 0x3FFFFFFF;

  if (hunkId != HUNK_HEADER)
    return NULL;

  /* Skip resident library names. */
  for (;;) {
    int n;
    FileReadVar(fh, n);
    if (n == 0)
      break;
    FileSeek(fh, n * sizeof(int), SEEK_CUR);
  }

  /*
   * number of hunks (including resident libraries and overlay hunks)
   * number of the first (root) hunk
   * number of the last (root) hunk
   */
  {
    struct {
      int hunks, first, last;
    } s;
    FileReadVar(fh, s);
    hunkCount = s.last - s.first + 1;
  }

  hunkArray = __builtin_alloca(sizeof(HunkT *) * hunkCount);
  if (AllocHunks(fh, hunkArray, hunkCount))
    if (LoadHunks(fh, hunkArray))
      return hunkArray[0];

  FreeHunkList(hunkArray[0]);
  return NULL;
}

void FreeHunkList(HunkT *hunk) {
  do {
    HunkT *next = hunk->next;
    MemFree(hunk);
    hunk = next;
  } while (hunk);
}
