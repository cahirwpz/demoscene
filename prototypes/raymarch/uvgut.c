#define NAME "gut"
#include "raymarch.h"

static void Load(void) {
  LoadTexture(0, "data/texture-inside.png");
  LoadTexture(1, "data/texture-outside.png");
}

const float pipeOuterRadius = 2.0;
const float pipeInnerRadius = 0.6;

static float PipeInnerDist(vec3 p, float r) {
  return sqrtf(p.x * p.x + p.y * p.y) - r;
}

static float PipeOuterDist(vec3 p, float r) {
  return r - sqrtf(p.x * p.x + p.y * p.y);
}

static hit GetDist(vec3 p) {
  float p0 = PipeOuterDist(p, pipeOuterRadius);
  float p1 = PipeInnerDist(p, pipeInnerRadius);

  if (p0 < p1)
    return (hit){p0, 0};
  else
    return (hit){p1, 1};
}

static pixel GetPixel(vec3 uv) {
  // camera-to-world transformation
  vec3 lookat = (vec3){0.0, 0.0, -1.0};
  vec3 ro = (vec3){1.9, 0.0, -2.5};

  mat4 ca = m4_camera_lookat(ro, lookat, 0.);
  vec3 co = v3_normalize((vec3){uv.x, uv.y, 2.0});
  vec3 rd = m4_translate(co, ca);

  hit h = RayMarch(ro, rd);
  if (h.obj < 0)
    return NOPIXEL;

  pixel px = {.obj = h.obj, .shade = 0.5f};

  vec3 p = v3_add(ro, v3_mul(rd, h.dist));

  vec3 tuv = v3_mul((vec3){atan2(p.x, p.y), p.z}, .5 / M_PI);

  // determine texture uv coordinates for the surface
  if (h.obj == 0) {
    // Outer pipe
    tuv.x *= 2.;
    tuv.y *= 1.;
    px.uv = v3_add(tuv, (vec3){iTime * 0.1, iTime * 0.1});
  } else {
    // Inner pipe
    tuv.x *= 2.;
    tuv.y *= 4.;
    px.uv = v3_add(tuv, (vec3){iTime * 0.1, -iTime * 0.25});
  }

  px.uv = v3_mul(px.uv, 2.0f);

  return px;
}
