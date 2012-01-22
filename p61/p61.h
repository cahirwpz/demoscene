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

int P61_Init(__reg("a0") APTR Module,
             __reg("a1") APTR Samples,
             __reg("a2") APTR Buffer);

/**
 * @brief Jump to a specific position in the song. Starts from the beginning if
 *        out of limits.	­
 *
 * @param Position position in a module (Starts from the beginning if is out of
 *                 limits)
 */

void P61_SetPosition(__reg("d0") LONG Position);

/**
 * @brief Stop the music.
 */
void P61_End();

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
};

extern struct P61_ControlBlock P61_ControlBlock;

#endif
