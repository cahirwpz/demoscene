#include <clib/exec_protos.h>

#include "system/resource_internal.h"
#include "system/fileio.h"
#include "gfx/common.h"
#include "distortion.h"

#define RSC_FILE(NAME, FILENAME, MEMTYPE) \
  RSC_ADD(NAME, ReadFileSimple(FILENAME, MEMTYPE))

#define WIDTH 320
#define HEIGHT 256

RSC_TYPE(module, APTR)
RSC_FILE(module, "data/tempest-acidjazzed_evening.p61", MEMF_CHIP)

RSC_TYPE(texture, APTR)
RSC_FILE(texture, "data/texture-01.raw", MEMF_PUBLIC)

RSC_TYPE(palette, APTR)
RSC_FILE(palette, "data/texture-01.pal", MEMF_PUBLIC)

RSC_TYPE(tunnel_map, struct DistortionMap *)
RSC_ADD(tunnel_map, NewDistortionMap(WIDTH, HEIGHT))
RSC_FREE(tunnel_map, DeleteDistortionMap)
RSC_INIT(tunnel_map) {
  GenerateTunnel(tunnel_map, 8192, WIDTH/2, HEIGHT/2);
  return TRUE;
}

RSC_TYPE(cross, PointT)
RSC_ARRAY(cross) = {
  {1, 0}, {2, 0}, {2, 1}, {3, 1}, {3, 2}, {2, 2}, {2, 3}, {1, 3}, {1, 2}, {0, 2}, {0, 1}, {1, 1}
};

RSC_START
RSC_STD(module)
RSC_STD(texture)
RSC_STD(palette)
RSC_CONST(cross)
RSC(tunnel_map)
RSC_END
