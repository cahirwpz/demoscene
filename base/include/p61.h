#ifndef __P61_H__
#define __P61_H__

#include "types.h"

extern u_short P61_visuctr[4];

typedef struct {
	void *P61_SampleOffset;
	u_short P61_SampleLength;
	void *P61_RepeatOffset;
	u_short P61_RepeatLength;
	u_short P61_SampleVolume;
	u_short P61_FineTune;
} __attribute__((packed)) P61_SampleBlock;

typedef struct {
	u_char SN_Note;
	u_char Command;
	u_char Info;
	u_char Pack;
	P61_SampleBlock *Sample;
	u_short OnOff;
	void *ChaPos;
	void *TempPos;
	u_short TempLen;

	u_short Note;
	u_short Period;
	u_short Volume;
	u_short Fine;

	u_short Offset;
	u_short LOffset;
	u_short ToPeriod;
	u_short TPSpeed;
	u_char VibCmd;
	u_char VibPos;
	u_char TreCmd;
	u_char TrePos;
	u_short RetrigCount;
	u_char Funkspd;
	u_char Funkoff;
	void *Wave;

	u_short TData;
	void *TChaPos;
	void *TTempPos;
	u_short TTempLen;
	u_short Shadow;

	/* Filled in by P61_osc call (div ptrs by 4) */
	void *OscPtr;		  /* Points to end of current frame's sample-chunk. */
	u_short OscPtrRem;  /* Remainder for precision (internal use only) */
	void *OscPtrWrap; /* Wrap (end) pointer for current Paula soundloop */

	u_short DMABit;
} __attribute__((packed)) P61_ChannelBlock;

extern struct {
	u_short	Master;		/* Master volume (0-64) */
	u_short	UseTempo;	/* Use tempo? (0=no,non-zero=yes) */
	u_short	Play;     /* Activation flag (0=stop,1=play) */
	u_short	E8;			  /* Info nybble after command E8 */
	void *VBR;      /* If you're using non-valid execbase */
	u_short	Pos;		  /* Current song position (read only) */
	u_short	Pattern;	/* Current pattern (read only) */
	u_short	Row;		  /* Current pattern row (read only) */
	int  ChannelOffset[4];
} __attribute__((packed)) P61_ControlBlock;

#define P61_CHANNEL(I) \
  ((P61_ChannelBlock *)(((void *)&P61_ControlBlock) + \
                        P61_ControlBlock.ChannelOffset[(I)]))

/**
 * @brief Initialize the playroutine.
 *
 * @param Module	address of the module
 * @param Samples	null if samples are internal to the module, address of
 *                samples otherwise
 * @param Buffer	address of sample buffer if the module uses packed samples,
 *                otherwise can be left uninitialized
 * @result 0 if success, non-zero otherwise
 */

int P61_Init(void *Module asm("a0"),
              void *Samples asm("a1"),
              void *Buffer asm("a2"));

/**
 * @brief Main entry point from CIA/VB int or frame loop.
 */

void P61_Music(void);

/**
 * @brief Read samples the player is about to play in this frame.
 */

typedef struct {
  char *SamplePtr;    /* a2 */
  char *LoopStartPtr; /* a1 */
  char *LoopEndPtr;   /* d2 */
  short WrapCount;     /* d0 */
  short Count;         /* d1 */
  short RepLen;        /* d4 */
} P61_OscData;

bool P61_Osc(P61_ChannelBlock *channel asm("a0"), P61_OscData *data asm("a3"));

/**
 * @brief Jump to a specific position in the song. Starts from the beginning if
 *        out of limits.
 *
 * @param Position position in a module (Starts from the beginning if is out of
 *                 limits)
 */

void P61_SetPosition(u_char Position asm("d0"));

/**
 * @brief Stop the music.
 */
void P61_End(void);

#endif
