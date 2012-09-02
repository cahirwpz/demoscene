#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "audio/stream.h"
#include "std/debug.h"
#include "std/memory.h"

int main() {
  AudioStreamT *audio = AudioStreamOpen("data/chembro.snd");

  if (AudioStreamPlay(audio)) {
    uint32_t ready = 1L << AudioStreamGetSignal(audio);
    uint32_t signals;

    do {
      Write(Output(), ".", 1);

      signals = Wait(ready | SIGBREAKF_CTRL_C);
     
      if (signals & SIGBREAKF_CTRL_C)
        break;

    } while (AudioStreamFeed(audio));

    if (signals & SIGBREAKF_CTRL_C) {
      LOG("***Break\n");
    } else {
      Wait(ready);
    }

    AudioStreamStop(audio);
  }

  MemUnref(audio);
}
