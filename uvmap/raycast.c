#include "std/debug.h"
#include "std/math.h"
#include "std/fastmath.h"
#include "uvmap/raycast.h"

static __regargs
void RaycastTunnelLine(Q16T *umap, Q16T *vmap, size_t w,
                       float x, float y, float z, Vector3D *dp)
{
  do {
    float t = FastInvSqrt(x * x + y * y);
    float a = FastAtan2(t * x, t * y);
    float u = a / (2.0f * M_PI);
    float v = t * z / 8.0f;

    int uu = lroundf(u * 65536.0f * 256.0f);
    int vv = lroundf(v * 65536.0f * 256.0f);

    *umap++ = AsQ16(uu);
    *vmap++ = AsQ16(vv);

    x += dp->x;
    y += dp->y;
    z += dp->z;
  } while (--w);
}

void RaycastTunnel(UVMapT *map, Vector3D *view) {
  Vector3D ray = view[0];
  Vector3D dp = view[1];
  Vector3D dq = view[2];

  size_t h = map->height;

  Q16T *umap = map->map.accurate.u;
  Q16T *vmap = map->map.accurate.v;

  ASSERT((map->type == UV_ACCURATE) && (map->textureW == 256) &&
         (map->textureH == 256),
         "Accurate UV map with texture size of (256, 256) required.");

  V3D_Scale(&dp, &view[1], 1.0f / (int)(map->width - 1));
  V3D_Scale(&dq, &view[2], 1.0f / (int)(map->height - 1));

  do {
    RaycastTunnelLine(umap, vmap, map->width, ray.x, ray.y, ray.z, &dp);

    umap += map->width;
    vmap += map->width;

    ray.x += dq.x;
    ray.y += dq.y;
    ray.z += dq.z;
  } while (--h);
}
