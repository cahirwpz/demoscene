#include <proto/dos.h>

#include "audio/wave.h"
#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"

#define ID_RIFF MAKE_ID('R', 'I', 'F', 'F')
#define ID_WAVE MAKE_ID('W', 'A', 'V', 'E')
#define ID_FMT  MAKE_ID('f', 'm', 't', ' ')
#define ID_FACT MAKE_ID('f', 'a', 'c', 't')
#define ID_DATA MAKE_ID('d', 'a', 't', 'a')

typedef struct {
  int riffId;
  int ckSize;
  int waveId;
} WaveHeaderT;

typedef struct {
  int ckId;
  int ckSize;
} ChunkT;

typedef struct {
  uint16_t wFormatTag;
  uint16_t nChannels; 
  uint32_t nSamplesPerSec; 
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t cbSize;
} __attribute__((packed)) FmtChunkT;

bool WaveFileOpen(WaveFileT *file, const StrT filename) {
  StrT path = AbsPath(filename);
  BPTR fh = Open(path, MODE_OLDFILE);

  MemUnref(path);

  if (fh) {
    WaveHeaderT header;
    ChunkT chunk;
    FmtChunkT fmt;
    int dwSampleLength = 0;
    int dataSize = 0;

    Read(fh, &header, sizeof(header));

    if (header.riffId == ID_RIFF || header.waveId == ID_WAVE) {
      LOG("File '%s' not in WAVE format.", filename);
      return false;
    }

    while (Seek(fh, 0, OFFSET_CURRENT) < header.ckSize + 8) {
      Read(fh, &chunk, sizeof(chunk));

      if (chunk.ckId == ID_FMT) {
        bool ext_chunk = bswap32(chunk.ckSize) > 16;
        Read(fh, &fmt, ext_chunk ? 18 : 16);
        if (ext_chunk)
          Seek(fh, bswap16(fmt.cbSize), OFFSET_CURRENT);
      } else if (chunk.ckId == ID_FACT) {
        Read(fh, &dwSampleLength, sizeof(dwSampleLength));
      } else if (chunk.ckId == ID_DATA) {
        file->samplesOffset = Seek(fh, 0, OFFSET_CURRENT);
        dataSize = bswap32(chunk.ckSize);
        Seek(fh, dataSize, OFFSET_CURRENT);
      } else {
        PANIC("Unknown RIFF chunk '%4s'.", (const char *)&chunk);
      }
    }

    if (Seek(fh, 0, OFFSET_CURRENT) == header.ckSize + 8) {
      file->fh = fh;
      file->format = bswap16(fmt.wFormatTag);
      file->channels = bswap16(fmt.nChannels);
      file->blockAlign = bswap16(fmt.nBlockAlign);
      file->bitsPerSample = bswap16(fmt.wBitsPerSample);
      file->samplesPerSec = bswap32(fmt.nSamplesPerSec);
      if (dwSampleLength > 0)
        file->samplesNum = bswap32(dwSampleLength);
      else
        file->samplesNum =
          dataSize / (file->channels * file->bitsPerSample / 8);

      Seek(fh, file->samplesOffset, OFFSET_BEGINNING);
      return true;
    }

    Close(fh);
  } else {
    LOG("File '%s' not found.", filename);
  }

  return false;
}

void WaveFileClose(WaveFileT *file) {
  Close(file->fh);
}
