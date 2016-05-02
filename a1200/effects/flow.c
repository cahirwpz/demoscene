#include <stdlib.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/random.h"

#include "gfx/pixbuf.h"
#include "gfx/palette.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

const int PARTICLES = 4000;

typedef struct Velocity {
  float u, v;
} VelocityT;

typedef struct Particle {
  float x, y;
  bool active;
} ParticleT;

static PixBufT *canvas;
static VelocityT *flow;
static ParticleT *particles;

void AcquireResources() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  flow = ReadFileSimple("data/flow.bin");
  particles = NewTable(ParticleT, PARTICLES);
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
}

void TearDownEffect() {
}

__regargs static void 
GetFilteredVelocity(VelocityT *array, VelocityT *v, float x, float y) 
{
  float xi, yi;
  float xf = modff(x, &xi);
  float yf = modff(y, &yi);
  VelocityT *data = &array[160 * (int)yi + (int)xi];

  VelocityT *p1 = &data[0];
  VelocityT *p2 = &data[1];
  VelocityT *p3 = &data[160];
  VelocityT *p4 = &data[161];

  float du31 = p1->u + ((p3->u - p1->u) * yf);
  float du42 = p2->u + ((p4->u - p2->u) * yf);
  float dv31 = p1->v + ((p3->v - p1->v) * yf);
  float dv42 = p2->v + ((p4->v - p2->v) * yf);

  v->u = du31 + ((du42 - du31) * xf);
  v->v = dv31 + ((dv42 - dv31) * xf);
}

void RenderFlow(PixBufT *canvas) {
  int i, j;

  for (i = 0; i < PARTICLES; i++) {
    if (particles[i].active) {
      float x = particles[i].x;
      float y = particles[i].y;
      VelocityT v;

      GetFilteredVelocity(flow, &v, x * 160.0f, y * 128.0f);

      x += v.u * 750;
      y += v.v * 750;

      if ((x < 0.0) || (x >= 1.0) || (y < 0.0) || (y >= 1.0)) {
        particles[i].active = false;
      } if ((v.u < 2 * 10e-8) && (v.v < 2 * 10e-8)) {
        particles[i].active = false;
      } else {
        particles[i].x = x;
        particles[i].y = y;

        PutPixel(canvas, x * 320.0, y * 256.0, 255);
      }
    }
  }

  for (i = 0, j = 5; i < PARTICLES && j > 0; i++) {
    if (!particles[i].active) {
      static int r = 0xb3a1778;

      particles[i].x = 0.0f;
      particles[i].y = (RandomInt32(&r) & 255) / 256.0f;
      particles[i].active = true;

      j--;
    }
  }
}

void RenderEffect(int frameNumber) {
  int n = canvas->width * canvas->height;
  uint8_t *data = canvas->data;

  do {
    int p = *data;
    p -= 32;
    if (p < 0)
      p = 0;
    *data++ = p;
  } while (--n);

  PROFILE(Flow)
    RenderFlow(canvas);
  PROFILE(C2P)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
