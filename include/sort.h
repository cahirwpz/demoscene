#ifndef __SORT_H__
#define __SORT_H__

#include "common.h"

typedef struct {
  short key, index;
} SortItemT;

void SortItemArray(SortItemT *table, short size);

#endif
