#include <clib/exec_protos.h>

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

RSC_TYPE(whelpz_img, PixBufT *)
RSC_ADD(whelpz_img, NewPixBufFromFile("data/whelpz.8", MEMF_PUBLIC))
RSC_FREE(whelpz_img, DeletePixBuf)
#define whelpz_img_init NULL

RSC_TYPE(whelpz_pal, PaletteT *)
RSC_ADD(whelpz_pal, NewPaletteFromFile("data/whelpz.pal", MEMF_PUBLIC))
RSC_FREE(whelpz_pal, DeletePalette)
#define whelpz_pal_init NULL

RSC_TYPE(tunnel_map, struct DistortionMap *)
RSC_ADD(tunnel_map, NewDistortionMap(WIDTH, HEIGHT))
RSC_FREE(tunnel_map, DeleteDistortionMap)
RSC_INIT(tunnel_map) {
  GenerateTunnel(tunnel_map, 8192, WIDTH/2, HEIGHT/2);
  return TRUE;
}

RSC_START
RSC_STD(module)
RSC(txt_img)
RSC(txt_pal)
RSC(code_img)
RSC(code_pal)
RSC(whelpz_img)
RSC(whelpz_pal)
RSC(tunnel_map)
RSC_END
