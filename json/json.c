#include "std/debug.h"
#include "std/memory.h"

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
      MemUnref(node->u.string);
      break;

    case JSON_ARRAY:
      for (i = 0; i < node->u.array.num; i++)
        FreeJsonNode(node->u.array.item[i]);
      MemUnref(node->u.array.item);
      break;

    case JSON_OBJECT:
      for (i = 0; i < node->u.object.num; i++) {
        FreeJsonNode(node->u.object.item[i].value);
        MemUnref(node->u.object.item[i].key);
      }
      MemUnref(node->u.object.item);
      break;
  }

  MemUnref(node);
}

static bool CountTokens(const char *json, int *num_p) {
  LexerT lexer;
  TokenT token;

  LexerInit(&lexer, json);

  (*num_p) = 0;

  while (LexerNextToken(&lexer, &token))
    (*num_p)++;

  if (lexer.pos < lexer.end) {
    LOG("Error: %s at position %d.",
        lexer.errmsg, (int)(lexer.end - lexer.start));
    return false;
  }

  LOG("Read %d tokens.", (*num_p));
  return true;
}

static TokenT *ReadTokens(const char *json, int num) {
  LexerT lexer;
  TokenT *tokens = NULL;
  int i;

  LexerInit(&lexer, json);

  if ((tokens = NewTable(TokenT, num))) {
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

  LOG("Lexing JSON.");

  if (CountTokens(json, &num)) {
    ParserT parser;

    /* read tokens into an array */
    TokenT *tokens = ReadTokens(json, num);

    /* now... parse! */
    ParserInit(&parser, tokens, num);

    LOG("Parsing JSON.");

    if (!ParseValue(&parser, &node)) {
#ifdef DEBUG_LEXER
      LOG("Parse error: %s at token ", parser.errmsg);
      TokenPrint(&parser.tokens[parser.pos]);
#else
      LOG("Parse error: %s.", parser.errmsg);
#endif
      FreeJsonNode(node);
      node = NULL;
    } else {
      LOG("Parsing finished.");
    }

    MemUnref(tokens);
  }

  return node;
}