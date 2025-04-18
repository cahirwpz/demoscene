#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

#include <types.h>

u_int Checksum(u_int cksum asm("d0"), u_int *data asm("a0"), ssize_t size asm("d1"));

#endif /* !__CHECKSUM_H__ */
