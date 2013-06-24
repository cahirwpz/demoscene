#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <stdint.h>

inline uint16_t endianSwap16L(uint16_t x) {
  return x;
}

inline uint16_t endianSwap16B(uint16_t x) {
  return ((x >> 8) & 0x00ff) | ((x << 8) & 0xff00);
}

inline uint32_t endianSwap32L(uint32_t x) {
  return x;
}

inline uint32_t endianSwap32B(uint32_t x) {
  return (endianSwap16B(x & 0x000ffff) << 16) | (endianSwap16B(x >> 16));
}

inline uint8_t endianReadU8Little(const uint8_t* pos) {
  return *pos;
}

inline uint8_t endianReadU8Big(const uint8_t* pos) {
  return *pos;
}

inline uint16_t endianReadU16Little(const uint16_t* pos) {
  return endianSwap16L(*pos);
}

inline uint16_t endianReadU16Big(const uint16_t* pos) {
  return endianSwap16B(*pos);
}

inline uint32_t endianReadU32Little(const uint32_t* pos) {
  return endianSwap32L(*pos);
}

inline uint32_t endianReadU32Big(const uint32_t* pos) {
  return endianSwap32B(*pos);
}

inline void endianWriteU8Little(uint8_t* pos, uint8_t value) {
  *pos = value;
}

inline void endianWriteU8Big(uint8_t* pos, uint8_t value) {
  *pos = value;
}

inline void endianWriteU16Little(uint16_t* pos, uint16_t value) {
  *pos = endianSwap16L(value);
}

inline void endianWriteU16Big(uint16_t* pos, uint16_t value) {
  *pos = endianSwap16B(value);
}

inline void endianWriteU32Little(uint32_t* pos, uint32_t value) {
  *pos = endianSwap32L(value);
}

inline void endianWriteU32Big(uint32_t* pos, uint32_t value) {
  *pos = endianSwap32B(value);
}

#endif
