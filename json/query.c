#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

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

JsonNodeT *JsonQuery(JsonNodeT *node, char *path) {
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
