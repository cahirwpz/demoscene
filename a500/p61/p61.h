#ifndef __P61_H__
#define __P61_H__

#include <exec/types.h>

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

int P61_Init(APTR Module asm("a0"),
             APTR Samples asm("a1"),
             APTR Buffer asm("a2"));

/**
 * @brief Jump to a specific position in the song. Starts from the beginning if
 *        out of limits.
 *
 * @param Position position in a module (Starts from the beginning if is out of
 *                 limits)
 */

void P61_SetPosition(LONG Position asm("d0"));

/**
 * @brief Stop the music.
 */
void P61_End();

typedef struct {
	UBYTE SN_Note;
	UBYTE Command;
	UBYTE Info;
	UBYTE Pack;
	APTR  Sample;
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
#ifdef P61_oscill
	APTR  OscPtr;		  /* points to end of current frame's sample-chunk. */
	UWORD OscPtrRem;  /* remainder for precision (internal use only) */
	APTR  OscPtrWrap; /* wrap (end) pointer for current Paula soundloop */
#endif
	UWORD DMABit;
} P61_ChannelBlock;

struct P61_ControlBlock
{
	UWORD	Master;		/* Master volume (0-64) */
	UWORD	UseTempo;	/* Use tempo? (0=no,non-zero=yes) */
	UWORD	Play;     /* Activation flag (0=stop,1=play) */
	UWORD	E8;			  /* Info nybble after command E8 */
	APTR	VBR;		  /* If you're using non-valid execbase */
	UWORD	Pos;		  /* Current song position (read only) */
	UWORD	Pattern;	/* Current pattern (read only) */
	UWORD	Row;		  /* Current pattern row (read only) */
	P61_ChannelBlock *Channel[4];
};

extern struct P61_ControlBlock P61_ControlBlock;

#endif
