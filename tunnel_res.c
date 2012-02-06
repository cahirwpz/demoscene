#include <clib/exec_protos.h>

#include "gfx/common.h"
#include "gfx/pixbuf.h"
#include "gfx/palette.h"
#include "std/resource_internal.h"
#include "system/fileio.h"
#include "distortion.h"

#define RSC_FILE(NAME, FILENAME, MEMTYPE) \
  RSC_ADD(NAME, ReadFileSimple(FILENAME, MEMTYPE))

#define WIDTH 320
#define HEIGHT 256

RSC_TYPE(module, void *)
RSC_FILE(module, "data/tempest-acidjazzed_evening.p61", MEMF_CHIP)

RSC_TYPE(txt_img, PixBufT *)
RSC_ADD(txt_img, NewPixBufFromFile("data/texture-01.8", MEMF_PUBLIC))
RSC_FREE(txt_img, DeletePixBuf)
#define txt_img_init NULL

RSC_TYPE(txt_pal, PaletteT *)
RSC_ADD(txt_pal, NewPaletteFromFile("data/texture-01.pal", MEMF_PUBLIC))
RSC_FREE(txt_pal, DeletePalette)
#define txt_pal_init NULL

RSC_TYPE(code_img, PixBufT *)
RSC_ADD(code_img, NewPixBufFromFile("data/code.8", MEMF_PUBLIC))
RSC_FREE(code_img, DeletePixBuf)
#define code_img_init NULL

RSC_TYPE(code_pal, PaletteT *)
RSC_ADD(code_pal, NewPaletteFromFile("data/code.pal", MEMF_PUBLIC))
RSC_FREE(code_pal, DeletePalette)
#define code_pal_init NULL

RSC_TYPE(music_img, PixBufT *)
RSC_ADD(music_img, NewPixBufFromFile("data/music.8", MEMF_PUBLIC))
RSC_FREE(music_img, DeletePixBuf)
#define music_img_init NULL

RSC_TYPE(music_pal, PaletteT *)
RSC_ADD(music_pal, NewPaletteFromFile("data/music.pal", MEMF_PUBLIC))
RSC_FREE(music_pal, DeletePalette)
#define music_pal_init NULL

RSC_TYPE(anniversary_img, PixBufT *)
RSC_ADD(anniversary_img, NewPixBufFromFile("data/anniversary.8", MEMF_PUBLIC))
RSC_FREE(anniversary_img, DeletePixBuf)
#define anniversary_img_init NULL

RSC_TYPE(anniversary_pal, PaletteT *)
RSC_ADD(anniversary_pal, NewPaletteFromFile("data/anniversary.pal", MEMF_PUBLIC))
RSC_FREE(anniversary_pal, DeletePalette)
#define anniversary_pal_init NULL

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
RSC(txt_img)
RSC(txt_pal)
RSC(code_img)
RSC(code_pal)
RSC(music_img)
RSC(music_pal)
RSC(anniversary_img)
RSC(anniversary_pal)
RSC_CONST(cross)
RSC(tunnel_map)
RSC_END
