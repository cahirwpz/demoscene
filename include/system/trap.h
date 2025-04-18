#ifndef __SYSTEM_TRAP_H__
#define __SYSTEM_TRAP_H__

#define T_UNKNOWN   0
#define T_BUSERR    1
#define T_ADDRERR   2
#define T_ILLINST   3
#define T_ZERODIV   4
#define T_CHKINST   5
#define T_TRAPVINST 6
#define T_PRIVINST  7
#define T_TRACE     8
#define T_FMTERR    9
#define T_TRAPINST  10
#define T_NTRAPS    11

#ifndef __ASSEMBLER__
#include <types.h>

/* Special Status Word: M68010 memory fault information */
#define SSW_RR 0x8000
#define SSW_IF 0x2000
#define SSW_DF 0x1000
#define SSW_RM 0x0800
#define SSW_HB 0x0400
#define SSW_BY 0x0200
#define SSW_RW 0x0100
#define SSW_FC 0x0007

typedef struct __packed {
  u_short sr;
  u_int pc;
} Trap68000;

typedef struct __packed {
  u_short status;
  u_int address;
  u_short instreg;
  u_short sr;
  u_int pc;
} MemAccTrap68000;

typedef struct __packed {
  u_short sr;
  u_int pc;
  u_short format;
} Trap68010;

typedef struct __packed {
  u_short sr;
  u_int pc;
  u_short format;
  u_short ssw;
  u_int address;
  u_short pad[22];
} MemAccTrap68010;

typedef struct __packed TrapFrame {
  u_int usp;
  u_int d0, d1, d2, d3, d4, d5, d6, d7;
  u_int a0, a1, a2, a3, a4, a5, a6, sp;
  u_short trapnum;
  union {
    Trap68000 _m68000;
    MemAccTrap68000 _m68000_memacc;
    Trap68010 _m68010;
    MemAccTrap68010 _m68010_memacc;
  } u;
} TrapFrameT;

#define m68000 u._m68000
#define m68000_memacc u._m68000_memacc
#define m68010 u._m68010
#define m68010_memacc u._m68010_memacc

void BadTrap(void);
void BusErrTrap(void);
void AddrErrTrap(void);
void IllegalTrap(void);
void ZeroDivTrap(void);
void ChkInstTrap(void);
void TrapvInstTrap(void);
void PrivInstTrap(void);
void TraceTrap(void);
void FmtErrTrap(void);
void TrapInstTrap(void);

void YieldHandler(void);
void CrashHandler(void);
#endif

#endif /* !__SYSTEM_TRAP_H__ */
