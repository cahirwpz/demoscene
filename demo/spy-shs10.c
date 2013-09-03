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
#include "tools/frame.h"

#include "system/audio.h"
#include "system/c2p.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

#include "config.h"
#include "demo.h"
#include "timeline.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

const char *DemoConfigPath = "spy-shs10.json";

static struct {
  bool showFrame; /* show the number of current frame */
  bool timeKeys;  /* enable rewinding and fast-forward keys */

  AudioStreamT *music;
  PixBufT *canvas;
  PixBufT *loadImg;
  PaletteT *loadPal;

  PaletteT *palette;
  PixBufT *image;
} Demo;

/*
 * Load demo.
 */
bool LoadDemo() {
  const char *loadImgPath = JsonQueryString(DemoConfig, "load/image");
  const char *loadPalPath = JsonQueryString(DemoConfig, "load/palette");
  const char *musicPath = JsonQueryString(DemoConfig, "music/file");

  Demo.showFrame = JsonQueryBoolean(DemoConfig, "flags/show-frame");
  Demo.timeKeys = JsonQueryBoolean(DemoConfig, "flags/time-keys");

  if ((Demo.loadImg = NewPixBufFromFile(loadImgPath)) &&
      (Demo.loadPal = NewPaletteFromFile(loadPalPath)) &&
      (Demo.music = AudioStreamOpen(musicPath)) &&
      (Demo.canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT)) &&
      InitDisplay(WIDTH, HEIGHT, DEPTH))
  {
    c2p1x1_8_c5_bm(Demo.loadImg->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
    LoadPalette(Demo.loadPal);
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
  MemUnref(Demo.loadImg);
  MemUnref(Demo.loadPal);

  /* Play the audio stream... */
  AudioStreamPlay(Demo.music);
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
  AudioStreamStop(Demo.music);

  MemUnref(Demo.music);
  MemUnref(Demo.canvas);
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

/*
 * Handle events during the demo.
 */
void HandleEvents(int frameNumber) {
  static InputEventT event; 
  static int counter = 1;

  while (EventQueuePop(&event)) {
    if (event.ie_Class == IECLASS_RAWKEY) {
      if (event.ie_Code & IECODE_UP_PREFIX) {
        if (Demo.timeKeys) {
          switch (event.ie_Code & ~IECODE_UP_PREFIX) {
            case KEY_UP:
              ChangeVBlankCounter(-10 * FRAMERATE);
              AudioStreamUpdatePos(Demo.music);
              break;

            case KEY_DOWN:
              ChangeVBlankCounter(10 * FRAMERATE);
              AudioStreamUpdatePos(Demo.music);
              break;

            case KEY_LEFT:
              ChangeVBlankCounter(-FRAMERATE);
              AudioStreamUpdatePos(Demo.music);
              break;

            case KEY_RIGHT:
              ChangeVBlankCounter(FRAMERATE);
              AudioStreamUpdatePos(Demo.music);
              break;

            case KEY_SPACE:
              LOG("Event %d at %.2f (frame %d).",
                  counter++, (float)frameNumber / FRAMERATE, frameNumber);
              break;

            default:
              break;
          }
        }

        if ((event.ie_Code & ~IECODE_UP_PREFIX) == KEY_ESCAPE)
          ExitDemo = TRUE;
      }
    }
  }
}

/*** Part 1 ******************************************************************/

PARAMETER(PixBufT *, TheTexture, NULL);
PARAMETER(PaletteT *, TheTexturePal, NULL);
PARAMETER(UVMapT *, TheMap, NULL);
PARAMETER(PixBufT *, TheTitle, NULL);
PARAMETER(PaletteT *, TheTitlePal, NULL);

CALLBACK(SetupPart1a) {
  AudioStreamSetVolume(Demo.music, 0.5f);
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

  AudioStreamSetVolume(Demo.music, volume);

  PixBufBlit(Demo.canvas, 0, 0, R_("slider.8"), NULL);
  PixBufBlit(Demo.canvas, 50 + (int)dx , 110 - (int)dy, R_("knob.8"), NULL);
}

CALLBACK(ShowVolume) {
  PixBufBlit(Demo.canvas, 0, 0, R_("slider.8"), NULL);
  PixBufBlit(Demo.canvas, 50, 110, R_("knob.8"), NULL);
}

CALLBACK(RenderPart1) {
  int du = 2 * frame->number;
  int dv = 4 * frame->number;

  UVMapSetOffset(TheMap, du, dv);
  UVMapSetTexture(TheMap, TheTexture);
  UVMapRender(TheMap, Demo.canvas);
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

  PixBufBlitScaled(Demo.canvas, x, y, w, h, TheTitle);
}

/*** Part 2 ******************************************************************/

static FrameT EpisodeFrame;
static int EpisodeNum = 0;

void SetupEpisode(FrameT *frame, char *imgName, char *palName) {
  EpisodeNum++;

  AudioStreamSetVolume(Demo.music, 1.0f);

  Demo.image = R_(imgName);

  if (Demo.image->uniqueColors <= 128) {
    Demo.palette = TheTexturePal;
    LinkPalettes(Demo.palette, R_(palName), NULL);
    PixBufRemap(R_(imgName), R_(palName));
  } else if (Demo.image->uniqueColors <= 192) {
    Demo.palette = R_(palName);

    if (EpisodeNum == 5) {
      LinkPalettes(Demo.palette, R_("greets1.pal"), NULL);
      PixBufRemap(R_("greets1.8"), R_("greets1.pal"));
    } else if (EpisodeNum == 6) {
      LinkPalettes(Demo.palette, R_("greets2.pal"), NULL);
      PixBufRemap(R_("greets2.8"), R_("greets2.pal"));
    }
  } else {
    Demo.palette = R_(palName);
  }

  memcpy(&EpisodeFrame, frame, sizeof(FrameT));
}

CALLBACK(Image01) { SetupEpisode(frame, "01.8", "01.pal"); }
CALLBACK(Image02) { SetupEpisode(frame, "02.8", "02.pal"); }
CALLBACK(Image03) { SetupEpisode(frame, "03.8", "03.pal"); }
CALLBACK(Image04) { SetupEpisode(frame, "04.8", "04.pal"); }
CALLBACK(Image05) { SetupEpisode(frame, "05.8", "05.pal"); }
CALLBACK(Image06) { SetupEpisode(frame, "06.8", "06.pal"); }
CALLBACK(Image07) { SetupEpisode(frame, "07.8", "07.pal"); }
CALLBACK(Image08) { SetupEpisode(frame, "08.8", "08.pal"); }
CALLBACK(Image09) { SetupEpisode(frame, "09.8", "09.pal"); }
CALLBACK(Image10) { SetupEpisode(frame, "10.8", "10.pal"); }
CALLBACK(Image11) { SetupEpisode(frame, "11.8", "11.pal"); }
CALLBACK(Image12) { SetupEpisode(frame, "12.8", "12.pal"); }
CALLBACK(Image13) { SetupEpisode(frame, "13.8", "13.pal"); }
CALLBACK(Image15) { SetupEpisode(frame, "15.8", "15.pal"); }
CALLBACK(Image16) { SetupEpisode(frame, "16.8", "16.pal"); }
CALLBACK(Image17) { SetupEpisode(frame, "17.8", "17.pal"); }
CALLBACK(Image18) { SetupEpisode(frame, "18.8", "18.pal"); }
CALLBACK(Image19) { SetupEpisode(frame, "19.8", "19.pal"); }
CALLBACK(ImageEnd1) { SetupEpisode(frame, "end1.8", "end1.pal"); }
CALLBACK(ImageEnd2) { SetupEpisode(frame, "end2.8", "end2.pal"); }
CALLBACK(ImageAudio) { SetupEpisode(frame, "audio.8", "audio.pal"); }
CALLBACK(ImageCode) { SetupEpisode(frame, "code.8", "code.pal"); }
CALLBACK(ImageGfx) { SetupEpisode(frame, "gfx.8", "gfx.pal"); }
CALLBACK(ImagePics) { SetupEpisode(frame, "pics.8", "pics.pal"); }

CALLBACK(Waiving) {
  if ((frame->number / (int)frame->beat) & 1) {
    SetupEpisode(frame, "14-2.8", "14-2.pal");
  } else {
    SetupEpisode(frame, "14-1.8", "14-1.pal");
  }
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
  if (Demo.image->uniqueColors <= 128) {
    int du = 2 * frame->number;
    int dv = 2 * frame->number;

    UVMapSetTexture(TheMap, TheTexture);
    UVMapSetOffset(TheMap, du, dv);
    UVMapRender(TheMap, Demo.canvas);
  }

  EpisodeFrame.number = frame->number + frame->first - EpisodeFrame.first;

  {
    RectT rect = { 0, 0, WIDTH, HEIGHT };

    frame = &EpisodeFrame;

    if (Demo.image->height > HEIGHT) {
      int frames = frame->last - frame->first + 1;
      int f = frame->number % frames;
      int dy = (f < frames / 2) ? f : (frames - f);

      rect.y = (int)((float)(Demo.image->height - HEIGHT) * 2 * dy / frames);
    }

    PixBufBlit(Demo.canvas, 0, 0, Demo.image, &rect);

    PaletteEffect(frame, Demo.palette, R_("EffectPal"));
  }

  if (EpisodeNum == 5 || EpisodeNum == 6) {
    RectT rect = { 0, 0, WIDTH, HEIGHT };
    int frames = frame->last - frame->first + 1;

    if (EpisodeNum == 5) {
      PixBufT *TheGreets = R_("greets1.8");

      rect.x = (TheGreets->width - WIDTH) * frame->number / frames;

      PixBufBlit(Demo.canvas, 0, 54, TheGreets, &rect);
    } else {
      PixBufT *TheGreets = R_("greets2.8");

      rect.x = (TheGreets->width - WIDTH) * (frames - frame->number) / frames;

      PixBufBlit(Demo.canvas, 0, 54, TheGreets, &rect);
    }
  }
}

/*** The demo ****************************************************************/

CALLBACK(Render) {
  c2p1x1_8_c5_bm(Demo.canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);

  if (Demo.showFrame) {
    RenderFrameNumber(frame->number);
    RenderFramesPerSecond(frame->number);
  }
}

CALLBACK(FeedAudioStream) {
  AudioStreamFeed(Demo.music);
}

CALLBACK(Quit) {
  ExitDemo = true;
}

#include "spy-shs10.syms"
