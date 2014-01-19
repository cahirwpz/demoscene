#ifndef __SYSTEM_RWOPS_H__
#define __SYSTEM_RWOPS_H__

/* Heavily inspired by SDL RWOps. */

typedef struct RwOps RwOpsT;

typedef enum { IO_SEEK_SET, IO_SEEK_CUR, IO_SEEK_END } SeekModeT;

#define IO_UNKNOWN   0   /* Unknown stream type */
#define IO_FILE      1   /* File stream */
#define IO_MEMORY    2   /* Memory stream */
#define IO_MEMORY_RO 3   /* Read-Only memory stream */

struct RwOps {
  int (*read)(RwOpsT *stream asm("a0"), void *buffer asm("d2"), unsigned int size asm("d3"));
  int (*write)(RwOpsT *stream asm("a0"), const void *buffer asm("d2"), unsigned int size asm("d3"));
  int (*seek)(RwOpsT *stream asm("a0"), int offset asm("d2"), SeekModeT whence asm("d3"));
  int (*tell)(RwOpsT *stream asm("a0"));
  int (*close)(RwOpsT *stream asm("a0"));

  int type;

  union {
    struct {
      int fd;
    } file;

    struct {
      uint8_t *base;
      uint8_t *here;
      uint8_t *stop;
    } memory;
  } u;
};

RwOpsT *RwOpsFromFile(const char *file, const char *mode);
RwOpsT *RwOpsFromMemory(void *ptr, int size);
RwOpsT *RwOpsFromConstMemory(const void *ptr, int size);

static inline int IoRead(RwOpsT *stream, void *buffer, unsigned int size) {
  return stream->read(stream, buffer, size);
}

static inline int IoWrite(RwOpsT *stream, const void *buffer, unsigned int size) {
  return stream->write(stream, buffer, size);
}

static inline int IoSeek(RwOpsT *stream, int offset, int whence) {
  return stream->seek(stream, offset, whence);
}

static inline int IoTell(RwOpsT *stream) {
  return stream->tell(stream);
}

static inline int IoClose(RwOpsT *stream) {
  return stream->close(stream);
}

__regargs int IoSize(RwOpsT *stream);

__regargs bool IoRead8(RwOpsT *stream, uint8_t *data);
__regargs bool IoRead16(RwOpsT *stream, uint16_t *data);
__regargs bool IoRead32(RwOpsT *stream, uint32_t *data);
__regargs bool IoReadLE16(RwOpsT *stream, uint16_t *data);
__regargs bool IoReadLE32(RwOpsT *stream, uint32_t *data);

#endif
