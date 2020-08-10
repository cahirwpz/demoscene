#include "debug.h"
#include "interrupts.h"
#include "memory.h"
#include "hardware.h"
#include "floppy.h"

#define DEBUG 0

#define LOWER 0
#define UPPER 1

#define OUTWARDS 0
#define INWARDS  1

/*
 * Amiga MFM track format:
 * http://lclevy.free.fr/adflib/adf_info.html#p22
 */

typedef struct {
  u_int magic;
  u_short sync[2];
  struct {
    u_char format;
    u_char trackNum;
    u_char sectorNum;
    u_char sectors;
  } info[2];
  u_char sectorLabel[2][16];
  u_int checksumHeader[2];
  u_int checksum[2];
  u_char data[2][512]; 
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

static short headDir;
static short trackNum;
static SectorT *track;

static inline void WaitDiskReady(void) {
  while (ciaa->ciapra & CIAF_DSKRDY);
}

#define STEP_SETTLE TIMER_MS(3)

static void StepHeads(void) {
  u_char *ciaprb = (u_char *)&ciab->ciaprb;

  bclr(ciaprb, CIAB_DSKSTEP);
  bset(ciaprb, CIAB_DSKSTEP);

  WaitTimerA(ciab, STEP_SETTLE);

  trackNum += headDir;
}

#define DIRECTION_REVERSE_SETTLE TIMER_MS(18)

static inline void HeadsStepDirection(short inwards) {
  u_char *ciaprb = (u_char *)&ciab->ciaprb;

  if (inwards) {
    bclr(ciaprb, CIAB_DSKDIREC);
    headDir = 2;
  } else {
    bset(ciaprb, CIAB_DSKDIREC);
    headDir = -2;
  }

  WaitTimerA(ciab, DIRECTION_REVERSE_SETTLE);
}

static inline void ChangeDiskSide(short upper) {
  u_char *ciaprb = (u_char *)&ciab->ciaprb;

  if (upper) {
    bclr(ciaprb, CIAB_DSKSIDE);
    trackNum++;
  } else {
    bset(ciaprb, CIAB_DSKSIDE);
    trackNum--;
  }
}

static inline bool HeadsAtTrack0(void) {
  return !(ciaa->ciapra & CIAF_DSKTRACK0);
}

static void FloppyMotorOn(void) {
  u_char *ciaprb = (u_char *)&ciab->ciaprb;

  bset(ciaprb, CIAB_DSKSEL0);
  bclr(ciaprb, CIAB_DSKMOTOR);
  bclr(ciaprb, CIAB_DSKSEL0);

  WaitDiskReady();
}

static void FloppyMotorOff(void) {
  u_char *ciaprb = (u_char *)&ciab->ciaprb;

  bset(ciaprb, CIAB_DSKSEL0);
  bset(ciaprb, CIAB_DSKMOTOR);
  bclr(ciaprb, CIAB_DSKSEL0);
}

void InitFloppy() {
  Log("[Init] Floppy drive driver!\n");

  custom->dsksync = DSK_SYNC;
  custom->adkcon = ADKF_SETCLR | ADKF_MFMPREC | ADKF_WORDSYNC | ADKF_FAST;

  DisableINT(INTF_DSKBLK);
  ClearIRQ(INTF_DSKBLK);
  DisableDMA(DMAF_DISK);

  track = MemAlloc(TRACK_SIZE, MEMF_CHIP);

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

__regargs void FloppyTrackRead(short num) {
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
  ClearIRQ(INTF_DSKBLK);
  EnableDMA(DMAF_DISK);

#if DEBUG
  Log("[Floppy] Read track %d.\n", num);
#endif

  custom->dskpt = (void *)track;
  /* Write track size twice to initiate DMA transfer. */
  custom->dsklen = DSK_DMAEN | (TRACK_SIZE / sizeof(short));
  custom->dsklen = DSK_DMAEN | (TRACK_SIZE / sizeof(short));

  WaitIRQ(INTF_DSKBLK);

  custom->dsklen = 0;
  DisableDMA(DMAF_DISK);
}

static inline u_int DecodeMFM(u_int odd, u_int even, u_int mask) {
  return ((odd & mask) << 1) | (even & mask);
}

__regargs void FloppyTrackDecode(u_int *buf) {
  short secnum = NUMSECS;
  SectorT *maybeSector = track;

  do {
    register u_int mask asm("d7") = 0x55555555;
    short *data = (short *)maybeSector;
    struct {
      u_char format;
      u_char trackNum;
      u_char sectorNum;
      u_char sectors;
    } info;
    SectorT *sec;

    /* Find synchronization marker and move to first location after it. */
    while (*data != DSK_SYNC) data++;
    while (*data == DSK_SYNC) data++;

    sec = (SectorT *)((void *)data - offsetof(SectorT, info[0]));

    *(u_int *)&info = DecodeMFM(*(u_int *)&sec->info[0], 
                                *(u_int *)&sec->info[1], mask);

#if DEBUG
    Log("[Floppy] Decode: data=%p, sector=%d, track=%d\n",
        sec, info.sectorNum, info.trackNum);
#endif

    {
      u_int *dst = (void *)buf + info.sectorNum * TD_SECTOR;
      u_int *odd = (void *)sec->data[0];
      u_int *even = (void *)sec->data[1];
      short n = TD_SECTOR / sizeof(u_int) / 2 - 1;

      do {
        *dst++ = DecodeMFM(*odd++, *even++, mask);
        *dst++ = DecodeMFM(*odd++, *even++, mask);
      } while (--n >= 0);
    }

    maybeSector = sec + 1;
  } while (--secnum);
}
