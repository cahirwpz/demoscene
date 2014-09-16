#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <exec/types.h>

typedef struct InterruptVector {
  APTR InitialSSP;             /*  0 */
  APTR InitialPC;              /*  1 */
  APTR BusError;               /*  2 */
  APTR AddressError;           /*  3 */
  APTR IllegalInstruction;     /*  4 */
  APTR DivideByZero;           /*  5 */
  APTR CheckInstruction;       /*  6 */
  APTR TrapInstruction;        /*  7 */
  APTR PrivilegeViolation;     /*  8 */
  APTR Trace;                  /*  9 */
  APTR UnimplementedOpcodeA;   /* 10 */
  APTR UnimplementedOpcodeF;   /* 11 */
  APTR reserved1[3];           /* 12 - 14 */
  APTR UninitializedInterrupt; /* 15 */
  APTR reserved2[8];           /* 16 - 23 */
  APTR SpuriousInterrupt;      /* 24 */
  APTR IntLevel1;              /* 25 */
  APTR IntLevel2;              /* 26 */
  APTR IntLevel3;              /* 27 */
  APTR IntLevel4;              /* 28 */
  APTR IntLevel5;              /* 29 */
  APTR IntLevel6;              /* 30 */
  APTR IntLevel7;              /* 31 */
  APTR Trap[16];               /* 32 - 47 */
  APTR reserved3[16];          /* 48 - 63 */
  APTR UserDefined[192];       /* 64 - 255 */
} InterruptVectorT;

extern InterruptVectorT *InterruptVector;

void SaveInterrupts();
void RestoreInterrupts();

InterruptVectorT *GetVBR();
void StopCPU();

#endif
