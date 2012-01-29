#ifndef __VBLANK_H__
#define __VBLANK_H__

void InstallVBlankIntServer();
void RemoveVBlankIntServer();
int GetVBlankCounter();
void SetVBlankCounter(int value);

#endif
