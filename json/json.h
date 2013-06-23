#ifndef __JSON_H__
#define __JSON_H__

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
      struct JsonNode *item;
    } array;
    struct {
      int num;
      struct JsonPair *item;
    } object;
  };
} JsonNodeT;

typedef struct JsonPair {
  char *key;
  JsonNodeT value;
} JsonPairT;

#endif
