#include <proto/exec.h>
#include <dos/doshunks.h>

#include "amigahunk.h"
#include "io.h"

#define ONSTACK(x) (&(x)), sizeof((x))

static __regargs BOOL AllocHunks(FileT *fh, LONG hunkNum, HunkT **hunks) {
  LONG memFlags, n, size;
  HunkT *hunk, *prev = NULL;

  while (hunkNum--) {
    /* size specifiers including memory attribute flags */
    FileRead(fh, ONSTACK(n));

    memFlags = n >> 30;
    size = n & 0x3FFFFFFF;
    if (memFlags == 1)
      memFlags = MEMF_CHIP;
    else if (memFlags == 2)
      memFlags = MEMF_FAST;
    else
      memFlags = MEMF_PUBLIC;

    if (!(hunk = AllocMem(sizeof(HunkT) + n * sizeof(LONG), memFlags)))
      return FALSE;

    hunk->size = n * sizeof(LONG);
    hunk->next = NULL;

    if (prev)
      prev->next = MKBADDR(&hunk->next);
    prev = hunk;

    *hunks++ = hunk;
  }

  return TRUE;
}

static __regargs BOOL LoadHunks(FileT *fh, HunkT **hunks) {
  LONG n, hunkCode, hunkNum;
  WORD hunkIdx = 0;
  BOOL hunkRoot = TRUE;

  while (FileRead(fh, &hunkCode, sizeof(LONG))) {
    WORD hunkId = hunkCode; /* hunkId = 10xx, so we can ignore upper bits */
    HunkT *hunk = hunks[hunkIdx];

    if (hunkId == HUNK_CODE || hunkId == HUNK_DATA || hunkId == HUNK_BSS) {
      hunkRoot = TRUE;
      FileRead(fh, ONSTACK(n));
      if (hunkId != HUNK_BSS)
        FileRead(fh, hunk->data, n * sizeof(LONG));
      else
        memset(hunk->data, 0, n * sizeof(LONG));
      {
        char *hunkType;

        if (hunkId == HUNK_CODE)
          hunkType = "CODE";
        else if (hunkId == HUNK_DATA)
          hunkType = "DATA";
        else
          hunkType = " BSS";

        Log("%s: %lx - %lx\n", hunkType, 
            (LONG)hunk->data, (LONG)(hunk->data + hunk->size));
      }
    } else if (hunkId == HUNK_DEBUG) {
      FileRead(fh, ONSTACK(n));
      FileSeek(fh, n * sizeof(LONG), SEEK_CUR);
    } else if (hunkId == HUNK_RELOC32) {
      LONG hunkRef, reloc;
      do {
        FileRead(fh, ONSTACK(n));
        if (n == 0) break;
        FileRead(fh, ONSTACK(hunkNum));
        hunkRef = (LONG)hunks[hunkNum]->data;
        while (n--) {
          FileRead(fh, ONSTACK(reloc));
          ((LONG *)hunk->data)[reloc] += hunkRef;
        }
      } while(1);
    } else if (hunkId == HUNK_SYMBOL) {
      do {
        FileRead(fh, ONSTACK(n));
        if (n == 0) break;
        FileSeek(fh, (n + 1) * sizeof(LONG), SEEK_CUR);
      } while(1);
    } else if (hunkId == HUNK_END) {
      if (hunkRoot) {
        hunkRoot = FALSE;
        hunkIdx++;
      }
    } else {
      Log("Unknown hunk $%08lx!\n", hunkCode);
      return FALSE;
    }
  }

  return TRUE;
}

__regargs BPTR LoadExecutable(FileT *fh) {
  HunkT **hunks;
  LONG n, hunkId, hunkNum;
 
  FileRead(fh, ONSTACK(hunkId));
  hunkId &= 0x3FFFFFFF;

  if (hunkId == HUNK_HEADER) {
    /* Skip resident library names. */
    do {
      FileRead(fh, ONSTACK(n));
      if (n == 0) break;
      FileSeek(fh, n * sizeof(LONG), SEEK_CUR);
    } while(1);

    /*
     * number of hunks (including resident libraries and overlay hunks)
     * number of the first (root) hunk
     * number of the last (root) hunk
     */
    {
      struct { LONG hunks, first, last; } s;
      FileRead(fh, ONSTACK(s));
      hunkNum = s.last - s.first + 1;
    }

    hunks = __builtin_alloca(sizeof(HunkT *) * hunkNum);

    if (AllocHunks(fh, hunkNum, hunks)) {
      BPTR seglist = MKBADDR(&(hunks[0]->next));
      if (LoadHunks(fh, hunks))
        return seglist;
      FreeSegList(seglist);
    }
  }

  return NULL;
}

__regargs void FreeSegList(BPTR seglist) {
  HunkT *hunk = seglist ? BADDR(seglist - 1) : NULL;

  while (hunk) {
    HunkT *next = hunk->next ? BADDR(hunk->next - 1) : NULL;
    FreeMem(hunk, hunk->size + sizeof(HunkT));
    hunk = next;
  }
}
