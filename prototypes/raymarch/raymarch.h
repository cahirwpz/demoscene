#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdbool.h>

#define __unused __attribute__((unused))

/* Requested render buffer size */
#define WIDTH 320
#define HEIGHT 240

/* Maximum number of textures */
#define NCHANNELS 16

/* Raymarcher parameters */
static const int MAX_STEPS = 1000;
static const float MAX_DIST = 100.0;
static const float SURF_DIST = 0.01;

static float iTime;
static SDL_Surface *iChannel[NCHANNELS];

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

static __unused float clamp(float v, float min, float max) {
  return fmin(fmax(v, min), max);
}

static __unused vec3 v3_abs(vec3 p) {
  return (vec3){fabs(p.x), fabs(p.y), fabs(p.z)};
}

static __unused vec3 v3_max(vec3 p, float v) {
  return (vec3){fmax(p.x, v), fmax(p.y, v), fmax(p.z, v)};
}

static __unused vec3 v3_add(vec3 a, vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static __unused vec3 v3_sub(vec3 a, vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static __unused vec3 v3_mul(vec3 a, float v) {
  return (vec3){a.x * v, a.y * v, a.z * v};
}

static __unused float v3_dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static __unused vec3 v3_cross(vec3 a, vec3 b) {
  return (vec3){a.y * b.z - b.y * a.z,
                a.z * b.x - b.z * a.x,
                a.x * b.y - b.x * a.y};
}

static __unused float v3_length(vec3 p) {
  return sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
}

static __unused vec3 v3_normalize(vec3 p) {
#if 1
  return v3_mul(p, 1.0f / v3_length(p));
#else
  return v3_mul(p, Q_rsqrt(p.x * p.x + p.y * p.y + p.z * p.z));
#endif
}

static __unused mat4 m4_mul(mat4 a, mat4 b) {
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
static __unused mat4 m4_rotate_x(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){1, 0, 0, 0, 0, c, -s, 0, 0, s, c, 0, 0, 0, 0, 1};
}

/* @brief Rotate around Y axis */
static __unused mat4 m4_rotate_y(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, 0, 0, 0, 1};
}

/* @brief Rotate around Z axis */
static __unused mat4 m4_rotate_z(float angle) {
  float s = sinf(angle);
  float c = cosf(angle);
  return (mat4){c, -s, 0, 0, s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

static __unused mat4 m4_move(float x, float y, float z) {
  return (mat4){1, 0, 0, -x, 0, 1, 0, -y, 0, 0, 1, -z, 0, 0, 0, 1};
}

static __unused mat4 m4_rotate(float x, float y, float z) {
  return m4_mul(m4_mul(m4_rotate_x(x), m4_rotate_y(y)), m4_rotate_z(z));
}

static __unused vec3 m4_translate(vec3 p, mat4 a) {
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

// Position:
//  x: left (-) <=> right (+)
//  y: down (-) <=> up (+)
//  z: back (-) <=> front (+)
static __unused mat4 m4_camera_lookat(vec3 pos, vec3 target, float rot) {
  // camera direction aka forward
	vec3 f = v3_normalize(v3_sub(target, pos));
  // camera up vector
	vec3 up = (vec3){sinf(rot), cosf(rot), 0.0};

	vec3 r = v3_normalize(v3_cross(f, up));
	vec3 u = v3_cross(r, f);

  return (mat4){
    r.x, u.x, f.x, 0.0,
    r.y, u.y, f.y, 0.0,
    r.z, u.z, f.z, 0.0,
    0.0, 0.0, 0.0, 1.0};
}

/* @brief Calculate distance from sphere
 * @param p point of interest
 * @param r sphere radius
 * @return distance of s from p
 */
static __unused float SphereDist(vec3 p, float r) {
  return v3_length(p) - r;
}

/* @brief Calculate distance from plane at (0,0,0) origin
 * @param p point of interest
 * @return distance of plane from p
 */
static __unused float PlaneDist(vec3 p) {
  return p.y;
}

/* @brief Calculate distance from a capsule */
static __unused float CapsuleDist(vec3 p, vec3 a, vec3 b, float r) {
  vec3 ab = v3_sub(b, a);
  vec3 ap = v3_sub(p, a);

  float t = v3_dot(ab, ap) / v3_dot(ab, ab);

  t = clamp(t, 0.0, 1.0);

  vec3 c = v3_add(a, v3_mul(ab, t));

  return v3_length(v3_sub(p, c)) - r;
}

/* @brief Calculate distance from a torus */
static __unused float TorusDist(vec3 p, float r1, float r2) {
  float r = sqrtf(p.x * p.x + p.z * p.z) - r1;
  float q = sqrtf(r * r + p.y * p.y);
  return q - r2;
}

/* @brief Calculate distance from a box */
static __unused float BoxDist(vec3 p, vec3 s) {
  vec3 d = v3_sub(v3_abs(p), s);
  float e = v3_length(v3_max(d, 0.0));
  float i = fmin(fmax(d.x, fmax(d.y, d.z)), 0.0);
  return e + i;
}

/* @brief Calculate distance from a cylinder */
static __unused float CylinderDist(vec3 p, vec3 a, vec3 b, float r) {
  vec3 ab = v3_sub(b, a);
  vec3 ap = v3_sub(p, a);

  float t = v3_dot(ab, ap) / v3_dot(ab, ab);

  vec3 c = v3_add(a, v3_mul(ab, t));
  float x = v3_length(v3_sub(p, c)) - r;
  float y = (fabs(t - 0.5) - 0.5) * v3_length(ab);
  float e = v3_length(v3_max((vec3){x, y}, 0.0));
  float i = fmin(fmax(x, y), 0.0);

  return e + i;
}

typedef struct hit {
  float dist;
  int obj;
} hit;

typedef struct rgb {
  uint8_t r, g, b;
} rgb;

#define BLACK (rgb){0, 0, 0}
#define WHITE (rgb){255, 255, 255}

typedef struct pixel {
  /* surface properties */
  int obj;
  vec3 uv;
  rgb color;
  /* illumination data */
  float shade; /* -1.0 black, 0.0 neutral, 1.0 white */
} pixel;

#define NOPIXEL (pixel){.obj = -1}

static rgb texture(SDL_Surface *img, vec3 uv) {
  Uint32 *tex = (Uint32 *)img->pixels;
  int u = (uv.x - floorf(uv.x)) * img->w;
  int v = (uv.y - floorf(uv.y)) * img->h;
  Uint32 c = tex[v * img->w + u];
  return (rgb){(c >> 24) & 0xff, (c >> 16) & 0xff, (c >> 8) & 255};
}

/* Signed distance function */
static hit GetDist(vec3 p);

static hit RayMarch(vec3 ro, vec3 rd) {
  // distance from origin
  float dO = 0.0;
  hit h;

  for (int i = 0; i < MAX_STEPS; i++) {
    vec3 p = v3_add(ro, v3_mul(rd, dO));
    // distance to the scene
    h = GetDist(p);
    if (h.dist < SURF_DIST)
      return (hit){dO, h.obj};
    dO += h.dist;
    // prevent ray from escaping the scene
    if (dO > MAX_DIST)
      return (hit){INFINITY, -1};
  }

  SDL_Log("Too many steps!\n");

  return (hit){dO, h.obj}; 
}

static vec3 GetNormal(vec3 p) {
  float d = GetDist(p).dist;
  vec3 e = (vec3){0.01, 0.0};
  vec3 n = (vec3){d - GetDist(v3_sub(p, (vec3){e.x, e.y, e.y})).dist,
                  d - GetDist(v3_sub(p, (vec3){e.y, e.x, e.y})).dist,
                  d - GetDist(v3_sub(p, (vec3){e.y, e.y, e.x})).dist};
  return v3_normalize(n);
}

static __unused float GetLight(vec3 p, vec3 lp) {
  vec3 l = v3_normalize(v3_sub(lp, p));
  vec3 n = GetNormal(p);
  vec3 ro = v3_add(p, v3_mul(n, SURF_DIST));

  float d = RayMarch(ro, l).dist;

  float diff = v3_dot(n, l);
  if (d < v3_length(lp))
    diff *= 0.1;

  return clamp(diff, 0.0, 1.0);
}

/* source: https://stackoverflow.com/questions/13488957/interpolate-from-one-color-to-another */

/**
 * convert a single default RGB (a.k.a. sRGB) channel to linear RGB
 * @param channel R, G or B channel
 * @return channel in linear RGB
 */
static float rgb2linear(uint8_t channel) {
  float s = channel / 255.0f;
  return s <= 0.04045 ? s / 12.92 : powf((s + 0.055) / 1.055, 2.4);
}

/**
 * convert a linear RGB channel to default RGB channel (a.k.a. sRGB)
 * @param linear R, G or B channel in linear format
 * @return converted channel to default RGB value
 */
static uint8_t linear2rgb(float linear) {
  float s = linear <= 0.0031308 ? linear * 12.92
                                : 1.055 * powf(linear, 1.0 / 2.4) - 0.055;
  return (uint8_t)(s * 255);
}

static rgb rgblerp(rgb c1, rgb c2, float frac) {
  float r1 = rgb2linear(c1.r);
  float r2 = rgb2linear(c2.r);
  float g1 = rgb2linear(c1.g);
  float g2 = rgb2linear(c2.g);
  float b1 = rgb2linear(c1.b);
  float b2 = rgb2linear(c2.b);

  return (rgb){
    .r = linear2rgb((r2 - r1) * frac + r1),
    .g = linear2rgb((g2 - g1) * frac + g1),
    .b = linear2rgb((b2 - b1) * frac + b1),
  };
}

/* Functions that need to be defined by particular scene */
static void Load(void);
static pixel GetPixel(vec3 uv);

static void Render(SDL_Surface *canvas) {
  Uint32 *buffer = (Uint32 *)canvas->pixels;

  uint32_t start = SDL_GetTicks();

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      // Normalized pixel coordinates (from -1.0 to 1.0)
      vec3 uv = (vec3){
        (2.0f * (float)x - WIDTH) / HEIGHT, 1.0f - (float)y / HEIGHT * 2.0f};

      pixel px = GetPixel(uv);

      if (px.obj == -1) {
        // SDL_Log("Missed the surface at (%d,%d)!", x, y);
        buffer[y * WIDTH + x] = SDL_MapRGBA(canvas->format, 0, 0, 0, 255);
        continue;
      }

      rgb color = iChannel[px.obj] ? texture(iChannel[px.obj], px.uv) : px.color;

      {
        float shade = clamp(px.shade, 0.0, 1.0);

        if (shade < 0.5f)
          color = rgblerp(BLACK, color, shade * 2.0f);
        if (shade > 0.5f)
          color = rgblerp(color, WHITE, shade * 2.0f - 1.0f);
      }

      buffer[y * WIDTH + x] = SDL_MapRGBA(canvas->format, color.r, color.g, color.b, 255);
    }
  }

  uint32_t end = SDL_GetTicks();

  SDL_Log("Render took %dms\n", end - start);
}

static __unused void LoadTexture(int i, const char *path) {
  SDL_Surface *loaded;
  SDL_Surface *native;
  SDL_PixelFormat *pixelfmt;

  SDL_assert(i >= 0 && i <= NCHANNELS);

  if (!(loaded = IMG_Load(path))) {
    SDL_Log("Unable to load image %s! SDL_image Error: %s\n",
            path, IMG_GetError());
    exit(EXIT_FAILURE);
  }

  pixelfmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
  native = SDL_ConvertSurface(loaded, pixelfmt, 0);
  SDL_FreeFormat(pixelfmt);
  SDL_FreeSurface(loaded);
  iChannel[i] = native;
}

static void PerFrame(void) {
  static float iTimeStart;
  static bool iTimeFirst = true;

  if (iTimeFirst) {
    iTimeStart = SDL_GetTicks() / 1000.0f;
    iTimeFirst = false;
  }

  iTime = SDL_GetTicks() / 1000.0 - iTimeStart;
}

int main(void) {
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG);

  SDL_Window *window =
    SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                     WIDTH, HEIGHT, 0);

  SDL_Surface *window_surface = SDL_GetWindowSurface(window);

  SDL_Surface *canvas = SDL_CreateRGBSurfaceWithFormat(
    0, WIDTH, HEIGHT, 32, SDL_PIXELFORMAT_RGBA8888);

  Load();

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
    for (int i = 0; i < NCHANNELS; i++) {
      if (iChannel[i])
        SDL_LockSurface(iChannel[i]);
    }
    Render(canvas);
    for (int i = 0; i < NCHANNELS; i++) {
      if (iChannel[i])
        SDL_UnlockSurface(iChannel[i]);
    }
    SDL_UnlockSurface(canvas);

    SDL_BlitSurface(canvas, 0, window_surface, 0);
    SDL_UpdateWindowSurface(window);

    SDL_Delay(10);
  }

  for (int i = 0; i < NCHANNELS; i++) {
    if (iChannel[i])
      SDL_FreeSurface(iChannel[i]);
  }
  SDL_FreeSurface(canvas);
  IMG_Quit();
  SDL_Quit();

  return 0;
}
