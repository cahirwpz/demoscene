#ifndef __STD_RESOURCE_H__
#define __STD_RESOURCE_H__

#include "std/types.h"

void StartResourceManager();
void StopResourceManager();

void AddRscSimple(const StrT name, PtrT ptr, FreeFuncT freeFunc);
void AddRscStatic(const StrT name, PtrT ptr);
PtrT GetResource(const StrT name);

#define RSC_STATIC(NAME, PTR) \
  AddRscStatic(NAME, PTR)
#define RSC_ARRAY(NAME, TYPE, SIZE) \
  AddRscSimple(NAME, NEW_A(TYPE, SIZE), (FreeFuncT)MemFree)
#define R_(NAME) GetResource(NAME)

#endif
