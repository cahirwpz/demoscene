/*
 * Prints out information about contents of a list of WAVE files.
 */

#include <stdio.h>

#include "audio/wave.h"

void WaveFilePrint(WaveFileT *file, const char *path) {
  printf("file            : %s\n", path);
  printf("format          : %s\n", 
         (file->format == WAVE_PCM) ? "PCM" : "IMA-ADPCM");
  printf("channels        : %d\n", file->channels);
  printf("block align     : %d\n", file->blockAlign);
  printf("bits per sample : %d\n", file->bitsPerSample);
  printf("sampling freq   : %d\n", file->samplesPerSec);
  printf("samples         : %ld\n", file->samplesNum);
}

int main(int argc, char **argv) {
  int i;

  for (i = 1; i < argc; i++) {
    WaveFileT file;

    if (WaveFileOpen(&file, argv[i])) {
      WaveFilePrint(&file, argv[i]);
      WaveFileClose(&file);
    }
  }

  return 0;
}
