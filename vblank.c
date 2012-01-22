#include <exec/interrupts.h>
#include <hardware/intbits.h>

#include <proto/exec.h>
#include <inline/exec_protos.h>

static int VBlankCounter = 0;

__amigainterrupt __saveds static int VBlankServer(void) {
	VBlankCounter++;
	return 0;
}

static struct Interrupt VBlankInt = {
	{
		NULL,
		NULL,
		NT_INTERRUPT,
		-60,
		"VBlankCounter"
	},
	(APTR)&VBlankCounter,
	(APTR)VBlankServer
};

void InstallVBlankIntServer() {
	AddIntServer(INTB_VERTB, &VBlankInt);
}

void RemoveVBlankIntServer() {
	RemIntServer(INTB_VERTB, &VBlankInt);
}

int GetVBlankCounter() {
	return VBlankCounter;
}

void SetVBlankCounter(int value) {
	VBlankCounter = value;
}
