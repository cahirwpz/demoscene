#include <stdio.h>
#include <stdlib.h>

#include "json.h"

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

static void Usage(const char *program) {
  fprintf(stderr, "Usage: %s file.json [path1 path2 ...]\n", program);
  exit(1);
}

int main(int argc, char **argv) {
  char *json;
  int i;

  if (argc < 2)
    Usage(argv[0]);

  if ((json = ReadText(argv[1]))) {
    JsonNodeT *node = JsonParse(json);

    free(json);

    if (argc == 2) {
      JsonPrint(node, 0);
    } else {
      for (i = 2; i < argc; i++) {
        JsonNodeT *child = JsonQuery(node, argv[i]);
        printf("%d: result for '%s':\n", i - 1, argv[i]);
        if (child) {
          JsonPrint(child, 2);
          putchar('\n');
        } else {
          puts("(not found)");
        }
      }
    }

    FreeJsonNode(node);
  }

  return 0;
}
