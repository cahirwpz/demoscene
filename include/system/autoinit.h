#ifndef __SYSTEM_AUTOINIT_H__
#define __SYSTEM_AUTOINIT_H__

#include <types.h>

typedef struct FuncItem {
  void (*func)(void);
  u_int pri;
} FuncItemT;

typedef struct FuncItemSet {
  u_int count; /* actually number of consecutive long words. */
  FuncItemT item[0];
} FuncItemSetT;

extern FuncItemSetT __INIT_LIST__;
extern FuncItemSetT __EXIT_LIST__;

/* Call functions in ascending order of priority. Destructor priorities are
 * reversed by ADD2EXIT so there's no need to handle extra case. */
void CallFuncList(FuncItemSetT *set);

#endif /* !__SYSTEM_AUTOINIT_H__ */
