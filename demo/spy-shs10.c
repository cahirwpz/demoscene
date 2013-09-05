#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

#include "audio/stream.h"
#include "uvmap/generate.h"
#include "uvmap/render.h"
#include "gfx/blit.h"
#include "gfx/colors.h"
#include "gfx/palette.h"

#include "system/audio.h"
#include "system/c2p.h"
#include "system/display.h"

#include "config.h"
#include "demo.h"
#include "timeline.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

const char *DemoConfigPath = "spy-shs10.json";

static AudioStreamT *TheMusic = NULL;
static PixBufT *TheCanvas = NULL;
static PixBufT *TheLoadImg = NULL;
static PaletteT *TheLoadPal = NULL;

/*
 * Load demo.
 */
bool LoadDemo() {
  const char *loadImgPath = JsonQueryString(DemoConfig, "load/image");
  const char *loadPalPath = JsonQueryString(DemoConfig, "load/palette");
  const char *musicPath = JsonQueryString(DemoConfig, "music/file");

  if ((TheLoadImg = NewPixBufFromFile(loadImgPath)) &&
      (TheLoadPal = NewPaletteFromFile(loadPalPath)) &&
      (TheMusic = AudioStreamOpen(musicPath)) &&
      (TheCanvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT)) &&
      InitDisplay(WIDTH, HEIGHT, DEPTH))
  {
    c2p1x1_8_c5_bm(TheLoadImg->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
    LoadPalette(TheLoadPal);
    DisplaySwap();

    return true;
  }

  return false;
}

/*
 * Transition to demo.
 */
void BeginDemo() {
  /* Release loading screen image... */
  MemUnref(TheLoadImg);
  MemUnref(TheLoadPal);

  /* Play the audio stream... */
  AudioStreamPlay(TheMusic);
}

/*
 * Tear down demo.
 */
void KillDemo() {
  UnlinkPalettes(R_("11.pal"));
  UnlinkPalettes(R_("texture-1.pal"));
  UnlinkPalettes(R_("texture-2.pal"));
  UnlinkPalettes(R_("texture-3.pal"));
  UnlinkPalettes(R_("texture-4.pal"));
  UnlinkPalettes(R_("texture-5.pal"));
  AudioStreamStop(TheMusic);

  MemUnref(TheMusic);
  MemUnref(TheCanvas);
}

/*
 * User updated the time.
 */
void DemoUpdateTime(int oldFrameNumber, int newFrameNumber) {
  AudioStreamUpdatePos(TheMusic);
}

#ifdef GENERATEMAPS
UVMapGenerate(0,
              0.3f / (r + 0.5f * x),
              3.0f * a / M_PI);

UVMapGenerate(1,
              x * cos(2.0f * r) - y * sin(2.0f * r),
              y * cos(2.0f * r) + x * sin(2.0f * r));

UVMapGenerate(2,
              pow(r, 0.33f),
              a / M_PI + r);

UVMapGenerate(3,
              cos(a) / (3 * r),
              sin(a) / (3 * r));

UVMapGenerate(4,
              0.04f * y + 0.06f * cos(a * 3) / r,
              0.04f * x + 0.06f * sin(a * 3) / r);

UVMapGenerate(5,
              0.1f * y / (0.11f + r * 0.15f),
              0.1f * x / (0.11f + r * 0.15f));

UVMapGenerate(6,
              0.5f * a / M_PI + 0.25f * r,
              pow(r, 0.25f));

UVMapGenerate(7,
              0.5f * a / M_PI,
              sin(5.0f * r));

UVMapGenerate(8,
              3.0f * a / M_PI,
              sin(3.0f * r) + 0.5f * cos(5.0f * a));
#endif

/*
 * Set up resources.
 */
