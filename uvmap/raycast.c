#include "std/math.h"
#include "std/fastmath.h"
#include "uvmap/raycast.h"

void RaycastTunnel(UVMapT *map, Vector3D *view) {
  Vector3D ray = view[0];
  Vector3D dp = view[1];
  Vector3D dq = view[2];

  size_t h = map->height;
  size_t i = 0;

  V3D_Scale(&dp, &view[1], 1.0f / (int)(map->width - 1));
  V3D_Scale(&dq, &view[2], 1.0f / (int)(map->height - 1));

  do {
    Vector3D leftRay = ray;
    size_t w = map->width;

    do {
      float t = FastInvSqrt(ray.x * ray.x + ray.y * ray.y);

      Vector3D intersection = { t * ray.x, t * ray.y, t * ray.z };

      float a = FastAtan2(intersection.x, intersection.y);
      float u = a / (2 * M_PI);
      float v = intersection.z / 8.0f;

      UVMapSet(map, i++, u, v);

      V3D_Add(&ray, &ray, &dp);
    } while (--w);

    V3D_Add(&ray, &leftRay, &dq);
  } while (--h);
}
