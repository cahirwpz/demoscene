#ifndef __SORT_H__
#define __SORT_H__

#include "common.h"

typedef struct {
  WORD key, index;
} SortItemT;

__regargs void SortItemArray(SortItemT *table, WORD size);

#endif
