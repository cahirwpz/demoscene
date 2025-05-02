#define NAME "donut"
#include "raymarch.h"

static mat4 tm;

static void Load(void) {
}

static hit GetDist(vec3 p) {
  return (hit){
    .dist = TorusDist(m4_translate(p, tm), 2.0, 0.666),
    .obj = 0};
}

static pixel GetPixel(vec3 uv) {
  // Determine torus origin & rotation, then create transformation matrix
  mat4 to = m4_move(0, 0, 6);
  mat4 tr = m4_rotate(iTime, iTime, 0.);
  tm = m4_mul(tr, to);

  // Determine light position
  vec3 lp = (vec3){0.0, 5.0, 6.0};
  lp.x += sinf(iTime) * 2.0;
  lp.z += cosf(iTime) * 2.0;

  // Simple camera model
  vec3 ro = (vec3){0.0, 4.0, 0.0};
  vec3 rd = v3_normalize((vec3){uv.x, uv.y - 0.5, 1.0});

  // Hit the object
  hit h = RayMarch(ro, rd);

  if (h.obj < 0)
    return NOPIXEL;

  // Position at with the object was hit
  vec3 p = v3_add(ro, v3_mul(rd, h.dist));

  float diff = GetLight(p, lp);

  return (pixel){
    .obj = 0,
    .color = (rgb){128, 128, 128},
    .shade = diff,
  };
}
