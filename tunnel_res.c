#include <clib/exec_protos.h>

#include "gfx/common.h"
#include "std/resource_internal.h"
#include "system/fileio.h"
#include "distortion.h"

#define RSC_FILE(NAME, FILENAME, MEMTYPE) \
  RSC_ADD(NAME, ReadFileSimple(FILENAME, MEMTYPE))

#define WIDTH 320
#define HEIGHT 256

RSC_TYPE(module, void *)
RSC_FILE(module, "data/tempest-acidjazzed_evening.p61", MEMF_CHIP)

RSC_TYPE(texture, PixBufT *)
RSC_ADD(texture, NewPixBufFromFile("data/texture-01.8", MEMF_PUBLIC))
RSC_FREE(texture, DeletePixBuf)
#define texture_init NULL

RSC_TYPE(palette, uint8_t *)
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
RSC(texture)
RSC_STD(palette)
RSC_CONST(cross)
RSC(tunnel_map)
RSC_END
