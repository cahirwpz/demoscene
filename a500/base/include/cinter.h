#ifndef __CINTER_H__
#define __CINTER_H__

#define CINTER_DEGREES 16384

typedef struct CinterChannel {
  LONG state;
  APTR sample;
  UWORD period;
  UWORD volume;
} CinterChannelT;

typedef struct CinterPlayer {
  LONG c_SampleState[3];
  WORD c_PeriodTable[36];
  WORD c_TrackSize;
  APTR c_InstPointer;
  UWORD *c_MusicPointer;
  UWORD *c_MusicEnd;
  UWORD *c_MusicLoop;
  CinterChannelT c_MusicState[3];
  WORD c_dma;
  WORD c_waitline;
  APTR c_Instruments[32*2];
  WORD c_Sinus[CINTER_DEGREES];
  LONG c_fix[3]; /* those are getting trashed by CinterInit */
} CinterPlayerT;

void CinterInit(APTR music asm("a2"), APTR instruments asm("a4"), 
                CinterPlayerT *player asm("a6"));
void CinterPlay1(CinterPlayerT *player asm("a6"));
void CinterPlay2(CinterPlayerT *player asm("a6"));

#endif 
