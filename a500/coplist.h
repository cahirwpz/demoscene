#ifndef __COPLIST_H__
#define __COPLIST_H__

#include <exec/types.h>

typedef struct CopList {
  UWORD length;
  UWORD flags;
  UWORD *curr;
  UWORD *last;
  UWORD entry[0]; 
} CopListT;

__regargs CopListT *NewCopList(UWORD length);
__regargs void DeleteCopList(CopListT *copList);
__regargs void CopListActivate(CopListT *copList);
__regargs void CopInit(CopListT *copList);
__regargs void CopWait(CopListT *copList, UWORD vp, UWORD hp);
__regargs void CopMove16(CopListT *copList, UWORD reg, UWORD data);
__regargs void CopMove32(CopListT *copList, UWORD reg, ULONG data);
__regargs void CopEnd(CopListT *copList);

#define CSREG(reg) offsetof(struct Custom, reg)

#endif
