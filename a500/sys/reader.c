#include "reader.h"
#include "ffp.h"

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
  ['a'...'f'] = ALPHA | XDIGIT,
  ['g'...'z'] = ALPHA
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

__regargs BOOL MatchString(char **data, const char *pattern) {
  char *str = *data;

  while (*str == *pattern) {
    str++, pattern++;
  }

  if (*pattern)
    return FALSE;

  *data = str;
  return TRUE;
}

/* Moves cursor to next word in a line. Returns FALSE if encountered end of line
 * or file. Always updates the pointer! Understands escape character '\' and
 * single line comments beginning with '#' character.  */
__regargs BOOL NextWord(char **data) {
  char *str = *data;
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
      else if (!comment && !isspace(c)) {
        found = TRUE;
        break;
      }
    }
    str++;
    escape = (!escape && c == '\\');
  }

  *data = str;
  return found;
}

/* Moves cursor to first white space character after next word. Understands
 * escape character '\'. */
__regargs void SkipWord(char **data) {
  char *str = *data;
  BOOL escape = FALSE;

  if (NextWord(&str)) {
    char c;

    while ((c = *str)) {
      if (!escape && isspace(c))
        break;
      str++;
      escape = (!escape && c == '\\');
    }

    *data = str;
  }
}

/* Checks if the cursor can move through comments and white spaces to the end of
 * line or file. If so, it updates the position of cursor and returns TRUE. */
__regargs BOOL EndOfLine(char **data) {
  char *str = *data;

  if (NextWord(&str))
    return FALSE;

  *data = str;
  return TRUE;
}

/* Moves cursor to next word of a non-empty line. Returns FALSE if there's no
 * next line. */
__regargs BOOL NextLine(char **data) {
  char *str = *data;

  while (!NextWord(&str)) {
    if (*str == '\n')
      str++;
    if (*str == '\0')
      break;
  }

  *data = str;
  return *str;
}

/* Moves cursor to the first character of next line. If there's no next line
 * moves cursor to '\0' character. */
__regargs void SkipLine(char **data) {
  char *str = *data;

  while (NextWord(&str))
    SkipWord(&str);

  if (*str == '\n')
    str++;

  *data = str;
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

__regargs BOOL ReadByte(char **data, BYTE *numptr) {
  LONG num;
  BOOL res = ReadInt(data, &num);
  *numptr = num;
  return res;
}

__regargs BOOL ReadShort(char **data, WORD *numptr) {
  LONG num;
  BOOL res = ReadInt(data, &num);
  *numptr = num;
  return res;
}

__regargs BOOL ReadInt(char **data, LONG *numptr) {
  char *str = *data;
  char c;

  LONG num = 0;
  BOOL minus = FALSE;
  BOOL hex = FALSE;

  /* Skip white spaces. */
  if (!NextWord(&str))
    return FALSE;

  /* Read optional sign character. */
  if (*str == '-')
    str++, minus = TRUE;

  if (*str == '$')
    str++, hex = TRUE;

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

  *data = str;

  if (numptr)
    *numptr = minus ? -num : num;

  return TRUE;
}

__regargs BOOL ReadFloat(char **data, FLOAT *numptr) {
  char *str = *data;
  char c;

  LONG p = 0, q = 1;
  BOOL minus = FALSE, dot = FALSE;

  /* Skip white spaces. */
  if (!NextWord(&str))
    return FALSE;

  /* Read optional sign character. */
  if (*str == '-')
    str++, minus = TRUE;

  /* Read at least one digit. */
  if (!isdigit(c = *str++))
    return FALSE;

  while (1) {
    if (!dot && c == '.') {
      dot = TRUE;
    } else if (!isdigit(c)) {
      break;
    } else {
      p = p * 10 + digit(c);
      if (dot)
        q *= 10;
    }
    c = *str++;
  }

  *data = str;

  if (numptr)
    *numptr = SPDiv(SPFlt(minus ? -p : p), SPFlt(q));

  return TRUE;
}

__regargs WORD ReadString(char **data, char *buf, WORD buflen) {
  char *str = *data;
  BOOL escape = FALSE;
  WORD n = 0;
  char c;

  if (!NextWord(&str))
    return 0;

  while ((c = *str) && n < buflen - 1) {
    if (!escape) {
      if (c == '#')
        break;
      if (isspace(c))
        break;
      *buf++ = c, n++;
    } else if (c != '\n') {
      if (c == 't') c = '\t';
      if (c == 'n') c = '\n';
      if (c == 'r') c = '\r';
      if (c == '0') c = '\0';
      *buf++ = c, n++;
    }
    str++;
    escape = (!escape && c == '\\');
  }

  if (n == buflen)
    return -1;

  buf[n] = '\0';
  *data = str;
  return n;
}
