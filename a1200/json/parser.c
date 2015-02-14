#include <stdlib.h>

#include "std/memory.h"
#include "parser.h"

void ParserInit(ParserT *parser, TokenT *tokens, int num) {
  parser->tokens = tokens;
  parser->num = num;
  parser->pos = 0;
  parser->errmsg = NULL;
}

static TokenT *ParserMatch(ParserT *parser, TokenIdT tokid) {
  if (parser->tokens[parser->pos].id == tokid) {
    parser->pos++;
    return &parser->tokens[parser->pos - 1];
  }
  return NULL;
}

static bool ParsePair(ParserT *parser, JsonPairT *pair) {
  TokenT *string = ParserMatch(parser, TOK_STRING);

  if (!string) {
    parser->errmsg = "string expected";
    return false;
  }

  if (!ParserMatch(parser, TOK_COLON)) {
    parser->errmsg = "':' expected";
    return false;
  }

  if (!ParseValue(parser, &pair->value))
    return false;

  pair->key = StrNDup(string->value + 1, string->size - 2);
  return true;
}

static bool ParseObject(ParserT *parser, JsonNodeT *node) {
  int i = 0;

  if (ParserMatch(parser, TOK_RBRACE) && node->u.object.num == 0)
    return true;

  while (true) {
    if (!ParsePair(parser, &node->u.object.item[i++]))
      return false;

    if (ParserMatch(parser, TOK_RBRACE))
      break;

    if (!ParserMatch(parser, TOK_COMMA)) {
      parser->errmsg = "',' or '}' expected";
      return false;
    }
  }

  return true;
}

static bool ParseArray(ParserT *parser, JsonNodeT *node) {
  int i = 0;

  if (ParserMatch(parser, TOK_RBRACKET) && node->u.array.num == 0)
    return true;

  while (true) {
    if (!ParseValue(parser, &node->u.array.item[i++]))
      return false;

    if (ParserMatch(parser, TOK_RBRACKET))
      break;

    if (!ParserMatch(parser, TOK_COMMA)) {
      parser->errmsg = "',' or ']' expected";
      return false;
    }
  }

  return true;
}

bool ParseValue(ParserT *parser, JsonNodeT **node_p) {
  TokenT *token;
  JsonNodeT *node = *node_p;

  if (!node)
    (*node_p) = node = NewInstance(JsonNodeT);

  if ((token = ParserMatch(parser, TOK_LBRACE))) {
    node->type = JSON_OBJECT;
    node->u.object.num = token->size;
    if (token->size)
      node->u.object.item = NewTable(JsonPairT, token->size);
    return ParseObject(parser, node);
  }
  else if ((token = ParserMatch(parser, TOK_LBRACKET))) {
    node->type = JSON_ARRAY;
    node->u.array.num = token->size;
    if (token->size)
      node->u.array.item = NewTable(JsonNodeT *, token->size);
    return ParseArray(parser, node);
  }
  else if ((token = ParserMatch(parser, TOK_INTEGER))) {
    node->type = JSON_INTEGER;
    node->u.integer = strtol(token->value, NULL, 10);
    return true;
  }
  else if ((token = ParserMatch(parser, TOK_REAL))) {
    node->type = JSON_REAL;
    node->u.real = strtod(token->value, NULL);
    return true;
  }
  else if ((token = ParserMatch(parser, TOK_STRING))) {
    node->type = JSON_STRING;
    node->u.string = StrNDup(token->value + 1, token->size - 2);
    return true;
  }
  else if ((token = ParserMatch(parser, TOK_TRUE)) ||
           (token = ParserMatch(parser, TOK_FALSE))) {
    node->type = JSON_BOOLEAN;
    node->u.boolean = (token->id == TOK_TRUE) ? true : false;
    return true;
  }
  else if ((token = ParserMatch(parser, TOK_NULL))) {
    node->type = JSON_NULL;
    return true;
  }

  parser->errmsg = "value expected";
  return false;
} 
