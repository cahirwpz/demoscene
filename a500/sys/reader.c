#include "reader.h"

static inline BOOL isspace(char c) {
  return (c == '\n') || (c == '\t') || (c == ' ');
}

static inline BOOL isdigit(char c) {
  return (c >= '0' && c <= '9');
}

static inline BOOL isalpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

__regargs void SkipSpaces(char **strptr) {
  char *str = *strptr;
  char c;

  /* Skip white spaces. */
  while ((c = *str)) {
    if (!isspace(c))
      break;
    str++;
  }

  *strptr = str;
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

  if (numptr)
    *numptr = minus ? -num : num;

  return TRUE;
}

__regargs WORD ReadSymbol(char **strptr, char **symptr) {
  char *str = *strptr;
  char *sym = NULL;
  char c;

  /* Skip white spaces. */
  while ((c = *str)) {
    if (!isspace(c))
      break;
    str++;
  }

  sym = str;

  if (!isalpha(c))
    return 0;

  str++;

  while ((c = *str)) {
    if (!isalpha(c) && (c != '_'))
      break;
    str++;
  }

  *strptr = str;
  *symptr = sym;

  return str - sym;
}

__regargs BOOL ExpectSymbol(char **strptr, char *expect) {
  char *symbol = NULL;
  WORD len;

  if (!(len = ReadSymbol(strptr, &symbol)))
    return FALSE;

  if (strlen(expect) != len)
    return FALSE;

  return (memcmp(symbol, expect, len) == 0);
}
