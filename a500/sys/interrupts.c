#include "interrupts.h"

APTR OldIntLevels[7];

void SaveInterrupts() {
  memcpy(OldIntLevels, &InterruptVector->IntLevel1, sizeof(APTR) * 7);
}

void RestoreInterrupts() {
  memcpy(&InterruptVector->IntLevel1, OldIntLevels, sizeof(APTR) * 7);
}
