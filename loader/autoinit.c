#include "types.h"
 
/* Introduce weak symbols in case no constructors or descrutors were defined. */
u_int __INIT_LIST__;
u_int __EXIT_LIST__;

typedef struct FuncItem {
  void (*func)(void);
  u_int pri;
} FuncItemT;

typedef struct FuncItemSet {
  u_int count; /* actually number of consecutive long words. */
  FuncItemT item[0];
} FuncItemSetT;

/* Call functions in ascending order of priority. Destructor priorities are
 * reversed by ADD2EXIT so there's no need to handle extra case. */
void CallFuncList(FuncItemSetT *set) {
  short n = set->count / 2;
  FuncItemT *start = (FuncItemT *)set->item;
  u_char cur_pri = 0;

  while (n > 0) {
    FuncItemT *item;
    u_char next_pri = 255;
    for (item = start; item->func; item++) {
      u_char pri = item->pri;
      if (pri < cur_pri)
        continue;
      if (pri == cur_pri) {
        item->func();
        n--;
      } else if (pri < next_pri) {
        next_pri = pri;
      }
    }
    cur_pri = next_pri;
  }
}
