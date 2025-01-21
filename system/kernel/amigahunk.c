#include <debug.h>
#include <crc32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <lzsa.h>
#include <zx0.h>
#include <system/amigahunk.h>
#include <system/boot.h>
#include <system/file.h>
#include <system/memory.h>

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

#define COMP_NONE (0 * __BIT(28))
#define COMP_LZSA (1 * __BIT(28))
#define COMP_ZX0 (3 * __BIT(28))
#define COMP_MASK (__BIT(29) | __BIT(28))

#define FLAG_MASK (HUNKF_CHIP | HUNKF_FAST)
#define SIZE_MASK (~(COMP_MASK | FLAG_MASK))

static struct {
  int count;
  HunkT **array;
} SharedHunks = { 0, NULL };

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
    int n, comp;

    if (hunkId == HUNK_CODE || hunkId == HUNK_DATA || hunkId == HUNK_BSS) {
      hunkRoot = true;
      comp = n = ReadLong(fh);
      n &= SIZE_MASK;
      comp &= COMP_MASK;
      if (hunkId != HUNK_BSS) {
        int size = n * sizeof(int);

        /* Compression type is added by packexe tool and notifies that the hunk
         * is compressed with LZSA or ZX0 algorithm. */
        if (comp != COMP_NONE) {
          int offset = hunk->size - size;
          FileRead(fh, hunk->data + offset, size);
          /* in-place decompression, watch out for delta */
          if (comp == COMP_ZX0) {
            zx0_decompress(hunk->data + offset, hunk->data);
          } else if (comp == COMP_LZSA) {
            lzsa_depack_stream(hunk->data + offset, hunk->data);
          }
        } else {
          FileRead(fh, hunk->data, size);
        }
      }
      {
        __unused const char *hunkType;

        if (hunkId == HUNK_CODE)
          hunkType = "CODE";
        else if (hunkId == HUNK_DATA)
          hunkType = "DATA";
        else
          hunkType = " BSS";

        Log("[Hunk] %s: $%x (size: %d)\n", hunkType, (int)hunk->data, hunk->size);
        Debug("%s: crc32: $%08x", hunkType, crc32(hunk->data, hunk->size));
      }
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
      Log("Unknown hunk $%04x!\n", hunkId);
      return false;
    }
  }

  return true;
}

HunkT *LoadHunkList(FileT *fh) {
  short hunkId = ReadLong(fh);
  int n, first, last, hunkCount;
  HunkT **hunkArray;

  if (hunkId != HUNK_HEADER) {
    Log("Not an Amiga Hunk file!\n");
    return NULL;
  }

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
  hunkArray = alloca(sizeof(HunkT *) * (hunkCount + SharedHunks.count));

  if (SharedHunks.count)
    memcpy(&hunkArray[hunkCount],
           SharedHunks.array, sizeof(HunkT *) * SharedHunks.count);

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

void SetupSharedHunks(HunkT *hunk) {
  HunkT **array;
  HunkT *h;
  int n, i;

  for (h = hunk, n = 0; h != NULL; h = h->next, n++);

  array = MemAlloc(sizeof(HunkT *) + n * sizeof(HunkT *), MEMF_PUBLIC);
  for (h = hunk, i = 0; h != NULL; h = h->next, i++)
    array[i] = h;

  if (SharedHunks.array)
    MemFree(SharedHunks.array);
  SharedHunks.count = n;
  SharedHunks.array = array;
}
