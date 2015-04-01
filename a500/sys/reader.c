#include "reader.h"

#define SPACE   1
#define DIGIT   2
#define XDIGIT  4
#define ALPHA   8
#define ALNUM   (ALPHA | DIGIT)

static const char ctype[128] = {
  ['\t'] = SPACE,
  ['\n'] = SPACE,
  ['\v'] = SPACE,
  ['\f'] = SPACE,
  ['\r'] = SPACE,
  [' '] = SPACE,
  ['0'...'9'] = DIGIT | XDIGIT,
  ['A'...'F'] = ALPHA | XDIGIT, 
  ['G'...'Z'] = ALPHA,
  ['a'...'z'] = ALPHA | XDIGIT,
  ['a'...'z'] = ALPHA
};

static inline int isspace(int c) {
  return ctype[c] & SPACE;
}

static inline int isdigit(int c) {
  return ctype[c] & DIGIT;
}

static inline int isxdigit(int c) {
  return ctype[c] & XDIGIT;
}

static inline int isalpha(int c) {
  return ctype[c] & ALPHA;
}

static inline int isalnum(int c) {
  return ctype[c] & ALNUM;
}

__regargs char *SkipSpaces(char *str) {
  do {
    char c = *str;
    if (!c || !isspace(c))
      break;
    str++;
  } while (1);

  return str;
}

__regargs char *NextLine(char *str) {
  do {
    char c = *str;
    if (!c)
      break;
    str++;
    if (c == '\n')
      break;
  } while (1);

  return str;
}

static inline WORD digit(WORD c) {
  return c - '0';
}

static inline WORD xdigit(WORD c) {
  if (c <= '9')
    return c - '0';
  if (c <= 'F')
    return c - ('A' - 10);
  return c - ('a' - 10);
}

__regargs BOOL ReadNumber(char **strptr, WORD *numptr) {
  char *str = *strptr;
  char c;

  UWORD num = 0;
  BOOL minus = FALSE;
  BOOL hex = FALSE;

  /* Skip white spaces. */
  str = SkipSpaces(str);

  /* Read optional sign character. */
  if (*str == '-') {
    minus = TRUE;
    str++;
  }

  if (*str == '$') {
    hex = TRUE;
    str++;
  }

  /* Read at least one digit. */
  c = *str;

  if (hex) {
    if (!isxdigit(c))
      return FALSE;

    while (c && isxdigit(c)) {
      num = num * 16 + xdigit(c);
      c = *(++str);
    }
  } else {
    if (!isdigit(c))
      return FALSE;

    while (c && isdigit(c)) {
      num = num * 10 + digit(c);
      c = *(++str);
    }
  }

  *strptr = str;

  if (numptr)
    *numptr = minus ? -num : num;

  return TRUE;
}

__regargs WORD ReadSymbol(char **strptr, char **symptr) {
  char *str = *strptr;
  char *sym;

  sym = str = SkipSpaces(str);

  if (!isalpha(*str))
    return 0;

  str++;

  do {
    char c = *str;
    if (!c || (!isalnum(c) && (c != '_') && (c != '.')))
      break;
    str++;
  } while (1);

  *strptr = str;
  *symptr = sym;

  return str - sym;
}
