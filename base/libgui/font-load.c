#include "io.h"
#include "ilbm.h"
#include "memory.h"
#include "reader.h"
#include "font.h"

typedef struct {
  const char *name;
  bool (*func)(char **, FontT *);
} ParserT;

static bool ParseBitmap(char **data, FontT *font) {
  char path[80];
  if (!(ReadString(data, path, sizeof(path)) && EndOfLine(data)))
    return false;
  font->data = LoadILBMCustom(path, BM_DISPLAYABLE);
  return true;
}

static bool ParseHeight(char **data, FontT *font) {
  return ReadShortU(data, &font->height) && EndOfLine(data);
}

static bool ParseSpace(char **data, FontT *font) {
  return ReadShortU(data, &font->space) && EndOfLine(data);
}

static bool ParseCharMap(char **data, FontT *font) {
  short n = 0;

  if (!EndOfLine(data))
    return false;

  while (NextLine(data) && !MatchString(data, "@end")) {
    char key[2];
    u_short i;

    if (!(ReadString(data, key, sizeof(key))))
      return false;

    i = *key - 33;
    if (i >= CHARMAP_SIZE) {
      Log("[Font] '%s' character ignored\n", key);
      SkipLine(data);
      continue;
    }

    if (!(ReadShortU(data, &font->charmap[i].y) &&
          ReadShortU(data, &font->charmap[i].width) &&
          EndOfLine(data)))
      return false;

    n++;
  }

  Log("[Font] %d characters defined\n", n);

  return true;
}

static ParserT TopLevelParser[] = {
  { "@bitmap", &ParseBitmap },
  { "@height", &ParseHeight },
  { "@space", &ParseSpace },
  { "@charmap", &ParseCharMap },
  { NULL, NULL }
};

static bool ParseWith(ParserT *parsers, char **data, FontT *font) {
  ParserT *parser;

  while (NextLine(data)) {
    for (parser = parsers; parser->name; parser++) {
      if (!MatchString(data, parser->name))
        continue;
      if (parser->func(data, font))
        break;
      return false;
    }
  }

  return true;
}

__regargs FontT *LoadFont(const char *filename) {
  char *file = LoadFile(filename, MEMF_PUBLIC);
  char *data = file;
  FontT *font = MemAlloc(sizeof(FontT), MEMF_PUBLIC|MEMF_CLEAR);

  Log("[Font] Parsing '%s' file\n", filename);

  if (!ParseWith(TopLevelParser, &data, font)) {
    Log("[Font] Parse error at position %ld!\n", (ptrdiff_t)(data - file));
    DeleteFont(font);
    MemFree(file);
    return NULL;
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
