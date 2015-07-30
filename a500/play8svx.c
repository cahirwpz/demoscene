#include <exec/execbase.h>
#include <proto/exec.h>

#include "interrupts.h"
#include "hardware.h"
#include "sound.h"
#include "8svx.h"

extern char *__commandline;

int __nocommandline = 1;
ULONG __oslibversion = 33;

int main() {
  UWORD len = strlen(__commandline);
  STRPTR filename = __builtin_alloca(len);

  memcpy(filename, __commandline, len--);
  filename[len] = '\0';

  /* Get Vector Base Register */
  if (SysBase->AttnFlags & AFF_68010)
    InterruptVector = (APTR)Supervisor((APTR)GetVBR);

  {
    static SoundT *sound;

    if ((sound = Load8SVX(filename))) {
      Forbid();

      custom->dmacon = DMAF_COPPER | DMAF_RASTER;

      /* 7kHz cut-off filter turn off. */
      ciaa->ciapra |= CIAF_LED;

      AudioSetVolume(CHAN_0, 64);
      AudioSetVolume(CHAN_2, 64);
      AudioAttachSound(CHAN_0, sound);
      AudioAttachSound(CHAN_2, sound);
      AudioPlay(CHAN_0);
      AudioPlay(CHAN_2);
      WaitMouse();
      AudioStop(CHAN_0);
      AudioStop(CHAN_2);

      custom->dmacon = DMAF_COPPER | DMAF_RASTER | DMAF_SETCLR;

      /* 7kHz cut-off filter turn on. */
      ciaa->ciapra &= ~CIAF_LED;

      Permit();

      DeleteSound(sound);
    }
  }

  return 0;
}
