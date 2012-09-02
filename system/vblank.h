#ifndef __VBLANK_H__
#define __VBLANK_H__

#define FRAMERATE 50

void InstallVBlankIntServer();
void RemoveVBlankIntServer();
int GetVBlankCounter();
void SetVBlankCounter(int value);
void ChangeVBlankCounter(int value);

#endif
