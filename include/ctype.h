#ifndef __CTYPE_H__
#define __CTYPE_H__

#define _SPACE   1
#define _DIGIT   2
#define _XDIGIT  4
#define _ALPHA   8
#define _ALNUM   (_ALPHA | _DIGIT)

extern const char _ctype_[128];

static inline int isspace(int c) {
  return _ctype_[c] & _SPACE;
}

static inline int isdigit(int c) {
  return _ctype_[c] & _DIGIT;
}

static inline int isxdigit(int c) {
  return _ctype_[c] & _XDIGIT;
}

static inline int isalpha(int c) {
  return _ctype_[c] & _ALPHA;
}

static inline int isalnum(int c) {
  return _ctype_[c] & _ALNUM;
}

#endif /* !__CTYPE_H__ */
