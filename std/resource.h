#ifndef __STD_RESOURCE_H__
#define __STD_RESOURCE_H__

#include "std/types.h"

void StartResourceManager();
void StopResourceManager();

void ResAdd(const StrT name, PtrT ptr);
void ResAddStatic(const StrT name, PtrT ptr);
PtrT ResGet(const StrT name);

#define ResAddTable(NAME, TYPE, SIZE) \
  ResAdd(NAME, NewTable(TYPE, SIZE))
#define R_(NAME) ResGet(NAME)

#endif
