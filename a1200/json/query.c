#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "std/debug.h"
#include "json/json.h"

/*
 * JsonQuery recursively traverses Json structure according to data contained
 * in a path.
 *
 * The path is composed of elements separated with '/' character.
 * The elements are strings representing either object members or array
 * indices.
 *
 * Examples:
 *  - "result/0/tags/3"
 *  - "people/1/name"
 */

JsonNodeT *JsonQuery(JsonNodeT *node, const char *path) {
  while (node && path) {
    JsonNodeT *child = NULL;
    char *next, *end;
    int i, len;
 
    next = strchr(path, '/');
    len = next ? (next - path) : strlen(path);

    if (next)
      next++;

    if (node->type == JSON_OBJECT) {
      for (i = 0; i < node->u.object.num; i++) {
        if (!strncmp(path, node->u.object.item[i].key, len)) {
          child = node->u.object.item[i].value;
          break;
        }
      }
    } else if (node->type == JSON_ARRAY) {
      i = strtol(path, &end, 10);

      if (end - path < len)
        return NULL;

      child = (i >= 0 && i < node->u.array.num) ? node->u.array.item[i] : NULL;
    }

    node = child;
    path = next;
  }

  return node;
}

JsonNodeT *JsonQuerySafe(JsonNodeT *node, const char *path) {
  JsonNodeT *result = JsonQuery(node, path);

  ASSERT(node, "Node '%s' does not exist.", path);

  return result;
}

JsonNodeT *JsonQueryArray(JsonNodeT *node, const char *path) {
  node = JsonQuerySafe(node, path);

  ASSERT(node->type == JSON_ARRAY, "Item '%s' is not an array.", path);

  return node;
}

JsonNodeT *JsonQueryObject(JsonNodeT *node, const char *path) {
  node = JsonQuerySafe(node, path);

  ASSERT(node->type == JSON_OBJECT, "Item '%s' is not an object.", path);

  return node;
}

const char *JsonQueryString(JsonNodeT *node, const char *path) {
  node = JsonQuerySafe(node, path);

  ASSERT(node->type == JSON_STRING, "Item '%s' is not a string.", path);

  return node->u.string;
}

int JsonQueryInteger(JsonNodeT *node, const char *path) {
  node = JsonQuerySafe(node, path);

  ASSERT(node->type == JSON_INTEGER, "Item '%s' is not an integer.", path);

  return node->u.integer;
}

float JsonQueryNumber(JsonNodeT *node, const char *path) {
  node = JsonQuerySafe(node, path);

  ASSERT(node->type == JSON_INTEGER || node->type == JSON_REAL,
         "Item '%s' is not a number.", path);

  return (node->type == JSON_INTEGER) ? (float)node->u.integer : node->u.real;
}

bool JsonQueryBoolean(JsonNodeT *node, const char *path) {
  node = JsonQuerySafe(node, path);

  ASSERT (node->type == JSON_BOOLEAN, "Item '%s' is not a boolean.", path);

  return node->u.boolean;
}
