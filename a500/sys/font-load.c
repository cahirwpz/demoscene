#include "io.h"
#include "ilbm.h"
#include "memory.h"
#include "reader.h"
#include "font.h"

typedef struct {
  const char *name;
  BOOL (*func)(char **, FontT *);
} ParserT;

static BOOL ParseBitmap(char **data, FontT *font) {
  char path[80];
  if (!(ReadString(data, path, sizeof(path)) && EndOfLine(data)))
    return FALSE;
  font->data = LoadILBMCustom(path, BM_DISPLAYABLE);
  return TRUE;
}

static BOOL ParseHeight(char **data, FontT *font) {
  return ReadShort(data, &font->height) && EndOfLine(data);
}

static BOOL ParseSpace(char **data, FontT *font) {
  return ReadShort(data, &font->space) && EndOfLine(data);
}

static BOOL ParseCharMap(char **data, FontT *font) {
  WORD n = 0;

  if (!EndOfLine(data))
    return FALSE;

  while (NextLine(data) && !MatchString(data, "@end")) {
    char key[2];
    UWORD i;

    if (!(ReadString(data, key, sizeof(key))))
      return FALSE;

    i = *key - 33;
    if (i >= CHARMAP_SIZE) {
      Log("[Font] '%s' character ignored\n", key);
      SkipLine(data);
      continue;
    }

    if (!(ReadShort(data, &font->charmap[i].y) &&
          ReadShort(data, &font->charmap[i].width) &&
          EndOfLine(data)))
      return FALSE;

    n++;
  }

  Log("[Font] %ld characters defined\n", (LONG)n);

  return TRUE;
}

static ParserT TopLevelParser[] = {
  { "@bitmap", &ParseBitmap },
  { "@height", &ParseHeight },
  { "@space", &ParseSpace },
  { "@charmap", &ParseCharMap },
  { NULL, NULL }
};

__regargs FontT *LoadFont(char *filename) {
  char *file = LoadFile(filename, MEMF_PUBLIC);
  char *data = file;
  FontT *font = MemAlloc(sizeof(FontT), MEMF_PUBLIC|MEMF_CLEAR);

  Log("[Font] Parsing '%s' file\n", filename);

  while (NextLine(&data)) {
    ParserT *parser = TopLevelParser;
    
    for (; parser->name; parser++) {
      if (!MatchString(&data, parser->name))
        continue;
      if (parser->func(&data, font))
        break;
      Log("[Font] Parse error at position %ld!\n", (LONG)(data - file));
      DeleteFont(font);
      MemFree(file);
      return NULL;
    }
  }

  MemFree(file);
  return font;
}

__regargs void DeleteFont(FontT *font) {
  if (font) {
    DeleteBitmap(font->data);
    MemFree(font);
  }
}
