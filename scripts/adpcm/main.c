#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codec.h"
#include "endian.h"

enum
{
  RIFF_ID	= 'RIFF',
  WAVE_ID	= 'WAVE',
  FMT_ID	= 'fmt ',
  DATA_ID	= 'data'
};

typedef struct RIFFHeaderTag
{
  uint32_t	RiffId;
  uint32_t	Size;
  uint32_t	WaveId;
} RIFFHeader;

typedef struct FMTHeaderTag
{
  uint32_t	FmtId;
  uint32_t	ChunkSize;
  uint16_t	Format;
  uint16_t	NumChannels;
  uint32_t	SamplesPerSec;
  uint32_t	AvgBytesPerSec;
  uint16_t	BlockAlign;
  uint16_t	BitsPerSample;
} FMTHeader;

typedef struct DATAHeaderTag
{
  uint32_t	DataId;
  uint32_t	ChunkSize;
} DATAHeader;

bool adpcm_encode(char* inFileName, char* outFileName)
{
  FILE* f = fopen(inFileName, "rb");

  if (!f)
    return false;

  fseek(f, 0, SEEK_END);
  int inFileSize = ftell(f);
  fseek(f, 0, SEEK_SET);
  uint8_t* inFileBuf = (uint8_t*) malloc(inFileSize);
  fread(inFileBuf, 1, inFileSize, f);
  fclose(f);

  RIFFHeader* riffHeader = (RIFFHeader*) inFileBuf;

  if (endianReadU32Big(&riffHeader->RiffId) != RIFF_ID)
    return false;
  if (endianReadU32Big(&riffHeader->WaveId) != WAVE_ID)
    return false;

  int waveSize = endianReadU32Little(&riffHeader->Size) - 4;

  FMTHeader* fmtHeader = 0;
  DATAHeader* dataHeader = 0;

  uint8_t* data = (uint8_t*) (riffHeader + 1);
  while (waveSize && !(fmtHeader && dataHeader))
  {
    uint32_t id = endianReadU32Big((uint32_t*) data);
    if (id == FMT_ID)
      fmtHeader = (FMTHeader*) data;
    else if (id == DATA_ID)
      dataHeader = (DATAHeader*) data;

    uint32_t chunkSize = endianReadU32Little((uint32_t*) (data + 4));
    data += chunkSize + 8;
    waveSize -= chunkSize + 8;
  }

  if (!(fmtHeader && dataHeader))
    return false;

  if (endianReadU16Little(&fmtHeader->BitsPerSample) != 16 ||
      endianReadU16Little(&fmtHeader->NumChannels) != 1 ||
      endianReadU16Little(&fmtHeader->Format) != 1)
    return false;

  int16_t* samples = (int16_t*) (dataHeader + 1);
  uint32_t numSamples = endianReadU32Little(&dataHeader->ChunkSize) / 2;

  uint8_t* adpcmData = (uint8_t*) malloc(numSamples / 2 + 2);

  CodecState state;
  memset(&state, 0, sizeof(state));
  encode(&state, samples, numSamples, adpcmData);

  FILE* outputFile = fopen(outFileName, "wb");
  fwrite(adpcmData, 1, numSamples / 2, outputFile);
  fclose(outputFile);

  return true;
}

bool adpcm_decode(char* inFileName, char* outFileName)
{
  FILE* f = fopen(inFileName, "rb");

  if (!f)
    return false;

  fseek(f, 0, SEEK_END);
  int inFileSize = ftell(f);
  fseek(f, 0, SEEK_SET);
  uint8_t* inFileBuf = (uint8_t*) malloc(inFileSize);
  fread(inFileBuf, 1, inFileSize, f);
  fclose(f);

  int numSamples = inFileSize * 2;

  int outBufferSize = sizeof(RIFFHeader) + sizeof(FMTHeader) + sizeof(DATAHeader) + numSamples * 2;

  uint8_t* outFileBuf = (uint8_t*) malloc(outBufferSize);

  RIFFHeader* riffHeader = (RIFFHeader*) outFileBuf;

  endianWriteU32Big(&riffHeader->RiffId, RIFF_ID);
  endianWriteU32Little(&riffHeader->Size, outBufferSize - sizeof(RIFFHeader) + 4);
  endianWriteU32Big(&riffHeader->WaveId, WAVE_ID);

  FMTHeader* fmtHeader = (FMTHeader*) (riffHeader + 1);

  endianWriteU32Big(&fmtHeader->FmtId, FMT_ID);
  endianWriteU32Little(&fmtHeader->ChunkSize, sizeof(FMTHeader) - 8);
  endianWriteU16Little(&fmtHeader->Format, 1);
  endianWriteU16Little(&fmtHeader->NumChannels, 1);
  endianWriteU32Little(&fmtHeader->SamplesPerSec, 44100);
  endianWriteU32Little(&fmtHeader->AvgBytesPerSec, 44100 * 2);
  endianWriteU16Little(&fmtHeader->BlockAlign, 2);
  endianWriteU16Little(&fmtHeader->BitsPerSample, 16);

  DATAHeader* dataHeader = (DATAHeader*) (fmtHeader + 1);

  endianWriteU32Big(&dataHeader->DataId, DATA_ID);
  endianWriteU32Little(&dataHeader->ChunkSize, numSamples * 2);

  CodecState state;
  memset(&state, 0, sizeof(state));
  decode68000(&state, inFileBuf, numSamples, (int16_t*) (dataHeader + 1));

  FILE* outputFile = fopen(outFileName, "wb");
  fwrite(outFileBuf, 1, outBufferSize, outputFile);
  fclose(outputFile);

  return true;
}

int main(int argc, char** argv)
{
  if (argc != 4)
  {
    printf("Usage: <encode|decode> <input file> <output file>\n");
    return 0;
  }

  initDecode68000();

  if (!strcasecmp(argv[1], "encode"))
  {
    bool success = adpcm_encode(argv[2], argv[3]);
    printf("%s\n", success ? "success" : "fail");
  }
  else if (!strcasecmp(argv[1], "decode"))
  {
    bool success = adpcm_decode(argv[2], argv[3]);
    printf("%s\n", success ? "success" : "fail");
  }
  else
  {
    printf("bad args\n");
  }

  return 0;
}
