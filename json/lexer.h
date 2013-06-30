#ifndef __JSON_LEXER_H__
#define __JSON_LEXER_H__

#include "std/types.h"

typedef enum {
  TOK_LBRACKET,
  TOK_RBRACKET,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_COMMA,
  TOK_COLON,
  TOK_INTEGER,
  TOK_REAL,
  TOK_STRING,
  TOK_TRUE,
  TOK_FALSE,
  TOK_NULL
} TokenIdT;

typedef struct Token {
  TokenIdT id;
  int parent;
  char *value;
  int pos;
  int size;
} TokenT;

typedef struct Lexer {
  char *start;
  char *end;
  char *pos;
  char *errmsg;
} LexerT;

void LexerInit(LexerT *lexer, const char *text);
bool LexerNextToken(LexerT *lexer, TokenT *token);
void TokenAssignParents(TokenT *tokens, size_t num);
void TokenPrint(TokenT *token);

#endif
