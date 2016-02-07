#include "hardware.h"
#include "memory.h"
#include "floppy.h"

#define LOWER 0
#define UPPER 1

#define OUTWARDS 0
#define INWARDS  1

/*
 * Amiga MFM track format:
 * http://lclevy.free.fr/adflib/adf_info.html#p22
 */

typedef struct {
  ULONG magic;
  UWORD sync[2];
  struct {
    UBYTE format;
    UBYTE trackNum;
    UBYTE sectorNum;
    UBYTE sectors;
  } info[2];
  UBYTE sectorLabel[2][16];
  ULONG checksumHeader[2];
  ULONG checksum[2];
  UBYTE data[2][512]; 
} SectorT;

/*
 * 3 1/2 inch dual density micro floppy disk drive specifications:
 * http://www.techtravels.org/wp-content/uploads/pefiles/SAMSUNG-SFD321B-070103.pdf
 *
 * Floppy disk rotates at 300 RPM, and transfer rate is 500Kib/s - which gives
 * exactly 12800 bytes per track. With Amiga track encoding that gives a gap of
 * 832 bytes between the end of sector #10 and beginning of sector #0.
 */

#define TRACK_SIZE (sizeof(SectorT) * NUMSECS + 832)

static WORD headDir;
static WORD trackNum;
static SectorT *track;

static inline void WaitDiskReady() {
 while (ciaa->ciapra & CIAF_DSKRDY);
}

#define STEP_SETTLE TIMER_MS(3)

static void StepHeads() {
  UBYTE *ciaprb = (UBYTE *)&ciab->ciaprb;

  bclr(ciaprb, CIAB_DSKSTEP);
  bset(ciaprb, CIAB_DSKSTEP);

  WaitTimerA(ciab, STEP_SETTLE);

  trackNum += headDir;
}

#define DIRECTION_REVERSE_SETTLE TIMER_MS(18)

static inline void HeadsStepDirection(WORD inwards) {
  UBYTE *ciaprb = (UBYTE *)&ciab->ciaprb;

  if (inwards) {
    bclr(ciaprb, CIAB_DSKDIREC);
    headDir = 2;
  } else {
    bset(ciaprb, CIAB_DSKDIREC);
    headDir = -2;
  }

  WaitTimerA(ciab, DIRECTION_REVERSE_SETTLE);
}

static inline void ChangeDiskSide(WORD upper) {
  UBYTE *ciaprb = (UBYTE *)&ciab->ciaprb;

  if (upper) {
    bclr(ciaprb, CIAB_DSKSIDE);
    trackNum++;
  } else {
    bset(ciaprb, CIAB_DSKSIDE);
    trackNum--;
  }
}

static inline BOOL HeadsAtTrack0() {
  return !(ciaa->ciapra & CIAF_DSKTRACK0);
}

static void FloppyMotorOn() {
  UBYTE *ciaprb = (UBYTE *)&ciab->ciaprb;

  bset(ciaprb, CIAB_DSKSEL0);
  bclr(ciaprb, CIAB_DSKMOTOR);
  bclr(ciaprb, CIAB_DSKSEL0);

  WaitDiskReady();
}

static void FloppyMotorOff() {
  UBYTE *ciaprb = (UBYTE *)&ciab->ciaprb;

  bset(ciaprb, CIAB_DSKSEL0);
  bset(ciaprb, CIAB_DSKMOTOR);
  bclr(ciaprb, CIAB_DSKSEL0);
}

void InitFloppy() {
  custom->dsksync = DSK_SYNC;
  custom->adkcon = ADKF_SETCLR | ADKF_MFMPREC | ADKF_WORDSYNC | ADKF_FAST;
  custom->intena = INTF_DSKBLK;
  custom->intreq = INTF_DSKBLK;
  custom->dmacon = DMAF_DISK;

  track = MemAlloc(TRACK_SIZE, MEMF_CHIP|MEMF_CLEAR);

  FloppyMotorOn();
  HeadsStepDirection(OUTWARDS);
  while (!HeadsAtTrack0())
    StepHeads();
  HeadsStepDirection(INWARDS);
  ChangeDiskSide(LOWER);
  trackNum = -1;
}

void KillFloppy() {
  FloppyMotorOff();
  MemFree(track);
}

#define DISK_SETTLE TIMER_MS(15)

__regargs void FloppyTrackRead(WORD num) {
  if (trackNum == num)
    return;

  if (trackNum == -1)
    trackNum = 0;

  if ((num ^ trackNum) & 1)
    ChangeDiskSide(num & 1);

  if (num != trackNum) {
    HeadsStepDirection(num > trackNum);
    while (num != trackNum)
      StepHeads();
  }

  WaitTimerA(ciab, DISK_SETTLE);

  custom->dsklen = 0; /* Make sure the DMA for the disk is turned off. */
  custom->intreq = INTF_DSKBLK;
  custom->dmacon = DMAF_SETCLR | DMAF_DISK;

  custom->dskpt = (APTR)track;
  /* Write track size twice to initiate DMA transfer. */
  custom->dsklen = DSK_DMAEN | (TRACK_SIZE / sizeof(WORD));
  custom->dsklen = DSK_DMAEN | (TRACK_SIZE / sizeof(WORD));

  while (!(custom->intreqr & INTF_DSKBLK));

  custom->dsklen = 0;
  custom->dmacon = DMAF_DISK;
}

#define DecodeMFM(odd, even, mask) \
  ((((odd) & (mask)) << 1) | ((even) & (mask)))

__regargs void FloppyTrackDecode(ULONG *buf) {
  WORD secnum = NUMSECS;
  SectorT *maybeSector = track;

  do {
    register ULONG mask asm("d7") = 0x55555555;
    WORD *data = (WORD *)maybeSector;
    struct {
      UBYTE format;
      UBYTE trackNum;
      UBYTE sectorNum;
      UBYTE sectors;
    } info;
    SectorT *sec;

    /* Find synchronization marker and move to first location after it. */
    do {
      while (*data++ != DSK_SYNC);
    } while (*data == DSK_SYNC);

    sec = (SectorT *)((APTR)data - offsetof(SectorT, info[0]));

    *(ULONG *)&info = DecodeMFM(*(ULONG *)&sec->info[0], 
                                *(ULONG *)&sec->info[1], mask);

#if 0
    Log("[Floppy] Decode: data = %lx, sector=%ld, track=%ld\n",
        (LONG)sec, (LONG)info.sectorNum, (LONG)info.trackNum);
#endif

    {
      ULONG *dst = (APTR)buf + info.sectorNum * TD_SECTOR;
      ULONG *odd = (APTR)sec->data[0];
      ULONG *even = (APTR)sec->data[1];
      WORD n = TD_SECTOR / sizeof(ULONG) / 2 - 1;

      do {
        *dst++ = DecodeMFM(*odd++, *even++, mask);
        *dst++ = DecodeMFM(*odd++, *even++, mask);
      } while (--n >= 0);
    }

    maybeSector = sec + 1;
  } while (--secnum);
}
