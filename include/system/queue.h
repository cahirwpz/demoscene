/* This code is covered by BSD license.
 * It was taken from OpenBSD sys/sys/queue.h and slightly modified. */

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <cdefs.h>

/*
 * This file defines five types of data structures: singly-linked lists,
 * lists, simple queues and tail queues.
 *
 *
 * A singly-linked list is headed by a single forward pointer. The elements
 * are singly linked for minimum space and pointer manipulation overhead at
 * the expense of O(n) removal for arbitrary elements. New elements can be
 * added to the list after an existing element or at the head of the list.
 * Elements being removed from the head of the list should use the explicit
 * macro for this purpose for optimum efficiency. A singly-linked list may
 * only be traversed in the forward direction.  Singly-linked lists are ideal
 * for applications with large datasets and few or no removals or for
 * implementing a LIFO queue.
 *
 * A list is headed by a single forward pointer (or an array of forward
 * pointers for a hash table header). The elements are doubly linked
 * so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before
 * or after an existing element or at the head of the list. A list
 * may only be traversed in the forward direction.
 *
 * A simple queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are singly
 * linked to save space, so elements can only be removed from the
 * head of the list. New elements can be added to the list before or after
 * an existing element, at the head of the list, or at the end of the
 * list. A simple queue may only be traversed in the forward direction.
 *
 * A tail queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or
 * after an existing element, at the head of the list, or at the end of
 * the list. A tail queue may be traversed in either direction.
 *
 * For details on the use of these macros, see the queue(3) manual page.
 */

/*
 * Singly-linked List definitions.
 */
#define SLIST_HEAD(name, type)                                                 \
  struct name {                                                                \
    struct type *slh_first; /* first element */                                \
  }

#define SLIST_HEAD_INITIALIZER(head)                                           \
  { NULL }

#define SLIST_ENTRY(type)                                                      \
  struct {                                                                     \
    struct type *sle_next; /* next element */                                  \
  }

/*
 * Singly-linked List access methods.
 */
#define SLIST_FIRST(head) ((head)->slh_first)
#define SLIST_END(head) NULL
#define SLIST_EMPTY(head) (SLIST_FIRST(head) == SLIST_END(head))
#define SLIST_NEXT(elm, field) ((elm)->field.sle_next)

#define SLIST_FOREACH(var, head, field)                                        \
  for ((var) = SLIST_FIRST(head); (var) != SLIST_END(head);                    \
       (var) = SLIST_NEXT(var, field))

