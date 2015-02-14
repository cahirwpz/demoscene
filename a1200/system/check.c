#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include "system/check.h"

bool SystemCheck() {
  bool kickv40 = SysBase->LibNode.lib_Version >= 40;
  bool chipaga = GfxBase->ChipRevBits0 & (GFXF_AA_ALICE|GFXF_AA_LISA);
  bool cpu68060 = SysBase->AttnFlags & AFF_68060;
  bool fpu68882 = SysBase->AttnFlags & AFF_68882;

  Printf("System check:\r\n");
  Printf(" - Kickstart v40 : %s\r\n", (ULONG)(kickv40 ? "yes" : "no"));
  Printf(" - ChipSet AGA : %s\r\n", (ULONG)(chipaga ? "yes" : "no"));
  Printf(" - CPU 68060 : %s\r\n", (ULONG)(cpu68060 ? "yes" : "no"));
  Printf(" - FPU 68882 : %s\r\n", (ULONG)(fpu68882 ? "yes" : "no"));

  return (kickv40 && cpu68060 && fpu68882 && chipaga);
}
