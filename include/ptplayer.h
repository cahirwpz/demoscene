#ifndef PTPLAYER_H
#define PTPLAYER_H

#include "types.h"

/*
 * Install a CIA-B interrupt for calling _mt_music.
 *
 * The music module is replayed via _mt_music when _mt_Enable is non-zero.
 */
void PtInstallCIA(void);

/*
 * Remove the CIA-B music interrupt and restore the old vector.
 */
void PtRemoveCIA(void);

/*
 * Initialize a new module.
 *
 * Reset speed to 6, tempo to 125 and start at the given position. Master
 * volume is at 64 (maximum). When `Samples` is NULL the samples are assumed
 * to be stored after the patterns.
 */
void PtInit(void *TrackerModule, void *Samples, u_char InitialSongPos);

/*
 * Stop playing current module.
 */
void PtEnd(void);

/*
 * PtEnable: Set this byte to non-zero to play music, zero to pause playing.
 *
 * Note that you can still play sound effects, while music is stopped.
 * It is set to 0 by PtInstallCIA().
 */
extern u_char PtEnable;

/*
 * PtE8Trigger: This byte reflects the value of the last E8 command.
 *  It is reset to 0 after PtInit().
 */
extern u_char PtE8Trigger;

typedef struct {
  u_char name[22];
  u_short size;
  u_char finetune;
  u_char volume;
  u_short loop_start;
  u_short loop_size;
} __attribute__((packed)) PtSample;

typedef struct {
  u_char name[20];
  PtSample sample[31];
  u_char len;
  u_char restart;
  u_char order[128];
  u_char magic[4];
  u_int pattern[0][64][4];
} __attribute__((packed)) PtModule;

typedef struct {
  u_short n_note;
  u_char n_cmd;
  u_char n_cmdlo;
  u_char n_index;
  u_char n_sfxpri;
  u_short n_reserved1;
  void *n_start;
  void *n_loopstart;
  u_short n_length;
  u_short n_replen;
  u_short n_period;
  u_short n_volume;
  void *n_pertab;
  u_short n_dmabit;
  u_short n_noteoff;
  u_short n_toneportspeed;
  u_short n_wantedperiod;
  u_short n_pattpos;
  u_short n_funk;
  void *n_wavestart;
  u_short n_reallength;
  u_short n_intbit;
  void *n_sfxptr;
  u_short n_sfxlen;
  u_short n_sfxper;
  u_short n_sfxvol;
  u_char n_looped;
  u_char n_minusft;
  u_char n_vibratoamp;
  u_char n_vibratospd;
  u_char n_vibratopos;
  u_char n_vibratoctrl;
  u_char n_tremoloamp;
  u_char n_tremolospd;
  u_char n_tremolopos;
  u_char n_tremoloctrl;
  u_char n_gliss;
  u_char n_sampleoffset;
  u_char n_loopcount;
  u_char n_funkoffset;
  u_char n_retrigcount;
  u_char n_freecnt;
  u_char n_musiconly;
  u_char n_enable;
} __attribute__((packed)) PtChannel;

typedef struct {
  PtChannel mt_chan[4];
  void *mt_SampleStarts[31];
  PtModule *mt_mod;
  void *mt_oldLev6;
  void *mt_timerval;
  u_char mt_oldtimers[4];
  void *mt_Lev6Int;
  u_short mt_Lev6Ena;
  u_short mt_PatternPos;
  u_short mt_PBreakPos;
  u_char mt_PosJumpFlag;
  u_char mt_PBreakFlag;
  u_char mt_Speed;
  u_char mt_Counter;
  u_char mt_SongPos;
  u_char mt_PattDelTime;
  u_char mt_PattDelTime2;
} __attribute__((packed)) PtPlayer;

extern PtPlayer PtData;

#endif
