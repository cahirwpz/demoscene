#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <system/amigahunk.h>
#include <system/file.h>

#define DEBUG 0

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

static long ReadLong(FileT *fh) {
  long v = 0;
  FileRead(fh, &v, sizeof(v));
  return v;
}

static void SkipLongs(FileT *fh, int n) {
  FileSeek(fh, n * sizeof(int), SEEK_CUR);
}

static bool AllocHunks(FileT *fh, HunkT **hunkArray, short hunkCount) {
  HunkT *prev = NULL;

  do {
    /* size specifiers including memory attribute flags */
    uint32_t n = ReadLong(fh);

    HunkT *hunk = MemAlloc(sizeof(HunkT) + n * sizeof(int),
                           (n & HUNKF_CHIP) ? MEMF_CHIP : MEMF_PUBLIC);
    *hunkArray++ = hunk;

    if (!hunk)
      return false;

    hunk->size = n * sizeof(int);
    hunk->next = NULL;
    bzero(hunk->data, n * sizeof(int));

    if (prev)
      prev->next = hunk;

    prev = hunk;
  } while (--hunkCount);

  return true;
}

static bool LoadHunks(FileT *fh, HunkT **hunkArray) {
  int hunkIndex = 0;
  HunkT *hunk = hunkArray[hunkIndex++];
  short hunkId;
  bool hunkRoot = true;

  while ((hunkId = ReadLong(fh))) {
    int n;

    if (hunkId == HUNK_CODE || hunkId == HUNK_DATA || hunkId == HUNK_BSS) {
      hunkRoot = true;
      n = ReadLong(fh);
      if (hunkId != HUNK_BSS)
        FileRead(fh, hunk->data, n * sizeof(int));
#if DEBUG
      {
        const char *hunkType;

        if (hunkId == HUNK_CODE)
          hunkType = "CODE";
        else if (hunkId == HUNK_DATA)
          hunkType = "DATA";
        else
          hunkType = " BSS";

        printf("%s: %p - %p\n", hunkType, hunk->data, hunk->data + hunk->size);
      }
#endif
    } else if (hunkId == HUNK_DEBUG) {
      n = ReadLong(fh);
      SkipLongs(fh, n);
    } else if (hunkId == HUNK_RELOC32) {
      while ((n = ReadLong(fh))) {
        int hunkNum = ReadLong(fh);
        int32_t hunkRef = (int32_t)hunkArray[hunkNum]->data;
        do {
          int hunkOff = ReadLong(fh);
          int32_t *ptr = (void *)hunk->data + hunkOff;
          *ptr += hunkRef;
        } while (--n);
      }
    } else if (hunkId == HUNK_SYMBOL) {
      while ((n = ReadLong(fh)))
        SkipLongs(fh, n + 1);
    } else if (hunkId == HUNK_END) {
      if (hunkRoot) {
        hunkRoot = false;
        hunk = hunkArray[hunkIndex++];
      }
    } else {
#if DEBUG
      printf("Unknown hunk $%04x!\n", hunkCode);
#endif
      return false;
    }
  }

  return true;
}

HunkT *LoadHunkList(FileT *fh) {
  short hunkId = ReadLong(fh);
  int n, first, last, hunkCount;
  HunkT **hunkArray;

  if (hunkId != HUNK_HEADER)
    return NULL;

  /* Skip resident library names. */
  while ((n = ReadLong(fh)))
    SkipLongs(fh, n);

  /*
   * number of hunks (including resident libraries and overlay hunks)
   * number of the first (root) hunk
   * number of the last (root) hunk
   */
  SkipLongs(fh, 1);
  first = ReadLong(fh);
  last = ReadLong(fh);

  hunkCount = last - first + 1;
  hunkArray = alloca(sizeof(HunkT *) * hunkCount);

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
