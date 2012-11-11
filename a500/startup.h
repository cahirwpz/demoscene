#ifndef __STARTUP_H__
#define __STARTUP_H__

#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dosextens.h>
#include <graphics/gfxbase.h>

extern struct DosLibrary *DOSBase;
extern struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;

#endif
