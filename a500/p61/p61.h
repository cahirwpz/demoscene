#ifndef __P61_H__
#define __P61_H__

#include <exec/types.h>

extern UWORD P61_visuctr[4];

typedef struct {
	APTR  P61_SampleOffset;
	UWORD P61_SampleLength;
	APTR  P61_RepeatOffset;
	UWORD P61_RepeatLength;
	UWORD P61_SampleVolume;
	UWORD P61_FineTune;
} __attribute__((packed)) P61_SampleBlock;

typedef struct {
	UBYTE SN_Note;
	UBYTE Command;
	UBYTE Info;
	UBYTE Pack;
	P61_SampleBlock *Sample;
	UWORD OnOff;
	APTR  ChaPos;
	APTR  TempPos;
	UWORD TempLen;

	UWORD Note;
	UWORD Period;
	UWORD Volume;
	UWORD Fine;

	UWORD Offset;
	UWORD LOffset;
	UWORD ToPeriod;
	UWORD TPSpeed;
	UBYTE VibCmd;
	UBYTE VibPos;
	UBYTE TreCmd;
	UBYTE TrePos;
	UWORD RetrigCount;
	UBYTE Funkspd;
	UBYTE Funkoff;
	APTR  Wave;

	UWORD TData;
	APTR  TChaPos;
	APTR  TTempPos;
	UWORD TTempLen;
	UWORD Shadow;

	/* Filled in by P61_osc call (div ptrs by 4) */
	APTR  OscPtr;		  /* Points to end of current frame's sample-chunk. */
	UWORD OscPtrRem;  /* Remainder for precision (internal use only) */
	APTR  OscPtrWrap; /* Wrap (end) pointer for current Paula soundloop */

	UWORD DMABit;
} __attribute__((packed)) P61_ChannelBlock;

extern struct {
	UWORD	Master;		/* Master volume (0-64) */
	UWORD	UseTempo;	/* Use tempo? (0=no,non-zero=yes) */
	UWORD	Play;     /* Activation flag (0=stop,1=play) */
	UWORD	E8;			  /* Info nybble after command E8 */
	APTR	VBR;		  /* If you're using non-valid execbase */
	UWORD	Pos;		  /* Current song position (read only) */
	UWORD	Pattern;	/* Current pattern (read only) */
	UWORD	Row;		  /* Current pattern row (read only) */
	LONG  ChannelOffset[4];
} __attribute__((packed)) P61_ControlBlock;

#define P61_CHANNEL(I) \
  ((P61_ChannelBlock *)(((APTR)&P61_ControlBlock) + \
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

LONG P61_Init(APTR Module asm("a0"),
              APTR Samples asm("a1"),
              APTR Buffer asm("a2"));

/**
 * @brief Read samples the player is about to play in this frame.
 */

typedef struct {
  BYTE *SamplePtr;    /* a2 */
  BYTE *LoopStartPtr; /* a1 */
  BYTE *LoopEndPtr;   /* d2 */
  WORD WrapCount;     /* d0 */
  WORD Count;         /* d1 */
  WORD RepLen;        /* d4 */
} P61_OscData;

BOOL P61_Osc(P61_ChannelBlock *channel asm("a0"), P61_OscData *data asm("a3"));

/**
 * @brief Jump to a specific position in the song. Starts from the beginning if
 *        out of limits.
 *
 * @param Position position in a module (Starts from the beginning if is out of
 *                 limits)
 */

void P61_SetPosition(UBYTE Position asm("d0"));

/**
 * @brief Stop the music.
 */
void P61_End();

#endif
