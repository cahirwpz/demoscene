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

typedef void (*JsonObjectIterFuncT)(const char *key, JsonNodeT *value);

JsonNodeT *JsonParse(const char *text);
JsonNodeT *JsonQuery(JsonNodeT *node, const char *path); 
JsonNodeT *JsonQueryObject(JsonNodeT *node,
                           const char *path, JsonNodeT *defval);
const char *JsonQueryString(JsonNodeT *node,
                            const char *path, const char *defval);
bool JsonQueryBoolean(JsonNodeT *node,
                     const char *path, bool defval);
void JsonObjectForEach(JsonNodeT *node, JsonObjectIterFuncT func);
void JsonPrint(JsonNodeT *node, int indent);

#endif
