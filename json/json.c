#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

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

typedef struct Parser {
  TokenT *tokens;
  int num;
  int pos;
  char *errmsg;
} ParserT;

void TokenAssignParents(TokenT *tokens, size_t num) {
  int i, parent = 0;

  for (i = 0; i < num; i++) {
    TokenT *token = &tokens[i]; 
    token->parent = parent;

    if (token->id == TOK_LBRACE || token->id == TOK_LBRACKET) {
      token->size = 0;
      parent = i;
    } else if (token->id == TOK_RBRACE || token->id == TOK_RBRACKET) {
      if (parent != i - 1)
        tokens[parent].size++;
      parent = tokens[parent].parent;
    } else if (token->id == TOK_COMMA) {
      tokens[parent].size++;
    }
  }
}

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

static void LexerInit(LexerT *lexer, char *text) {
  lexer->start = text;
  lexer->end = text + strlen(text);
  lexer->pos = text;
  lexer->errmsg = NULL;
}

bool LexerNextToken(LexerT *lexer, TokenT *token) {
  char *prev;

  lexer->errmsg = NULL;

  /* get rid of comments and white spaces */
  do {
    prev = lexer->pos;

    while (isspace(*lexer->pos))
      lexer->pos++;

    if (lexer->pos[0] == '/') {
      if (lexer->pos[1] == '/') {
        /* single line comment */
        char *pos = strchr(lexer->pos + 2, '\n');
        lexer->pos = pos ? pos + 1 : lexer->end;
      } else if (lexer->pos[1] == '*') {
        /* multiline comment */
        char *pos = strstr(lexer->pos + 2, "*/");
        if (!pos) {
          lexer->errmsg = "unfinished multiline comment";
          return false;
        }
        lexer->pos = pos + 2;
      }
    }
  } while (prev < lexer->pos);

  token->value = lexer->pos;
  token->pos = lexer->pos - lexer->start;

  {
    char c = (*lexer->pos);

    if (c == '[') {
      token->id = TOK_LBRACKET;
      lexer->pos++;
    }
    else if (c == ']') {
      token->id = TOK_RBRACKET;
      lexer->pos++;
    }
    else if (c == '{') {
      token->id = TOK_LBRACE;
      lexer->pos++;
    }
    else if (c == '}') {
      token->id = TOK_RBRACE;
      lexer->pos++;
    }
    else if (c == ',') {
      token->id = TOK_COMMA;
      lexer->pos++;
    }
    else if (c == ':') {
      token->id = TOK_COLON;
      lexer->pos++;
    }
    else if (c == '"') {
      token->id = TOK_STRING;
      /* skip string */
      do {
        char *pos = strchr(lexer->pos + 1, '"');
        if (!pos) {
          lexer->errmsg = "malformed string";
          return false;
        }
        lexer->pos = pos + 1;
      } while (lexer->pos[-2] == '\\');
    }
    else if (isdigit(c) || c == '-') 
    {
      char *pos1, *pos2;

      (void)strtol(lexer->pos, &pos1, 10);
      (void)strtod(lexer->pos, &pos2);

      if (pos1 == lexer->pos) {
        lexer->errmsg = "malformed number";
        return false;
      }

      if (pos1 >= pos2) {
        token->id = TOK_INTEGER;
        lexer->pos = pos1;
      } else {
        token->id = TOK_REAL;
        lexer->pos = pos2;
      }
    }
    else if (!strncmp(lexer->pos, "true", 4)) {
      token->id = TOK_TRUE;
      lexer->pos += 4;
    }
    else if (!strncmp(lexer->pos, "false", 5)) {
      token->id = TOK_FALSE;
      lexer->pos += 5;
    }
    else if (!strncmp(lexer->pos, "null", 4)) {
      token->id = TOK_NULL;
      lexer->pos += 4;
    }
    else {
      lexer->errmsg = "unknown token";
      return false;
    }

    token->size = lexer->pos - token->value;
    return true;
  }
}

static void ParserInit(ParserT *parser, TokenT *tokens, int num) {
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

static bool ParseValue(ParserT *parser, JsonNodeT **node_pp);

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

  JsonNodeT *item = &pair->value;

  if (!ParseValue(parser, &item))
    return false;

  pair->key = strndup(string->value, string->size);
  return true;
}

static bool ParseObject(ParserT *parser, JsonNodeT *node) {
  int i = 0;

  if (ParserMatch(parser, TOK_RBRACE) && node->object.num == 0)
    return true;

  while (true) {
    if (!ParsePair(parser, &node->object.item[i++]))
      return false;

    if (ParserMatch(parser, TOK_RBRACE))
      break;

    if (!ParserMatch(parser, TOK_COMMA)) {
      parser->errmsg = "',' or '}' expected";
      return false;
    }
  }

  assert(i == node->object.num);
  return true;
}

static bool ParseArray(ParserT *parser, JsonNodeT *node) {
  int i = 0;

  if (ParserMatch(parser, TOK_RBRACKET) && node->array.num == 0)
    return true;

  while (true) {
    JsonNodeT *item = &node->array.item[i++];

    if (!ParseValue(parser, &item))
      return false;

    if (ParserMatch(parser, TOK_RBRACKET))
      break;

    if (!ParserMatch(parser, TOK_COMMA)) {
      parser->errmsg = "',' or ']' expected";
      return false;
    }
  }

  assert(i == node->array.num);
  return true;
}

