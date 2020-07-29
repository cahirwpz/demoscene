#ifndef __CINTER_H__
#define __CINTER_H__

#define CINTER_DEGREES 16384

typedef struct CinterChannel {
  int state;
  void *sample;
  u_short period;
  u_short volume;
} CinterChannelT;

typedef struct CinterPlayer {
  int c_SampleState[3];
  short c_PeriodTable[36];
  short c_TrackSize;
  void *c_InstPointer;
  u_short *c_MusicPointer;
  u_short *c_MusicEnd;
  u_short *c_MusicLoop;
  CinterChannelT c_MusicState[3];
  short c_dma;
  short c_waitline;
  void *c_Instruments[32*2];
  short c_Sinus[CINTER_DEGREES];
  int c_fix[3]; /* those are getting trashed by CinterInit */
} CinterPlayerT;

void CinterInit(void *music asm("a2"), void *instruments asm("a4"), 
                CinterPlayerT *player asm("a6"));
void CinterPlay1(CinterPlayerT *player asm("a6"));
void CinterPlay2(CinterPlayerT *player asm("a6"));

#endif 
