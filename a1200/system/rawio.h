#ifndef __SYSTEM_RAWIO_H__
#define __SYSTEM_RAWIO_H__

void KPutChar(char c asm("d0"));
void KPutByte(int n asm("d0"));
void KPutWord(int n asm("d0"));
void KPutLong(int n asm("d0"));
void KPutStr(char *str asm("d0"));

void DPutChar(char c asm("d0"));
void DPutByte(int n asm("d0"));
void DPutWord(int n asm("d0"));
void DPutLong(int n asm("d0"));
void DPutStr(char *str asm("d0"));

#endif
