#include "audio/wave.h"
#include "std/debug.h"
#include "std/memory.h"

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

bool WaveFileOpen(WaveFileT *wave, const char *filename) {
  RwOpsT *file = RwOpsFromFile(filename, "r");

  if (file) {
    WaveHeaderT header;
    ChunkT chunk;
    FmtChunkT fmt;
    int dwSampleLength = 0;
    int dataSize = 0;

    IoRead(file, &header, sizeof(header));
    header.ckSize = bswap32(header.ckSize);

    if (header.riffId == ID_RIFF && header.waveId == ID_WAVE) {
      while (IoRead(file, &chunk, sizeof(chunk)) > 0) {
        if (chunk.ckId == ID_FMT) {
          bool ext_chunk = bswap32(chunk.ckSize) > 16;
          IoRead(file, &fmt, ext_chunk ? 18 : 16);
          if (ext_chunk)
            IoSeek(file, bswap16(fmt.cbSize), IO_SEEK_CUR);
        } else if (chunk.ckId == ID_FACT) {
          IoReadLE32(file, &dwSampleLength);
        } else if (chunk.ckId == ID_DATA) {
          wave->samplesOffset = IoTell(file);
          dataSize = bswap32(chunk.ckSize);
          IoSeek(file, dataSize, IO_SEEK_CUR);
        } else {
          PANIC("Unknown RIFF chunk '%4s'.", (const char *)&chunk);
        }
      }

      if (IoTell(file) == header.ckSize + 8) {
        wave->file = file;
        wave->format = bswap16(fmt.wFormatTag);
        wave->channels = bswap16(fmt.nChannels);
        wave->blockAlign = bswap16(fmt.nBlockAlign);
        wave->bitsPerSample = bswap16(fmt.wBitsPerSample);
        wave->samplesPerSec = bswap32(fmt.nSamplesPerSec);

        if (dwSampleLength > 0)
          wave->samplesNum = dwSampleLength;
        else
          wave->samplesNum =
            dataSize / (wave->channels * wave->bitsPerSample / 8);

        LOG("File '%s' - fmt: %d, chs: %d, bits: %d, freq: %d.", filename,
            wave->format, wave->channels, wave->bitsPerSample,
            wave->samplesPerSec);

        IoSeek(file, wave->samplesOffset, IO_SEEK_SET);
        return true;
      }
    } else {
      LOG("File '%s' not in WAVE format.", filename);
    }

    IoClose(file);
  } else {
    LOG("File '%s' not found.", filename);
  }

  return false;
}

void WaveFileClose(WaveFileT *wave) {
  IoClose(wave->file);
}

void WaveFileChangePosition(WaveFileT *wave, float second) {
  size_t sampleWidth = wave->channels * wave->bitsPerSample / 8;
  size_t offset = wave->samplesOffset +
    (size_t)(wave->samplesPerSec * second) * sampleWidth;

  IoSeek(wave->file, offset, IO_SEEK_SET);
}

size_t WaveFileReadSamples(WaveFileT *wave, PtrT samples, size_t requested) {
  int sampleWidth = wave->channels * wave->bitsPerSample / 8;
  int obtained;

  requested *= sampleWidth;
  obtained = IoRead(wave->file, samples, requested);

  /* Clear rest of buffer if needed. */
  if (obtained < requested)
    memset(samples + obtained, 0, requested - obtained);

  return obtained / sampleWidth;
}
