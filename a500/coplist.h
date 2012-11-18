#ifndef __COPLIST_H__
#define __COPLIST_H__

#include <exec/types.h>

typedef union CopIns {
  struct {
    WORD vpos;
    WORD hpos;
  } wait;
  struct {
    WORD reg;
    WORD data;
  } move;
} CopInsT;

typedef struct CopList {
  UWORD length;
  UWORD flags;

  CopInsT *curr;
  CopInsT entry[0]; 
} CopListT;

__regargs CopListT *NewCopList(UWORD length);
__regargs void DeleteCopList(CopListT *copList);
__regargs void CopListActivate(CopListT *copList);
__regargs void CopInit(CopListT *copList);
__regargs CopInsT *CopWait(CopListT *copList, UWORD vp, UWORD hp);
__regargs CopInsT *CopMove16(CopListT *copList, UWORD reg, UWORD data);
__regargs CopInsT *CopMove32(CopListT *copList, UWORD reg, ULONG data);
__regargs void CopEnd(CopListT *copList);

#define CSREG(reg) offsetof(struct Custom, reg)

#endif
