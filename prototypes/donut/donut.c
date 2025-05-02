#include <SDL.h>
#include <math.h>

static const int WIDTH = 640;
static const int HEIGHT = 512;

static const int MAX_STEPS = 100;
static const float MAX_DIST = 100.0;
static const float SURF_DIST = 0.01;

static float iTime;

typedef struct vec3 {
  float x, y, z, w;
} vec3;

typedef struct mat4 {
  float m00, m01, m02, m03;
  float m10, m11, m12, m13;
  float m20, m21, m22, m23;
  float m30, m31, m32, m33;
} mat4;

/* https://en.wikipedia.org/wiki/Fast_inverse_square_root */
float Q_rsqrt(float number) {
  union {
    float f;
    uint32_t i;
  } conv = {.f = number};
  conv.i = 0x5f3759df - (conv.i >> 1);
  conv.f *= 1.5f - (number * 0.5f * conv.f * conv.f);
  return conv.f;
}

float clamp(float v, float min, float max) {
  return fmin(fmax(v, min), max);
}

vec3 v3_abs(vec3 p) {
  return (vec3){fabs(p.x), fabs(p.y), fabs(p.z)};
}

vec3 v3_max(vec3 p, float v) {
  return (vec3){fmax(p.x, v), fmax(p.y, v), fmax(p.z, v)};
}

vec3 v3_add(vec3 a, vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 v3_sub(vec3 a, vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3 v3_mul(vec3 a, float v) {
  return (vec3){a.x * v, a.y * v, a.z * v};
}

float v3_dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float v3_length(vec3 p) {
  return sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
}

vec3 v3_normalize(vec3 p) {
  /* return v3_mul(p, 1.0f / v3_length(p)); */
  return v3_mul(p, Q_rsqrt(p.x * p.x + p.y * p.y + p.z * p.z));
}

mat4 m4_mul(mat4 a, mat4 b) {
  mat4 c;

  float *ma = (float *)&a.m00;
  float *mb = (float *)&b.m00;
  float *mc = (float *)&c.m00;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      mc[i * 4 + j] = 0;
      for (int k = 0; k < 4; k++) {
        mc[i * 4 + j] += ma[i * 4 + k] * mb[k * 4 + j];
      }
    }
  }

  return c;
}

/* @brief Rotate around X axis */
mat4 m4_rotateX(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){1, 0, 0, 0, 0, c, -s, 0, 0, s, c, 0, 0, 0, 0, 1};
}

/* @brief Rotate around Y axis */
mat4 m4_rotateY(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, 0, 0, 0, 1};
}

/* @brief Rotate around Z axis */
mat4 m4_rotateZ(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

mat4 m4_move(float x, float y, float z) {
  return (mat4){1, 0, 0, -x, 0, 1, 0, -y, 0, 0, 1, -z, 0, 0, 0, 1};
}

mat4 m4_rotate(float x, float y, float z) {
  return m4_mul(m4_mul(m4_rotateX(x), m4_rotateY(y)), m4_rotateZ(z));
}

vec3 m4_translate(vec3 p, mat4 a) {
  vec3 r;

  p.w = 1.0;

  float *vp = (float *)&p.x;
  float *vr = (float *)&r.x;
  float *ma = (float *)&a.m00;

  for (int i = 0; i < 4; i++) {
    vr[i] = 0;
    for (int j = 0; j < 4; j++) {
      vr[i] += ma[i * 4 + j] * vp[j];
    }
  }

  return r;
}

/* @brief Calculate distance from a torus */
float TorusDist(vec3 p, float r1, float r2) {
  float r = sqrtf(p.x * p.x + p.z * p.z) - r1;
  float q = sqrtf(r * r + p.y * p.y);
  return q - r2;
}

static mat4 tm;
static vec3 lightPos;

void PerFrame(void) {
  iTime = SDL_GetTicks() / 1000.0;

  mat4 to = m4_move(0, 0, 6);
  mat4 tr = m4_rotate(iTime, iTime, 0.);
  tm = m4_mul(tr, to);

  lightPos = (vec3){0.0, 5.0, 6.0};
  lightPos.x += sinf(iTime) * 2.0;
  lightPos.z += cosf(iTime) * 2.0;
}

float GetDist(vec3 p) {
  return TorusDist(m4_translate(p, tm), 2.0, 0.666);
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