#define SLIST_FOREACH_SAFE(var, head, field, tvar)                             \
  for ((var) = SLIST_FIRST(head);                                              \
       (var) && ((tvar) = SLIST_NEXT(var, field), 1); (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#define SLIST_INIT(head)                                                       \
  { SLIST_FIRST(head) = SLIST_END(head); }

#define SLIST_INSERT_AFTER(slistelm, elm, field)                               \
  do {                                                                         \
    (elm)->field.sle_next = (slistelm)->field.sle_next;                        \
    (slistelm)->field.sle_next = (elm);                                        \
  } while (0)

#define SLIST_INSERT_HEAD(head, elm, field)                                    \
  do {                                                                         \
    (elm)->field.sle_next = (head)->slh_first;                                 \
    (head)->slh_first = (elm);                                                 \
  } while (0)

#define SLIST_REMOVE_AFTER(elm, field)                                         \
  do {                                                                         \
    (elm)->field.sle_next = (elm)->field.sle_next->field.sle_next;             \
  } while (0)

#define SLIST_REMOVE_HEAD(head, field)                                         \
  do {                                                                         \
    (head)->slh_first = (head)->slh_first->field.sle_next;                     \
  } while (0)

#define SLIST_REMOVE(head, elm, type, field)                                   \
  do {                                                                         \
    if ((head)->slh_first == (elm)) {                                          \
      SLIST_REMOVE_HEAD((head), field);                                        \
    } else {                                                                   \
      struct type *curelm = (head)->slh_first;                                 \
                                                                               \
      while (curelm->field.sle_next != (elm))                                  \
        curelm = curelm->field.sle_next;                                       \
      curelm->field.sle_next = curelm->field.sle_next->field.sle_next;         \
    }                                                                          \
  } while (0)

/*
 * List definitions.
 */
#define LIST_HEAD(name, type)                                                  \
  struct name {                                                                \
    struct type *lh_first; /* first element */                                 \
  }

#define LIST_HEAD_INITIALIZER(head)                                            \
  { NULL }

#define LIST_ENTRY(type)                                                       \
  struct {                                                                     \
    struct type *le_next;  /* next element */                                  \
    struct type **le_prev; /* address of previous next element */              \
  }

/*
 * List access methods.
 */
#define LIST_FIRST(head) ((head)->lh_first)
#define LIST_END(head) NULL
#define LIST_EMPTY(head) (LIST_FIRST(head) == LIST_END(head))
#define LIST_NEXT(elm, field) ((elm)->field.le_next)

#define LIST_FOREACH(var, head, field)                                         \
  for ((var) = LIST_FIRST(head); (var) != LIST_END(head);                      \
       (var) = LIST_NEXT(var, field))

#define LIST_FOREACH_SAFE(var, head, field, tvar)                              \
  for ((var) = LIST_FIRST(head); (var) && ((tvar) = LIST_NEXT(var, field), 1); \
       (var) = (tvar))

/*
 * List functions.
 */
#define LIST_INIT(head)                                                        \
  do {                                                                         \
    LIST_FIRST(head) = LIST_END(head);                                         \
  } while (0)

#define LIST_INSERT_AFTER(listelm, elm, field)                                 \
  do {                                                                         \
    if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)             \
      (listelm)->field.le_next->field.le_prev = &(elm)->field.le_next;         \
    (listelm)->field.le_next = (elm);                                          \
    (elm)->field.le_prev = &(listelm)->field.le_next;                          \
  } while (0)

#define LIST_INSERT_BEFORE(listelm, elm, field)                                \
  do {                                                                         \
    (elm)->field.le_prev = (listelm)->field.le_prev;                           \
    (elm)->field.le_next = (listelm);                                          \
    *(listelm)->field.le_prev = (elm);                                         \
    (listelm)->field.le_prev = &(elm)->field.le_next;                          \
  } while (0)

#define LIST_INSERT_HEAD(head, elm, field)                                     \
  do {                                                                         \
    if (((elm)->field.le_next = (head)->lh_first) != NULL)                     \
      (head)->lh_first->field.le_prev = &(elm)->field.le_next;                 \
    (head)->lh_first = (elm);                                                  \
    (elm)->field.le_prev = &(head)->lh_first;                                  \
  } while (0)

#define LIST_REMOVE(elm, field)                                                \
  do {                                                                         \
    if ((elm)->field.le_next != NULL)                                          \
      (elm)->field.le_next->field.le_prev = (elm)->field.le_prev;              \
    *(elm)->field.le_prev = (elm)->field.le_next;                              \
  } while (0)

#define LIST_REPLACE(elm, elm2, field)                                         \
  do {                                                                         \
    if (((elm2)->field.le_next = (elm)->field.le_next) != NULL)                \
      (elm2)->field.le_next->field.le_prev = &(elm2)->field.le_next;           \
    (elm2)->field.le_prev = (elm)->field.le_prev;                              \
    *(elm2)->field.le_prev = (elm2);                                           \
  } while (0)

/*
 * Simple queue definitions.
 */
#define SIMPLEQ_HEAD(name, type)                                               \
  struct name {                                                                \
    struct type *sqh_first; /* first element */                                \
    struct type **sqh_last; /* addr of last next element */                    \
  }

#define SIMPLEQ_HEAD_INITIALIZER(head)                                         \
  { NULL, &(head).sqh_first }

#define SIMPLEQ_ENTRY(type)                                                    \
  struct {                                                                     \
    struct type *sqe_next; /* next element */                                  \
  }

/*
 * Simple queue access methods.
 */
#define SIMPLEQ_FIRST(head) ((head)->sqh_first)
#define SIMPLEQ_END(head) NULL
#define SIMPLEQ_EMPTY(head) (SIMPLEQ_FIRST(head) == SIMPLEQ_END(head))
#define SIMPLEQ_NEXT(elm, field) ((elm)->field.sqe_next)

#define SIMPLEQ_FOREACH(var, head, field)                                      \
  for ((var) = SIMPLEQ_FIRST(head); (var) != SIMPLEQ_END(head);                \
       (var) = SIMPLEQ_NEXT(var, field))

#define SIMPLEQ_FOREACH_SAFE(var, head, field, tvar)                           \
  for ((var) = SIMPLEQ_FIRST(head);                                            \
       (var) && ((tvar) = SIMPLEQ_NEXT(var, field), 1); (var) = (tvar))

/*
 * Simple queue functions.
 */
#define SIMPLEQ_INIT(head)                                                     \
  do {                                                                         \
    (head)->sqh_first = NULL;                                                  \
    (head)->sqh_last = &(head)->sqh_first;                                     \
  } while (0)

#define SIMPLEQ_INSERT_HEAD(head, elm, field)                                  \
  do {                                                                         \
    if (((elm)->field.sqe_next = (head)->sqh_first) == NULL)                   \
      (head)->sqh_last = &(elm)->field.sqe_next;                               \
    (head)->sqh_first = (elm);                                                 \
  } while (0)

#define SIMPLEQ_INSERT_TAIL(head, elm, field)                                  \
  do {                                                                         \
    (elm)->field.sqe_next = NULL;                                              \
    *(head)->sqh_last = (elm);                                                 \
    (head)->sqh_last = &(elm)->field.sqe_next;                                 \
  } while (0)

#define SIMPLEQ_INSERT_AFTER(head, listelm, elm, field)                        \
  do {                                                                         \
    if (((elm)->field.sqe_next = (listelm)->field.sqe_next) == NULL)           \
      (head)->sqh_last = &(elm)->field.sqe_next;                               \
    (listelm)->field.sqe_next = (elm);                                         \
  } while (0)

#define SIMPLEQ_REMOVE_HEAD(head, field)                                       \
  do {                                                                         \
    if (((head)->sqh_first = (head)->sqh_first->field.sqe_next) == NULL)       \
      (head)->sqh_last = &(head)->sqh_first;                                   \
  } while (0)

#define SIMPLEQ_REMOVE_AFTER(head, elm, field)                                 \
  do {                                                                         \
    if (((elm)->field.sqe_next = (elm)->field.sqe_next->field.sqe_next) ==     \
        NULL)                                                                  \
      (head)->sqh_last = &(elm)->field.sqe_next;                               \
  } while (0)

#define SIMPLEQ_CONCAT(head1, head2)                                           \
  do {                                                                         \
    if (!SIMPLEQ_EMPTY((head2))) {                                             \
      *(head1)->sqh_last = (head2)->sqh_first;                                 \
      (head1)->sqh_last = (head2)->sqh_last;                                   \
      SIMPLEQ_INIT((head2));                                                   \
    }                                                                          \
  } while (0)

/*
 * Tail queue definitions.
 */
#define TAILQ_HEAD(name, type)                                                 \
  struct name {                                                                \
    struct type *tqh_first; /* first element */                                \
    struct type **tqh_last; /* addr of last next element */                    \
  }

#define TAILQ_HEAD_INITIALIZER(head)                                           \
  { NULL, &(head).tqh_first }

#define TAILQ_ENTRY(type)                                                      \
  struct {                                                                     \
    struct type *tqe_next;  /* next element */                                 \
    struct type **tqe_prev; /* address of previous next element */             \
  }

/*
 * Tail queue access methods.
 */
#define TAILQ_FIRST(head) ((head)->tqh_first)
#define TAILQ_END(head) NULL
#define TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)
#define TAILQ_LAST(head, headname)                                             \
  (*(((struct headname *)((head)->tqh_last))->tqh_last))
/* XXX */
#define TAILQ_PREV(elm, headname, field)                                       \
  (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#define TAILQ_EMPTY(head) (TAILQ_FIRST(head) == TAILQ_END(head))

#define TAILQ_FOREACH(var, head, field)                                        \
  for ((var) = TAILQ_FIRST(head); (var) != TAILQ_END(head);                    \
       (var) = TAILQ_NEXT(var, field))

#define TAILQ_FOREACH_SAFE(var, head, field, tvar)                             \
  for ((var) = TAILQ_FIRST(head);                                              \
       (var) != TAILQ_END(head) && ((tvar) = TAILQ_NEXT(var, field), 1);       \
       (var) = (tvar))

#define TAILQ_FOREACH_REVERSE(var, head, headname, field)                      \
  for ((var) = TAILQ_LAST(head, headname); (var) != TAILQ_END(head);           \
       (var) = TAILQ_PREV(var, headname, field))

#define TAILQ_FOREACH_REVERSE_SAFE(var, head, headname, field, tvar)           \
  for ((var) = TAILQ_LAST(head, headname);                                     \
       (var) != TAILQ_END(head) &&                                             \
       ((tvar) = TAILQ_PREV(var, headname, field), 1);                         \
       (var) = (tvar))

/*
 * Tail queue functions.
 */
#define TAILQ_INIT(head)                                                       \
  do {                                                                         \
    (head)->tqh_first = NULL;                                                  \
    (head)->tqh_last = &(head)->tqh_first;                                     \
  } while (0)

#define TAILQ_INSERT_HEAD(head, elm, field)                                    \
  do {                                                                         \
    if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)                   \
      (head)->tqh_first->field.tqe_prev = &(elm)->field.tqe_next;              \
    else                                                                       \
      (head)->tqh_last = &(elm)->field.tqe_next;                               \
    (head)->tqh_first = (elm);                                                 \
    (elm)->field.tqe_prev = &(head)->tqh_first;                                \
  } while (0)

#define TAILQ_INSERT_TAIL(head, elm, field)                                    \
  do {                                                                         \
    (elm)->field.tqe_next = NULL;                                              \
    (elm)->field.tqe_prev = (head)->tqh_last;                                  \
    *(head)->tqh_last = (elm);                                                 \
    (head)->tqh_last = &(elm)->field.tqe_next;                                 \
  } while (0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, field)                          \
  do {                                                                         \
    if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)           \
      (elm)->field.tqe_next->field.tqe_prev = &(elm)->field.tqe_next;          \
    else                                                                       \
      (head)->tqh_last = &(elm)->field.tqe_next;                               \
    (listelm)->field.tqe_next = (elm);                                         \
    (elm)->field.tqe_prev = &(listelm)->field.tqe_next;                        \
  } while (0)

