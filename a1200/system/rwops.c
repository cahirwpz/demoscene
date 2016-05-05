#include "std/debug.h"
#include "std/memory.h"
#include "system/rwops.h"

__regargs bool IoRead8(RwOpsT *stream, uint8_t *data) {
  return (IoRead(stream, data, sizeof(uint8_t)) == sizeof(uint8_t));
}

__regargs bool IoRead16(RwOpsT *stream, uint16_t *data) {
  return (IoRead(stream, data, sizeof(uint16_t)) == sizeof(uint16_t));
}

__regargs bool IoRead32(RwOpsT *stream, uint32_t *data) {
  return (IoRead(stream, data, sizeof(uint32_t)) == sizeof(uint32_t));
}

__regargs bool IoReadLE16(RwOpsT *stream, uint16_t *data) {
  uint16_t res;

  if (IoRead(stream, &res, sizeof(uint16_t)) != sizeof(uint16_t))
    return false;

  asm("ror.w #8,%0;" : "+d" (res));
  *data = res;
  return true;
}

__regargs bool IoReadLE32(RwOpsT *stream, uint32_t *data) {
  uint32_t res;

  if (IoRead(stream, &res, sizeof(uint32_t)) != sizeof(uint32_t))
    return false;

  asm("ror.w #8,%0;"
      "swap %0;"
      "ror.w #8,%0;"
      : "+d" (res));
  *data = res;
  return true;
}
