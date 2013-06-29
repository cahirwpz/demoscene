#include <stdio.h>

#include "json.h"

static void PrintSpaces(int n) {
  while (n--)
    putchar(' ');
}

void JsonPrint(JsonNodeT *node, int indent) {
  int i;

  switch (node->type) {
    case JSON_NULL:
      printf("null");
      break;
    case JSON_BOOLEAN:
      printf(node->u.boolean ? "true" : "false");
      break;
    case JSON_INTEGER:
      printf("%ld", node->u.integer);
      break;
    case JSON_REAL:
      printf("%f", node->u.real);
      break;
    case JSON_STRING:
      printf("%s", node->u.string);
      break;
    case JSON_ARRAY:
      putchar('[');
      if (node->u.array.num)
        putchar('\n');
      for (i = 0; i < node->u.array.num; i++) {
        JsonPrint(node->u.array.item[i], indent + 2);
        if (i < node->u.array.num - 1)
          puts(",");
        else
          putchar('\n');
      }
      if (node->u.array.num)
        PrintSpaces(indent);
      putchar(']');
      break;
    case JSON_OBJECT:
      PrintSpaces(indent);
      putchar('{');
      if (node->u.object.num)
        putchar('\n');
      for (i = 0; i < node->u.object.num; i++) {
        PrintSpaces(indent + 1);
        printf("%s : ", node->u.object.item[i].key);
        JsonPrint(node->u.object.item[i].value, indent + 2);
        if (i < node->u.object.num - 1)
          puts(",");
        else
          putchar('\n');
      }
      if (node->u.object.num)
        PrintSpaces(indent);
      putchar('}');
      break;
  }
}
