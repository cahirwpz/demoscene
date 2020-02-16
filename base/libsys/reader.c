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

__regargs bool MatchString(char **data, const char *pattern) {
  char *str = *data;

  while (*str == *pattern)
    str++, pattern++;

  if (*pattern)
    return false;

  *data = str;
  return true;
}

/* Moves cursor to next word in a line. Returns false if encountered end of line
 * or file. Always updates the pointer! Understands escape character '\' and
 * single line comments beginning with '#' character.  */
__regargs bool NextWord(char **data) {
  char *str = *data;
  bool escape = false;
  bool comment = false;
  bool found = false;
  char c;

  while ((c = *str)) { 
    if (escape) {
      escape = false;
      if (c != '\n') {
        found = true;
        str--;
        break;
      }
    } else if (c == '\\') {
      escape = true;
    } else if (c == '#') {
      comment = true;
    } else if (c == '\n') {
      break;
    } else if (!comment && !isspace(c)) {
      found = true;
      break;
    }
    str++;
  }

  *data = str;
  return found;
}

/* Moves cursor to first white space character after next word. Understands
 * escape character '\'. */
__regargs void SkipWord(char **data) {
  char *str = *data;
  bool escape = false;

  if (NextWord(&str)) {
    char c;

    while ((c = *str)) {
      if (escape) {
        escape = false;
      } else if (c == '\\') {
        escape = true;
      } else if (isspace(c)) {
        break;
      }
      str++;
    }

    *data = str;
  }
}

/* Checks if the cursor can move through comments and white spaces to the end of
 * line or file. If so, it updates the position of cursor and returns true. */
__regargs bool EndOfLine(char **data) {
  char *str = *data;

  if (NextWord(&str))
    return false;

  *data = str;
  return true;
}

/* Moves cursor to next word of a non-empty line. Returns false if there's no
 * next line. */
__regargs bool NextLine(char **data) {
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

static inline short digit(short c) {
  return c - '0';
}

static inline short xdigit(short c) {
  if (c <= '9')
    return c - '0';
  if (c <= 'F')
    return c - ('A' - 10);
  return c - ('a' - 10);
}

__regargs bool ReadByte(char **data, char *numptr) {
  int num;
  bool res = ReadInt(data, &num);
  *numptr = num;
  return res;
}

__regargs bool ReadShort(char **data, short *numptr) {
  int num;
  bool res = ReadInt(data, &num);
  *numptr = num;
  return res;
}

__regargs bool ReadInt(char **data, int *numptr) {
  char *str = *data;
  char c;

  int num = 0;
  bool minus = false;
  bool hex = false;

  /* Skip white spaces. */
  if (!NextWord(&str))
    return false;

  /* Read optional sign character. */
  if (*str == '-')
    str++, minus = true;

  if (*str == '$')
    str++, hex = true;

  /* Read at least one digit. */
  c = *str;

  if (hex) {
    if (!isxdigit(c))
      return false;

    while (isxdigit(c)) {
      num = num * 16 + xdigit(c);
      c = *(++str);
    }
  } else {
    if (!isdigit(c))
      return false;

    while (isdigit(c)) {
      num = num * 10 + digit(c);
      c = *(++str);
    }
  }

  *data = str;

  if (numptr)
    *numptr = minus ? -num : num;

  return true;
}

__regargs bool ReadFloat(char **data, float *numptr) {
  char *str = *data;
  char c;

  int p = 0, q = 1;
  bool minus = false, dot = false;

  /* Skip white spaces. */
  if (!NextWord(&str))
    return false;

  /* Read optional sign character. */
  if (*str == '-')
    str++, minus = true;

  /* Read at least one digit. */
  c = *str;

  if (!isdigit(c))
    return false;

  while (1) {
    if (!dot && c == '.') {
      dot = true;
    } else if (!isdigit(c)) {
      break;
    } else {
      p = p * 10 + digit(c);
      if (dot)
        q *= 10;
    }
    c = *(++str);
  }

  *data = str;

  if (numptr)
    *numptr = SPDiv(SPFlt(q), SPFlt(minus ? -p : p));

  return true;
}

__regargs short ReadString(char **data, char *buf, short buflen) {
  char *str = *data;
  bool escape = false;
  bool quote = false;
  short n = 0;
  char c;

  if (!NextWord(&str))
    return 0;

  if (*str == '"')
    str++, quote = true;

  while ((c = *str) && n < buflen - 1) {
    if (!escape) {
      if (c == '#')
        break;
      else if (c == '\\')
        escape = true;
      else if (c == '"' && quote) {
        str++; break;
      } else if (isspace(c) && !quote)
        break;
      else
        *buf++ = c, n++;
    } else {
      escape = false;

      if (c != '\n') {
        if (c == 't') c = '\t';
        if (c == 'n') c = '\n';
        if (c == 'r') c = '\r';
        if (c == '0') c = '\0';
        *buf++ = c, n++;
      }
    }
    str++;
  }

  if (n == buflen)
    return -1;

  *buf = '\0';
  *data = str;
  return n;
}
