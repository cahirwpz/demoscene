#ifndef __SETJMP_H__
#define __SETJMP_H__

#include <cdefs.h>
#include <types.h>

#define _JB_D2 0
#define _JB_D3 1
#define _JB_D4 2
#define _JB_D5 3
#define _JB_D6 4
#define _JB_D7 5
#define _JB_A2 6
#define _JB_A3 7
#define _JB_A4 8
#define _JB_A5 9
#define _JB_A6 10
#define _JB_SP 11
#define _JB_PC 12

#define _JBLEN 13

typedef u_int jmp_buf[_JBLEN];

__returns_twice int setjmp(jmp_buf asm("a0"));
__noreturn void longjmp(jmp_buf asm("a0"), int asm("d1"));

#endif /* !__SETJMP_H__ */
