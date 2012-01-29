#include <clib/alib_stdio_protos.h>
#include <clib/graphics_protos.h>
#include <inline/graphics_protos.h>
#include <proto/graphics.h>

#include "frame_tools.h"

void RenderFrameNumber(int frameNumber, struct DBufRaster *raster) {
  struct RastPort rastPort;

  InitRastPort(&rastPort);
  rastPort.BitMap = raster->BitMap;
  SetDrMd(&rastPort, JAM1);

  char number[4];

  sprintf(number, "%04d", frameNumber);

  Move(&rastPort, 2, 8);
  Text(&rastPort, number, 4);
}