#define TAILQ_INSERT_BEFORE(listelm, elm, field)                               \
  do {                                                                         \
    (elm)->field.tqe_prev = (listelm)->field.tqe_prev;                         \
    (elm)->field.tqe_next = (listelm);                                         \
    *(listelm)->field.tqe_prev = (elm);                                        \
    (listelm)->field.tqe_prev = &(elm)->field.tqe_next;                        \
  } while (0)

#define TAILQ_REMOVE(head, elm, field)                                         \
  do {                                                                         \
    if (((elm)->field.tqe_next) != NULL)                                       \
      (elm)->field.tqe_next->field.tqe_prev = (elm)->field.tqe_prev;           \
    else                                                                       \
      (head)->tqh_last = (elm)->field.tqe_prev;                                \
    *(elm)->field.tqe_prev = (elm)->field.tqe_next;                            \
  } while (0)

#define TAILQ_REPLACE(head, elm, elm2, field)                                  \
  do {                                                                         \
    if (((elm2)->field.tqe_next = (elm)->field.tqe_next) != NULL)              \
      (elm2)->field.tqe_next->field.tqe_prev = &(elm2)->field.tqe_next;        \
    else                                                                       \
      (head)->tqh_last = &(elm2)->field.tqe_next;                              \
    (elm2)->field.tqe_prev = (elm)->field.tqe_prev;                            \
    *(elm2)->field.tqe_prev = (elm2);                                          \
  } while (0)

#define TAILQ_CONCAT(head1, head2, field)                                      \
  do {                                                                         \
    if (!TAILQ_EMPTY(head2)) {                                                 \
      *(head1)->tqh_last = (head2)->tqh_first;                                 \
      (head2)->tqh_first->field.tqe_prev = (head1)->tqh_last;                  \
      (head1)->tqh_last = (head2)->tqh_last;                                   \
      TAILQ_INIT((head2));                                                     \
    }                                                                          \
  } while (0)

#endif /* !_QUEUE_H_ */
