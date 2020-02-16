#ifndef __FFP_H__
#define __FPP_H__

/* Motorola Fast Floating Point support. */

#include <proto/mathffp.h>

#include "types.h"

/* "mathtrans.library" is not a rom library, we cannot assume it's present in
 * the system, so we cannot use it, unfortunately. Thus we have to reimplement
 * some useful functions :( */

float SPFieee(float num asm("d0"));

#endif
