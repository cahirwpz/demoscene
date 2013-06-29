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

int main(int argc, char **argv) {
  char *json;
  int i;

  for (i = 1; i < argc; i++) {
    if ((json = ReadText(argv[i]))) {
      JsonNodeT *node = JsonParse(json);
      JsonPrint(node, 0);
      FreeJsonNode(node);
      free(json);
    }
  }

  return 0;
}
