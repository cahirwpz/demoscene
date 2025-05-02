#define NAME "scene"
#include "raymarch.h"

static mat4 bm, tm, sm;

static void Load(void) {
}

static hit GetDist(vec3 p) {
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

  return (hit){
    .dist = fmin(fmin(fmin(ccd, bd), fmin(sd, td)), fmin(pd, cd)),
    .obj = 0};
}

static pixel GetPixel(vec3 uv) {
  // Determine scene objects origin & rotation, then create transformation matrices
  sm = m4_move(0, 1, 5);

  mat4 to = m4_move(-3, 1, 6);
  mat4 tr = m4_rotate(iTime, 0., 0.);
  tm = m4_mul(tr, to);

  mat4 bo = m4_move(0, 1, 8);
  mat4 br = m4_rotate(0., iTime * .2, 0.);
  bm = m4_mul(br, bo);

  // Determine light position
  vec3 lp = (vec3){0.0, 5.0, 6.0};
  lp.x += sinf(iTime) * 2.0;
  lp.z += cosf(iTime) * 2.0;

  // Simple camera model
  vec3 ro = (vec3){0.0, 4.0, 0.0};
  vec3 rd = v3_normalize((vec3){uv.x, uv.y - 0.5, 1.0});

  // Hit the object
  hit h = RayMarch(GetDist, ro, rd);

  if (h.obj < 0)
    return NOPIXEL;

  // Position at with the object was hit
  vec3 p = v3_add(ro, v3_mul(rd, h.dist));

  float diff = GetLight(GetDist, p, lp);

  return (pixel){
    .obj = 0,
    .color = (rgb){128, 128, 128},
    .shade = diff,
  };
}
