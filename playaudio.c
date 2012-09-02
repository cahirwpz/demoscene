#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "audio/stream.h"
#include "std/debug.h"
#include "std/memory.h"

int main() {
  AudioStreamT *audio = AudioStreamOpen("data/chembro.snd");

  if (AudioStreamPlay(audio)) {
    uint32_t signals;

    do {
      Write(Output(), ".", 1);

      signals = AudioStreamHungryWait(audio, SIGBREAKF_CTRL_C);
    } while (!(signals & SIGBREAKF_CTRL_C) && AudioStreamFeed(audio));

    if (signals & SIGBREAKF_CTRL_C) {
      LOG("***Break\n");
    } else {
      AudioStreamHungryWait(audio, 0);
    }

    AudioStreamStop(audio);
  }

  MemUnref(audio);
}
