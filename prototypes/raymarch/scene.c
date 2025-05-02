#define NAME "scene"
#include "raymarch.h"

static mat4 bm, tm, sm;
static vec3 lightPos;

void PerFrame(void) {
  iTime = SDL_GetTicks() / 1000.0;

  sm = m4_move(0, 1, 5);

  mat4 to = m4_move(-3, 1, 6);
  mat4 tr = m4_rotate(iTime, 0., 0.);
  tm = m4_mul(tr, to);

  mat4 bo = m4_move(0, 1, 8);
  mat4 br = m4_rotate(0., iTime * .2, 0.);
  bm = m4_mul(br, bo);

  lightPos = (vec3){0.0, 5.0, 6.0};
  lightPos.x += sinf(iTime) * 2.0;
  lightPos.z += cosf(iTime) * 2.0;
}

float GetDist(vec3 p) {
#if 1
  vec3 sp = m4_translate(p, sm);
  float sd = SphereDist(sp, 1.0);
#else
  float sd = MAX_DIST;
#endif

#if 1
  float pd = PlaneDist(p);
#else
  float pd = MAX_DIST;
#endif

#if 1
  float cd = CapsuleDist(p, (vec3){3, 1, 8}, (vec3){3, 3, 8}, .5);
#else
  float cd = MAX_DIST;
#endif

#if 1
  float ccd = CylinderDist(p, (vec3){2, 1, 6}, (vec3){3, 1, 4}, .5);
#else
  float ccd = MAX_DIST;
#endif

  vec3 tp = m4_translate(p, tm);
  float td = TorusDist(tp, 1.0, 0.25);

#if 1
  vec3 bp = m4_translate(p, bm);
  float bd = BoxDist(bp, (vec3){1, 1, 1});
#else
  float bd = MAX_DIST;
#endif

  return fmin(fmin(fmin(ccd, bd), fmin(sd, td)), fmin(pd, cd));
}

vec3 GetNormal(vec3 p) {
  float d = GetDist(p);
  vec3 e = (vec3){0.01, 0.0};
  vec3 n = (vec3){d - GetDist(v3_sub(p, (vec3){e.x, e.y, e.y})),
                  d - GetDist(v3_sub(p, (vec3){e.y, e.x, e.y})),
                  d - GetDist(v3_sub(p, (vec3){e.y, e.y, e.x}))};
  return v3_normalize(n);
}

float RayMarch(vec3 ro, vec3 rd) {
  // distance from origin
  float dO = 0.0;

  for (int i = 0; i < MAX_STEPS; i++) {
    vec3 p = v3_add(ro, v3_mul(rd, dO));
    // distance to the scene
    float dS = GetDist(p);
    if (dS < SURF_DIST)
      break;
    dO += dS;
    // prevent ray from escaping the scene
    if (dO > MAX_DIST)
      break;
  }

  return dO;
}

float GetLight(vec3 p) {
  vec3 l = v3_normalize(v3_sub(lightPos, p));
  vec3 n = GetNormal(p);

  float diff = v3_dot(n, l);

  float d = RayMarch(v3_add(p, v3_mul(n, SURF_DIST)), l);

  if (d < v3_length(lightPos))
    diff *= 0.1;

  return clamp(diff, 0.0, 1.0);
}

void Render(SDL_Surface *canvas) {
  Uint32 *buffer = (Uint32 *)canvas->pixels;

  uint32_t start = SDL_GetTicks();

  for (int y = 0; y < HEIGHT; y++) {
    // SDL_Log("y = %d\n", y);
    for (int x = 0; x < WIDTH; x++) {
      // Normalized pixel coordinates (from -1.0 to 1.0)
      vec3 uv =
        (vec3){(float)x * 2.0 / WIDTH - 1.0, 1.0 - (float)y * 2.0 / HEIGHT};

      // Simple camera model
      vec3 ro = (vec3){0.0, 4.0, 0.0};
      vec3 rd = v3_normalize((vec3){uv.x, uv.y - 0.5, 1.0});

      float d = RayMarch(ro, rd);

      vec3 p = v3_add(ro, v3_mul(rd, d));

      float diff = GetLight(p);

      diff = clamp(diff, 0.0, 1.0);

      int c = (int)(diff * 255.0);
      buffer[y * WIDTH + x] = SDL_MapRGBA(canvas->format, c, c, c, 255);
    }
  }

  uint32_t end = SDL_GetTicks();

  SDL_Log("Render took %dms\n", end - start);
}

int main(void) {
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_Window *window =
    SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                     WIDTH, HEIGHT, 0);

  SDL_Surface *window_surface = SDL_GetWindowSurface(window);

  SDL_Surface *canvas = SDL_CreateRGBSurfaceWithFormat(
    0, WIDTH, HEIGHT, 32, SDL_PIXELFORMAT_RGBA8888);

  int quit = 0;
  SDL_Event event;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = 1;
      }
    }

    PerFrame();

    SDL_LockSurface(canvas);
    Render(canvas);
    SDL_UnlockSurface(canvas);

    SDL_BlitSurface(canvas, 0, window_surface, 0);
    SDL_UpdateWindowSurface(window);

    SDL_Delay(10);
  }

  SDL_Quit();

  return 0;
}
