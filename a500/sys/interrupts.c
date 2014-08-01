#include <proto/exec.h>

#include "interrupts.h"

APTR OldIntLevels[7];

void SaveInterrupts() {
  CopyMem(&InterruptVector->IntLevel1, OldIntLevels, sizeof(APTR) * 7);
}

void RestoreInterrupts() {
  CopyMem(OldIntLevels, &InterruptVector->IntLevel1, sizeof(APTR) * 7);
}
