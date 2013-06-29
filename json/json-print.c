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
        JsonPrint(node->array.item[i], indent + 2);
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
        JsonPrint(node->object.item[i].value, indent + 2);
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
