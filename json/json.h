#ifndef __JSON_H__
#define __JSON_H__

#include "std/types.h"

typedef enum {
  JSON_NULL,
  JSON_BOOLEAN,
  JSON_INTEGER, 
  JSON_REAL,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT
} JsonNodeTypeT;

struct JsonPair;

typedef struct JsonNode {
  JsonNodeTypeT type;
  union {
    bool boolean;
    long integer;
    double real;
    char *string;
    struct {
      int num;
      struct JsonNode **item;
    } array;
    struct {
      int num;
      struct JsonPair *item;
    } object;
  } u;
} JsonNodeT;

typedef struct JsonPair {
  char *key;
  JsonNodeT *value;
} JsonPairT;

TYPEDEF(JsonNodeT);

JsonNodeT *JsonParse(const char *text);
JsonNodeT *JsonQuery(JsonNodeT *node, char *path); 
void JsonPrint(JsonNodeT *node, int indent);

#endif
