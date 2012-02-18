#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>

#include "system/check.h"

bool SystemCheck() {
  bool kickv40 = SysBase->LibNode.lib_Version >= 40;
  bool chipaga = GfxBase->ChipRevBits0 & (GFXF_AA_ALICE|GFXF_AA_LISA);
  bool cpu68040 = SysBase->AttnFlags & AFF_68040;
  bool fpu68882 = SysBase->AttnFlags & AFF_68882;

  Printf("System check:\n");
  Printf(" - Kickstart v40 : %s\n", (ULONG)(kickv40 ? "yes" : "no"));
  Printf(" - ChipSet AGA : %s\n", (ULONG)(chipaga ? "yes" : "no"));
  Printf(" - CPU 68040 : %s\n", (ULONG)(cpu68040 ? "yes" : "no"));
  Printf(" - FPU 68882 : %s\n", (ULONG)(fpu68882 ? "yes" : "no"));

  return (kickv40 && cpu68040 && fpu68882 && chipaga);
}