static bool ParseValue(ParserT *parser, JsonNodeT **node_pp) {
  TokenT *token;
  JsonNodeT *node = *node_pp;

  if (!node) {
    node = calloc(1, sizeof(JsonNodeT));
    (*node_pp) = node;
  }

  if ((token = ParserMatch(parser, TOK_LBRACE))) {
    node->type = JSON_OBJECT;
    node->object.num = token->size;
    node->object.item = calloc(token->size, sizeof(JsonPairT));
    return ParseObject(parser, node);
  }
  else if ((token = ParserMatch(parser, TOK_LBRACKET))) {
    node->type = JSON_ARRAY;
    node->object.num = token->size;
    node->object.item = calloc(token->size, sizeof(JsonNodeT));
    return ParseArray(parser, node);
  }
  else if ((token = ParserMatch(parser, TOK_INTEGER))) {
    node->type = JSON_INTEGER;
    node->integer = strtol(token->value, NULL, 10);
    return true;
  }
  else if ((token = ParserMatch(parser, TOK_REAL))) {
    node->type = JSON_REAL;
    node->real = strtod(token->value, NULL);
    return true;
  }
  else if ((token = ParserMatch(parser, TOK_STRING))) {
    node->type = JSON_STRING;
    node->string = strndup(token->value, token->size);
    return true;
  }
  else if ((token = ParserMatch(parser, TOK_TRUE)) ||
           (token = ParserMatch(parser, TOK_FALSE))) {
    node->type = JSON_BOOLEAN;
    node->boolean = (token->id == TOK_TRUE) ? true : false;
    return true;
  }
  else if ((token = ParserMatch(parser, TOK_NULL))) {
    node->type = JSON_NULL;
    return true;
  }

  parser->errmsg = "value expected";
  return false;
} 

static JsonNodeT *Parse(ParserT *parser) {
  JsonNodeT *node = NULL;
  if (!ParseValue(parser, &node)) {
    printf("%s: ", parser->errmsg);
    TokenPrint(&parser->tokens[parser->pos]);
    return NULL;
  }
  return node;
}

void PrintSpaces(int n) {
  while (n--)
    putchar(' ');
}

void JsonPrint(JsonNodeT *node, int indent) {
  int i;

  switch (node->type) {
    case JSON_NULL:
      puts("null");
      break;
    case JSON_BOOLEAN:
      puts(node->boolean ? "true" : "false");
      break;
    case JSON_INTEGER:
      printf("%ld", node->integer);
      break;
    case JSON_REAL:
      printf("%f", node->real);
      break;
    case JSON_STRING:
      printf("%s", node->string);
      break;
    case JSON_ARRAY:
      putchar('[');
      if (node->array.num)
        putchar('\n');
      for (i = 0; i < node->array.num; i++) {
        JsonPrint(&node->array.item[i], indent + 2);
        if (i < node->array.num - 1)
          puts(",");
        else
          putchar('\n');
      }
      if (node->array.num)
        PrintSpaces(indent);
      putchar(']');
      break;
    case JSON_OBJECT:
      PrintSpaces(indent);
      putchar('{');
      if (node->object.num)
        putchar('\n');
      for (i = 0; i < node->object.num; i++) {
        PrintSpaces(indent + 1);
        printf("%s : ", node->object.item[i].key);
        JsonPrint(&node->object.item[i].value, indent + 2);
        if (i < node->object.num - 1)
          puts(",");
        else
          putchar('\n');
      }
      if (node->object.num)
        PrintSpaces(indent);
      putchar('}');
      break;
  }
}

static char *ReadText(const char *path) {
  FILE *fh = fopen(path, "r");
  uint32_t filesize;
  char *text = NULL;

  if (fh) {
    (void)fseek(fh, 0, SEEK_END);
    filesize = ftell(fh);
    (void)fseek(fh, 0, SEEK_SET);

    text = malloc(filesize + 1);
    text[filesize] = '\0';
    fread(text, filesize, 1, fh);
    fclose(fh);
  }

  return text;
}

int main(int argc, char **argv) {
  char *json;
  int i;

  for (i = 1; i < argc; i++) {
    if ((json = ReadText(argv[i]))) {
      LexerT lexer;
      int num = 0;

      /* count tokens and check for errors */
      {
        TokenT token;
        LexerInit(&lexer, json);

        while (LexerNextToken(&lexer, &token))
          num++;

        if (lexer.pos < lexer.end)
          printf("Error: %s at position %d.\n",
                 lexer.errmsg, (int)(lexer.end - lexer.start));
      }

      printf("%s: %d tokens.\n", argv[i], num);

      /* read tokens once again, but into an array */
      {
        TokenT *tokens = calloc(num, sizeof(TokenT));
        int j;

        LexerInit(&lexer, json);

        for (j = 0; j < num; j++)
          LexerNextToken(&lexer, &tokens[j]);

        TokenAssignParents(tokens, num);

        for (j = 0; j < num; j++) {
          printf("%4d: ", j);
          TokenPrint(&tokens[j]);
        }

        /* now... parse! */
        {
          ParserT parser;
          JsonNodeT *node;

          ParserInit(&parser, tokens, num);
          node = Parse(&parser);
          JsonPrint(node, 0);
        }

        free(tokens);
      }

      free(json);
    }
  }

  return 0;
}
