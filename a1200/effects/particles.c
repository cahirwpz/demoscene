#include <stdlib.h>

#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"

#include "engine/plane.h"
#include "engine/sphere.h"
#include "engine/vector3d.h"
#include "gfx/blit.h"
#include "gfx/ellipse.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "txtgen/procedural.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

typedef struct ParticleFlags {
  uint32_t inactive : 1;
} ParticleFlagsT;

typedef struct Particle {
  ParticleFlagsT flags;
  Vector3D position;
  Vector3D velocity;
  Vector3D force;
  float mass;
  int age;
} ParticleT;

typedef struct ParticleEngine ParticleEngineT;
typedef void (*ParticleEngineFuncT)(ParticleEngineT *engine);

struct ParticleEngine {
  ParticleT *particles;
  ParticleEngineFuncT adder;
};

static void DeleteParticleEngine(ParticleEngineT *self) {
  MemUnref(self->particles);
}

TYPEDECL(ParticleEngineT, (FreeFuncT)DeleteParticleEngine);

ParticleEngineT *NewParticleEngine(size_t maxParticles,
                                   ParticleEngineFuncT adder)
{
  ParticleEngineT *engine = NewInstance(ParticleEngineT);

  engine->particles = MemNewTable(maxParticles, sizeof(ParticleT));
  engine->adder = adder;

  return engine;
}

/*
 * 1) Add particles.
 * 2) Clear forces.
 * 3) Calculate forces.
 * 4) Detect collisions.
 * 5) Remove unwanted particles.
 * 6) Apply velocity change due to collisions.
 * 7) Move particles.
 */
void ParticleEngineStep(ParticleEngineT *engine) {
  static SphereT sphere = { {0.0, 0.0, 0.0}, 80.0f };
  int i;

  for (i = 0; i < TableSize(engine->particles); i++) {
    ParticleT *particle = &engine->particles[i];

    particle->force.x = 0;
    particle->force.y = 0;
    particle->force.z = 0;
  }

  if (engine->adder)
    engine->adder(engine);

  /* No forces right now */
  for (i = 0; i < TableSize(engine->particles); i++) {
    ParticleT *particle = &engine->particles[i];

    float before, after;
    Vector3D newPosition;

    V3D_Add(&newPosition, &particle->position, &particle->velocity);

    before = PointDistanceFromSphere(&sphere, &particle->position);
    after = PointDistanceFromSphere(&sphere, &newPosition);

    particle->flags.inactive = BOOL(before * after < 0);

    if (!particle->flags.inactive) {
      V3D_Add(&particle->velocity, &particle->velocity, &particle->force);
      V3D_Add(&particle->position, &particle->position, &particle->velocity);
      particle->age++;
    }
  }
}

static void AddParticles(ParticleEngineT *engine) {
  static uint16_t seed[3] = { 0xDEAD, 0x1EE7, 0xC0DE };
  int i;

  for (i = 0; i < TableSize(engine->particles); i++) {
    ParticleT *p = &engine->particles[i];

    if (p->flags.inactive) {
      float radian = erand48(seed) * 4 * M_PI;
      float radius = erand48(seed) * 3 + 1;

      p->velocity.x = cos(radian) * radius;
      p->velocity.y = sin(radian) * radius;
      p->velocity.z = 0.0f;
      break;
    }
  }
}

static PixBufT *flare;
static ParticleEngineT *engine;
static PixBufT *canvas;
static PixBufT *flare;

void AcquireResources() {
  flare = NewPixBuf(PIXBUF_GRAY, 32, 32);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  engine = NewParticleEngine(30, AddParticles);
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  float lightRadius = 1.0f;

  PixBufClear(canvas);
  GeneratePixels(flare, (GenPixelFuncT)LightNormalFalloff, &lightRadius);
  PixBufSetBlitMode(flare, BLIT_SUBSTRACTIVE_CLIP);
}

void TearDownEffect() {
}

void RenderFlares(int frameNumber) {
  int i;

  PixBufClear(canvas);
  ParticleEngineStep(engine);

  canvas->fgColor = 192;
  DrawEllipse(canvas, WIDTH / 2, HEIGHT / 2, 90, 90);
  canvas->fgColor = 64;
  DrawEllipse(canvas, WIDTH / 2, HEIGHT / 2, 70, 70);

  for (i = 0; i < TableSize(engine->particles); i++) {
    ParticleT *particle = &engine->particles[i];
    PointT point = { particle->position.x + WIDTH / 2,
                     particle->position.y + HEIGHT / 2 };

    PixBufBlit(canvas, point.x - flare->width / 2, point.y - flare->height / 2,
               flare, NULL);
  }

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void MainLoop() {
  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderFlares(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while (ReadLoopEvent() != LOOP_EXIT);
}
