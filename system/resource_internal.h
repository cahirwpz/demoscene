#ifndef __RESOURCE_INTERNAL_H__
#define __RESOURCE_INTERNAL_H__

#include "resource.h"

#define _T(X) X ## _ptr

#define RSC_TYPE(NAME, TYPE) \
    typedef TYPE _T(NAME);

#define RSC_ADD(NAME, ALLOC_FUNC) \
    static _T(NAME) NAME ## _alloc(void) { return ALLOC_FUNC; }

#define RSC_FREE(NAME, FREE_FUNC) \
    static void NAME ## _free(_T(NAME) NAME) { FREE_FUNC(NAME); }

#define RSC_INIT(NAME) \
  static BOOL NAME ## _init(_T(NAME) NAME)

#define RSC(NAME) \
  {#NAME, \
   NULL, \
   (AllocFuncType)NAME ## _alloc, \
   (FreeFuncType)NAME ## _free, \
   (InitFuncType)NAME ## _init},

#define RSC_STD(NAME) \
  {#NAME, NULL, (AllocFuncType)NAME ## _alloc, NULL, NULL},

#define RSC_START \
  struct Resource ResourceList[] = {

#define RSC_END \
  {NULL, NULL, NULL, NULL, NULL}};

#endif