void SetupResources() {
  ResAdd("EffectPal", NewPalette(256));

#ifdef GENERATEMAPS
  ResAdd("Map0", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map1", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map2", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map3", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map4", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map5", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map6", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map7", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));
  ResAdd("Map8", NewUVMap(WIDTH, HEIGHT, UV_OPTIMIZED, 256, 256));

  LOG("Generating map 0.");
  UVMapGenerate0(R_("Map0"));
  UVMapWriteToFile(R_("Map0"), "map0.bin");
  LOG("Generating map 1.");
  UVMapGenerate1(R_("Map1"));
  UVMapWriteToFile(R_("Map1"), "map1.bin");
  LOG("Generating map 2.");
  UVMapGenerate2(R_("Map2"));
  UVMapWriteToFile(R_("Map2"), "map2.bin");
  LOG("Generating map 3.");
  UVMapGenerate3(R_("Map3"));
  UVMapWriteToFile(R_("Map3"), "map3.bin");
  LOG("Generating map 4.");
  UVMapGenerate4(R_("Map4"));
  UVMapWriteToFile(R_("Map4"), "map4.bin");
  LOG("Generating map 5.");
  UVMapGenerate5(R_("Map5"));
  UVMapWriteToFile(R_("Map5"), "map5.bin");
  LOG("Generating map 6.");
  UVMapGenerate6(R_("Map6"));
  UVMapWriteToFile(R_("Map6"), "map6.bin");
  LOG("Generating map 7.");
  UVMapGenerate7(R_("Map7"));
  UVMapWriteToFile(R_("Map7"), "map7.bin");
  LOG("Generating map 8.");
  UVMapGenerate8(R_("Map8"));
  UVMapWriteToFile(R_("Map8"), "map8.bin");
#else
  ResAdd("Map0", NewUVMapFromFile("data/map0.bin"));
  ResAdd("Map1", NewUVMapFromFile("data/map1.bin"));
  ResAdd("Map2", NewUVMapFromFile("data/map2.bin"));
  ResAdd("Map3", NewUVMapFromFile("data/map3.bin"));
  ResAdd("Map4", NewUVMapFromFile("data/map4.bin"));
  ResAdd("Map5", NewUVMapFromFile("data/map5.bin"));
  ResAdd("Map6", NewUVMapFromFile("data/map6.bin"));
  ResAdd("Map7", NewUVMapFromFile("data/map7.bin"));
  ResAdd("Map8", NewUVMapFromFile("data/map8.bin"));
#endif
}

/*** Part 1 ******************************************************************/

PARAMETER(PixBufT *, TheTexture, NULL);
PARAMETER(PaletteT *, TheTexturePal, NULL);
PARAMETER(UVMapT *, TheMap, NULL);
PARAMETER(PixBufT *, TheTitle, NULL);
PARAMETER(PaletteT *, TheTitlePal, NULL);

CALLBACK(SetupPart1a) {
  AudioStreamSetVolume(TheMusic, 0.5f);
  LinkPalettes(TheTexturePal, TheTitlePal, NULL);
  PixBufRemap(TheTitle, TheTitlePal);
  LoadPalette(TheTexturePal);
}

CALLBACK(SetupPart1b) {
  LinkPalettes(TheTexturePal, TheTitlePal, NULL);
  PixBufRemap(TheTitle, TheTitlePal);
  LoadPalette(TheTexturePal);
}

CALLBACK(SetupPart1c) {
  LinkPalettes(TheTexturePal, R_("slider.pal"), NULL);
  PixBufRemap(R_("slider.8"), R_("slider.pal"));
  PixBufRemap(R_("knob.8"), R_("slider.pal"));
  LoadPalette(TheTexturePal);
}

CALLBACK(VolumeUp) {
  int frames = frame->last - frame->first + 1;
  float volume = 0.5f + 0.5f * ((float)frame->number / frames);
  float dx = 1.75f * (frame->number * 4) / 3;
  float dy = 1.75f * frame->number;

  AudioStreamSetVolume(TheMusic, volume);

  PixBufBlit(TheCanvas, 0, 0, R_("slider.8"), NULL);
  PixBufBlit(TheCanvas, 50 + (int)dx , 110 - (int)dy, R_("knob.8"), NULL);
}

CALLBACK(ShowVolume) {
  PixBufBlit(TheCanvas, 0, 0, R_("slider.8"), NULL);
  PixBufBlit(TheCanvas, 50, 110, R_("knob.8"), NULL);
}

CALLBACK(RenderPart1) {
  int du = 2 * frame->number;
  int dv = 4 * frame->number;

  UVMapSetOffset(TheMap, du, dv);
  UVMapSetTexture(TheMap, TheTexture);
  UVMapRender(TheMap, TheCanvas);
}

CALLBACK(ShowTitle) {
  int frames = frame->beat * 8;
  int w = TheTitle->width;
  int h = TheTitle->height;
  int x, y;

  if (frame->number < frames) {
    w = w * frame->number / frames;
    h = h * frame->number / frames;
  }

  x = (WIDTH - w) / 2;
  y = (HEIGHT - h) / 2;

  PixBufBlitScaled(TheCanvas, x, y, w, h, TheTitle);
}

/*** Part 2 ******************************************************************/

PARAMETER(PixBufT *, TheImage, NULL);
PARAMETER(PaletteT *, TheImagePal, NULL);

static PaletteT *ThePalette = NULL;

static FrameT EpisodeFrame;
static int EpisodeNum = 0;

CALLBACK(SetupEpisode) {
  EpisodeNum++;

  AudioStreamSetVolume(TheMusic, 1.0f);

  if (TheImage->uniqueColors <= 128) {
    ThePalette = TheTexturePal;
    LinkPalettes(ThePalette, TheImagePal, NULL);
    PixBufRemap(TheImage, TheImagePal);
  } else if (TheImage->uniqueColors <= 192) {
    ThePalette = TheImagePal;

    if (EpisodeNum == 5) {
      LinkPalettes(ThePalette, R_("greets1.pal"), NULL);
      PixBufRemap(R_("greets1.8"), R_("greets1.pal"));
    } else if (EpisodeNum == 6) {
      LinkPalettes(ThePalette, R_("greets2.pal"), NULL);
      PixBufRemap(R_("greets2.8"), R_("greets2.pal"));
    }
  } else {
    ThePalette = TheImagePal;
  }

  memcpy(&EpisodeFrame, frame, sizeof(FrameT));
}

CALLBACK(Waiving) {
  if ((frame->number / (int)frame->beat) & 1) {
    TheImage = R_("14-2.8");
    TheImagePal = R_("14-2.pal");
  } else {
    TheImage = R_("14-1.8");
    TheImagePal = R_("14-1.pal");
  }

  SetupEpisode(frame);
}

static bool CountBeat(FrameT *frame) {
  static int lastFrame = 0;

  float lf = lastFrame / frame->beat;
  float tf = frame->number / frame->beat;
  float li, ti;

  lf = modff(lf, &li);
  tf = modff(tf, &ti);

  lastFrame = frame->number;

  return li < ti;
}

static void PaletteEffect(FrameT *frame, PaletteT *src, PaletteT *dst) {
  bool beatFlash = CountBeat(frame);
  int j = 0;

  while (src) {
    int i;

    for (i = 0; i < src->count; i++) {
      RGB col = src->colors[i];

      int r = col.r;
      int g = col.g;
      int b = col.b;

      if (beatFlash) {
        r += 64; g += 64; b += 64;
      }

      {
        int f = frame->number;

        if (f >= 25)
          f = (frame->last - frame->first) - f;

        if (f < 25) {
          r = r * f / 25;
          g = g * f / 25;
          b = b * f / 25;
        }
      }

      dst->colors[j++] = MakeRGB(r, g, b);

      if (j >= 256)
        break;
    }

    src = src->next;
  }
  
  LoadPalette(dst);
}

CALLBACK(RenderPart2) {
  if (TheImage->uniqueColors <= 128) {
    int du = 2 * frame->number;
    int dv = 2 * frame->number;

    UVMapSetTexture(TheMap, TheTexture);
    UVMapSetOffset(TheMap, du, dv);
    UVMapRender(TheMap, TheCanvas);
  }

  EpisodeFrame.number = frame->number + frame->first - EpisodeFrame.first;

  {
    RectT rect = { 0, 0, WIDTH, HEIGHT };

    frame = &EpisodeFrame;

    if (TheImage->height > HEIGHT) {
      int frames = frame->last - frame->first + 1;
      int f = frame->number % frames;
      int dy = (f < frames / 2) ? f : (frames - f);

      rect.y = (int)((float)(TheImage->height - HEIGHT) * 2 * dy / frames);
    }

    PixBufBlit(TheCanvas, 0, 0, TheImage, &rect);

    PaletteEffect(frame, ThePalette, R_("EffectPal"));
  }

  if (EpisodeNum == 5 || EpisodeNum == 6) {
    RectT rect = { 0, 0, WIDTH, HEIGHT };
    int frames = frame->last - frame->first + 1;

    if (EpisodeNum == 5) {
      PixBufT *TheGreets = R_("greets1.8");

      rect.x = (TheGreets->width - WIDTH) * frame->number / frames;

      PixBufBlit(TheCanvas, 0, 54, TheGreets, &rect);
    } else {
      PixBufT *TheGreets = R_("greets2.8");

      rect.x = (TheGreets->width - WIDTH) * (frames - frame->number) / frames;

      PixBufBlit(TheCanvas, 0, 54, TheGreets, &rect);
    }
  }
}

/*** The demo ****************************************************************/

CALLBACK(Render) {
  c2p1x1_8_c5_bm(TheCanvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

CALLBACK(FeedAudioStream) {
  AudioStreamFeed(TheMusic);
}

CALLBACK(Quit) {
  ExitDemo = true;
}

#include "spy-shs10.syms"
