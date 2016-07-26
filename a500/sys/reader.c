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

/* Moves cursor to next word in a line. Returns FALSE if encountered end of line
 * or file. Always updates the pointer! Understands escape character '\' and
 * single line comments beginning with '#' character.  */
__regargs BOOL NextWord(char **strptr) {
  char *str = *strptr;
  BOOL escape = FALSE;
  BOOL comment = FALSE;
  BOOL found = FALSE;
  char c;

  while ((c = *str) && !found) {
    if (!escape) {
      if (c == '#')
        comment = TRUE;
      else if (c == '\n')
        break;
      else if (!comment && !isspace(c))
        found = TRUE;
    }
    str++;
    escape = (!escape && c == '\\');
  }

  *strptr = str;
  return found;
}

/* Moves cursor to first white space character after next word. Understands
 * escape character '\'. */
__regargs void SkipWord(char **strptr) {
  char *str = *strptr;
  BOOL escape = FALSE;

  if (NextWord(&str)) {
    char c;

    while ((c = *str)) {
      if (!escape && isspace(c))
        break;
      str++;
      escape = (!escape && c == '\\');
    }

    *strptr = str;
  }
}

/* Checks if the cursor can move through comments and white spaces to the end of
 * line or file. If so, it updates the position of cursor and returns TRUE. */
__regargs BOOL EndOfLine(char **strptr) {
  char *str = *strptr;

  if (NextWord(&str))
    return FALSE;

  *strptr = str;
  return TRUE;
}

/* Moves cursor to next word of a non-empty line. Returns FALSE if there's no
 * next line. */
__regargs BOOL NextLine(char **strptr) {
  char *str = *strptr;

  while (!NextWord(&str)) {
    if (*str == '\n')
      str++;
    if (*str == '\0')
      break;
  }

  *strptr = str;
  return *str;
}

/* Moves cursor to the first character of next line. If there's no next line
 * moves cursor to '\0' character. */
__regargs void SkipLine(char **strptr) {
  char *str = *strptr;

  while (NextWord(&str))
    SkipWord(&str);

  if (*str == '\n')
    str++;

  *strptr = str;
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

__regargs BOOL ReadShort(char **strptr, WORD *numptr) {
  LONG num;
  BOOL res = ReadInt(strptr, &num);
  *numptr = num;
  return res;
}

__regargs BOOL ReadInt(char **strptr, LONG *numptr) {
  char *str = *strptr;
  char c;

  ULONG num = 0;
  BOOL minus = FALSE;
  BOOL hex = FALSE;

  /* Skip white spaces. */
  if (!NextWord(&str))
    return FALSE;

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

    while (isxdigit(c)) {
      num = num * 16 + xdigit(c);
      c = *(++str);
    }
  } else {
    if (!isdigit(c))
      return FALSE;

    while (isdigit(c)) {
      num = num * 10 + digit(c);
      c = *(++str);
    }
  }

  *strptr = str;

  if (numptr)
    *numptr = minus ? -num : num;

  return TRUE;
}

/* Symbol is defined by "[a-zA-Z][a-zA-Z0-9_\.]*" regular expression. */
__regargs WORD ReadSymbol(char **strptr, char **symptr) {
  char *str = *strptr;
  WORD n = 1;

  if (!NextWord(&str))
    return 0;

  *symptr = str;

  {
    char c = *str++;

    if (!isalpha(c))
      return 0;

    while (isalnum(c) || (c == '_') || (c == '.')) {
      c = *str++;
      n++;
    }
  }

  *strptr = str;

  return n;
}
