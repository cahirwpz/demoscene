#include "engine/object.h"
#include "std/resource_internal.h"
#include "system/memory.h"

RSC_TYPE(object, ObjectT *)
RSC_ADD(object, NewObjectFromFile("data/whelpz.robj", MEMF_PUBLIC))
RSC_FREE(object, DeleteObject)
#define object_init NULL

RSC_START
RSC(object)
RSC_END
