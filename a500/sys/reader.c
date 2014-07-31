#include "reader.h"

static inline BOOL isspace(char c) {
  return (c == '\n') || (c == '\t') || (c == ' ');
}

static inline BOOL isdigit(char c) {
  return (c >= '0' && c <= '9');
}

__regargs BOOL ReadNumber(char **strptr, WORD *numptr) {
  char *str = *strptr;
  char c;

  UWORD num = 0;
  BOOL minus = FALSE;

  /* Skip white spaces. */
  while ((c = *str)) {
    if (!isspace(c))
      break;
    str++;
  }

  /* Read optional sign character. */
  if (*str == '-') {
    minus = TRUE;
    str++;
  }

  /* At least one digit. */
  c = *str;

  if (!isdigit(c))
    return FALSE;

  num = c - '0';
  str++;

  while ((c = *str)) {
    if (!isdigit(c))
      break;
    num = num * 10 + (c - '0');
    str++;
  }

  *strptr = str;
  *numptr = minus ? -num : num;

  return TRUE;
}
