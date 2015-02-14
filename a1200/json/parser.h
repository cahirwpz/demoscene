#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include "lexer.h"
#include "json.h"

typedef struct Parser {
  TokenT *tokens;
  int num;
  int pos;
  char *errmsg;
} ParserT;

void ParserInit(ParserT *parser, TokenT *tokens, int num);
bool ParseValue(ParserT *parser, JsonNodeT **node_p);

#endif
