#include <string.h>

#include "lexer.h"

#undef DEBUG_LEXER

void LexerInit(LexerT *lexer, const char *text) {
  lexer->start = (char *)text;
  lexer->end = (char *)text + strlen(text);
  lexer->pos = (char *)text;
  lexer->errmsg = NULL;
}

static inline bool isdigit(char c) {
  return (c >= '0' && c <= '9');
}

static bool SkipNumber(LexerT *lexer, TokenT *token) {
  char *pos = lexer->pos;

  /* optional minus sign */
  if (*pos == '-')
    pos++;

  /* at least one digit is mandatory */
  if (!isdigit(*pos))
    goto error;

  /* 0 | [1-9][0-9]+ */
  if (*pos == '0') {
    pos++;
  } else {
    while (isdigit(*pos))
      pos++;
  }

  /* if no fractional part, then this is integer */
  if (*pos != '.') {
    token->id = TOK_INTEGER;
    lexer->pos = pos;
    return true;
  } else {
    pos++;
  }

  /* fractional part has at least one digit */
  if (!isdigit(*pos++))
    goto error;

  while (isdigit(*pos))
    pos++;

  token->id = TOK_REAL;
  lexer->pos = pos;
  return true;

error:
  lexer->errmsg = "malformed number";
  return false;
}

bool LexerNextToken(LexerT *lexer, TokenT *token) {
  char *prev;

  lexer->errmsg = NULL;

  /* get rid of comments and white spaces */
  do {
    prev = lexer->pos;

    while (true) {
      char c = *lexer->pos;

      if (c != '\n' && c != '\t' && c != ' ')
        break;

      lexer->pos++;
    }

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
      if (!SkipNumber(lexer, token))
        return false;
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
