#include <stdio.h>

#include "lexer.h"

void TokenPrint(TokenT *token) {
  char *id = "?";
  int i;

  if (token->id == TOK_LBRACKET)
    id = "[";
  else if (token->id == TOK_RBRACKET)
    id = "]";
  else if (token->id == TOK_LBRACE)
    id = "{";
  else if (token->id == TOK_RBRACE)
    id = "}";
  else if (token->id == TOK_COMMA)
    id = ",";
  else if (token->id == TOK_COLON)
    id = ":";
  else if (token->id == TOK_INTEGER)
    id = "int";
  else if (token->id == TOK_REAL)
    id = "float";
  else if (token->id == TOK_STRING)
    id = "str";
  else if (token->id == TOK_TRUE)
    id = "true";
  else if (token->id == TOK_FALSE)
    id = "false";
  else if (token->id == TOK_NULL)
    id = "null";

  printf("Token( '%s'", id);

  if (token->id == TOK_INTEGER ||
      token->id == TOK_REAL ||
      token->id == TOK_STRING)
  {
      printf(" | ");
      for (i = 0; i < token->size; i++)
        putchar(token->value[i]);
  }

  if (token->id == TOK_LBRACE || token->id == TOK_LBRACKET)
      printf(" | %d", token->size);

  printf(" | parent=%d | pos=%d )\n", token->parent, token->pos);
}
