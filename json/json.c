#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "json.h"

void FreeJsonNode(JsonNodeT *node) {
  int i;

  if (!node)
    return;

  switch (node->type) {
    case JSON_NULL:
    case JSON_BOOLEAN:
    case JSON_INTEGER:
    case JSON_REAL:
      break;

    case JSON_STRING:
      free(node->u.string);
      break;

    case JSON_ARRAY:
      for (i = 0; i < node->u.array.num; i++)
        FreeJsonNode(node->u.array.item[i]);
      free(node->u.array.item);
      break;

    case JSON_OBJECT:
      for (i = 0; i < node->u.object.num; i++) {
        FreeJsonNode(node->u.object.item[i].value);
        free(node->u.object.item[i].key);
      }
      free(node->u.object.item);
      break;
  }

  free(node);
}

static bool CountTokens(const char *json, int *num_p) {
  LexerT lexer;
  TokenT token;

  LexerInit(&lexer, json);

  (*num_p) = 0;

  while (LexerNextToken(&lexer, &token))
    (*num_p)++;

  if (lexer.pos < lexer.end) {
    printf("Error: %s at position %d.\n",
           lexer.errmsg, (int)(lexer.end - lexer.start));
    return false;
  }

  printf("%d tokens.\n", (*num_p));
  return true;
}

static TokenT *ReadTokens(const char *json, int num) {
  LexerT lexer;
  TokenT *tokens = NULL;
  int i;

  LexerInit(&lexer, json);

  if ((tokens = calloc(num, sizeof(TokenT)))) {
    LexerInit(&lexer, json);

    for (i = 0; i < num; i++) {
      LexerNextToken(&lexer, &tokens[i]);
#ifdef DEBUG_LEXER
      printf("%4d: ", i);
      TokenPrint(&tokens[i]);
#endif
    }

    TokenAssignParents(tokens, num);
  }

  return tokens;
}

JsonNodeT *JsonParse(const char *json) {
  JsonNodeT *node = NULL;
  int num = 0;

  if (CountTokens(json, &num)) {
    ParserT parser;

    /* read tokens into an array */
    TokenT *tokens = ReadTokens(json, num);

    /* now... parse! */
    ParserInit(&parser, tokens, num);

    puts("Parsing...");

    if (!ParseValue(&parser, &node)) {
#ifdef DEBUG_LEXER
      printf("%s: ", parser.errmsg);
      TokenPrint(&parser.tokens[parser.pos]);
#else
      puts(parser.errmsg);
#endif
      FreeJsonNode(node);
      node = NULL;
    }

    free(tokens);
  }

  return node;
}
