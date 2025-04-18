#ifndef __SYSTEM_MEMFILE_H__
#define __SYSTEM_MEMFILE_H__

#include <types.h>

struct File;

typedef struct MemBlock {
  const void *addr;
  u_int length; /* 0 is a list terminator */
} MemBlockT;

/* @brief Use multiple block of memory to back a file-like object.
 * @note Blocks should be non-overlapping, last element is terminator.
 */
struct File *MemOpen(const MemBlockT *blocks);

#endif /* !__SYSTEM_MEMFILE_H__ */
