#include <custom.h>
#include <sprite.h>

void ResetSprites(void) {
  short i;

  DisableDMA(DMAF_SPRITE);

  /* Move sprites into foreground. */
  custom->bplcon2 = BPLCON2_PF1P2|BPLCON2_PF2P2;

  for (i = 0; i < 8; i++) {
    custom->sprpt[i] = NullSprData;
    custom->spr[i].datab = 0;
    custom->spr[i].dataa = 0;
  }
}
