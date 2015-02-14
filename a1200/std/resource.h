#ifndef __STD_RESOURCE_H__
#define __STD_RESOURCE_H__

#include "std/types.h"

void StartResourceManager();
void StopResourceManager();

void ResAdd(const char *name, PtrT ptr);
void ResAddStatic(const char *name, PtrT ptr);
PtrT ResGet(const char *name);

#define ResAddTable(NAME, TYPE, SIZE) \
  ResAdd(NAME, NewTable(TYPE, SIZE))
#define R_(NAME) ResGet(NAME)

#endif
