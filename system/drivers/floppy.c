#include <custom.h>
#include <common.h>
#include <debug.h>
#include <string.h>
#include <system/cia.h>
#include <system/errno.h>
#include <system/floppy.h>
#include <system/file.h>
#include <system/interrupt.h>
#include <system/memory.h>
#include <system/mutex.h>
#include <system/task.h>
#include <system/timer.h>

#define CIAF_DSKDESEL (CIAF_DSKSEL0|CIAF_DSKSEL1|CIAF_DSKSEL2|CIAF_DSKSEL3)

#define LOWER 0
#define UPPER 1

#define OUTWARDS 0
#define INWARDS  1

#define FLOPPY_SIZE (SECTOR_SIZE * NSECTORS * NTRACKS)

/*
 * Amiga MFM track format:
 * http://lclevy.free.fr/adflib/adf_info.html#p22
 */

typedef struct SectorInfo {
  u_char format;
  u_char trackNum;
  u_char sectorNum;
  u_char gapDist;
} SectorInfoT;

typedef struct Sector {
  u_int magic;
  u_short sync[2];
  u_int info[2];
  u_int sectorLabel[2][4];
  u_int checksumHeader[2];
  u_int checksum[2];
  u_int data[2][SECTOR_SIZE / sizeof(u_int)];
} SectorT;

/*
 * 3 1/2 inch dual density micro floppy disk drive specifications:
 * http://www.techtravels.org/wp-content/uploads/pefiles/SAMSUNG-SFD321B-070103.pdf
 *
 * Floppy disk rotates at 300 RPM, and transfer rate is 500Kib/s - which gives
 * exactly 12800 bytes per track. With Amiga track encoding that gives a gap of
 * 832 bytes between the end of sector #10 and beginning of sector #0.
 */

struct File {
  FileOpsT *ops;
  int pos;

  short headDir;    /* determine PRB.DSKDIREC bit */
  short trackNum;   /* determine PRB.DSKSIDE bit */
  CIATimerT *fdtmr;

  u_char prb;       /* caches CIAB PRB register */
  short trkInBuf;
  SectorT *encoded;
  u_char *decoded;
};

static inline void WaitDiskReady(void) {
  while (ciaa->ciapra & CIAF_DSKRDY);
}

#define STEP_SETTLE TIMER_MS(3)

static void StepHeads(FileT *f) {
  volatile u_char *cia = (u_char *)&ciab->ciaprb;
  u_char prb = f->prb;

  *cia = prb | CIAF_DSKDESEL;
  *cia = prb & ~CIAF_DSKSTEP;
  *cia = prb;

  f->trackNum += f->headDir;

  WaitTimerSleep(f->fdtmr, STEP_SETTLE);
}

#define DIRECTION_REVERSE_SETTLE TIMER_MS(18)

static void HeadsStepDirection(FileT *f, short inwards) {
  u_char prb = f->prb;

  if (inwards) {
    prb &= ~CIAF_DSKDIREC;
    f->headDir = 2;
  } else {
    prb |= CIAF_DSKDIREC;
    f->headDir = -2;
  }

  f->prb = prb;
 
  /* Update CIA-B PRB */
  {
    volatile u_char *cia = (u_char *)&ciab->ciaprb;

    *cia = prb | CIAF_DSKDESEL;
    *cia = prb;
  }

  WaitTimerSleep(f->fdtmr, DIRECTION_REVERSE_SETTLE);
}

static void ChangeDiskSide(FileT *f, short upper) {
  u_char prb = f->prb;

  if (upper) {
    prb &= ~CIAF_DSKSIDE;
    f->trackNum++;
  } else {
    prb |= CIAF_DSKSIDE;
    f->trackNum--;
  }

  f->prb = prb;

  /* Update CIA-B PRB */
  {
    volatile u_char *cia = (u_char *)&ciab->ciaprb;

    *cia = prb | CIAF_DSKDESEL;
    *cia = prb;
  }
}

static inline bool HeadsAtTrack0(void) {
  return !(ciaa->ciapra & CIAF_DSKTRACK0);
}

static void FloppyMotorOn(FileT *f) {
  volatile u_char *cia = (u_char *)&ciab->ciaprb;
  u_char prb = f->prb;

  *cia = prb | CIAF_DSKDESEL;
  *cia = prb & ~CIAF_DSKMOTOR;
  *cia = prb;

  f->prb = prb & ~CIAF_DSKMOTOR;

  /* TODO Or wait 500ms? */
  WaitDiskReady();
}

static void FloppyMotorOff(FileT *f) {
  volatile u_char *cia = (u_char *)&ciab->ciaprb;
  u_char prb = f->prb;

  *cia = prb | CIAF_DSKDESEL;
  *cia = prb | CIAF_DSKMOTOR;
  *cia = prb;

  /* TODO: Wait for motor to turn off? */
  f->prb = prb | CIAF_DSKMOTOR;
}

#define DISK_SETTLE TIMER_MS(15)

static void FloppyTrackRead(FileT *f, short trknum) {
  if (f->trackNum == trknum)
    return;

  if (f->trackNum == -1)
    f->trackNum = 0;

  if ((trknum ^ f->trackNum) & 1)
    ChangeDiskSide(f, trknum & 1);

  if (trknum != f->trackNum) {
    HeadsStepDirection(f, trknum > f->trackNum);
    while (trknum != f->trackNum)
      StepHeads(f);
  }

  WaitTimerSleep(f->fdtmr, DISK_SETTLE);

  custom->dsklen = 0; /* Make sure the DMA for the disk is turned off. */
  ClearIRQ(INTF_DSKBLK);
  EnableDMA(DMAF_DISK);

  Debug("Read track %d", trknum);

  custom->dskpt = (void *)f->encoded;
  /* Write track size twice to initiate DMA transfer. */
  custom->dsklen = DSK_DMAEN | (RAW_TRACK_SIZE / sizeof(short));
  custom->dsklen = DSK_DMAEN | (RAW_TRACK_SIZE / sizeof(short));

  TaskWait(INTF_DSKBLK);

  custom->dsklen = 0;
  DisableDMA(DMAF_DISK);
}

static inline SectorT *HeaderToSector(uint16_t *header) {
  return (SectorT *)((uintptr_t)header - offsetof(SectorT, info[0]));
}

static inline u_int DecodeLong(u_int odd, u_int even, u_int mask) {
  return ((odd & mask) << 1) | (even & mask);
}

static SectorT *FindSectorHeader(void *ptr) {
  uint16_t *data = ptr;
  /* Find synchronization marker and move to first location after it. */
  while (*data != DSK_SYNC)
    data++;
  while (*data == DSK_SYNC)
    data++;
  return HeaderToSector(data);
}

static bool FloppyTrackDecode(FileT *f, short trknum) {
  register u_int mask asm("d7") = 0x55555555;
  u_short *data = (u_short *)f->encoded;
  u_int *buf = (u_int *)f->decoded;
  SectorT *sector;
  short secnum = NSECTORS;

  /* Skip first word if it is not corrupted. */
  if (*data == DSK_SYNC)
    data++;

  sector = HeaderToSector(data);

  do {
    SectorInfoT info;

    *(u_int *)&info = DecodeLong(sector->info[0], sector->info[1], mask);
    if (info.trackNum != trknum) {
      Log("[Floppy] Reading from wrong track %d (expected %d)!\n",
          info.trackNum, trknum);
      return false;
    }

    Debug("sector=%p, #sector=%d, #track=%d",
          sector, info.sectorNum, info.trackNum);
    Assert(info.sectorNum < NSECTORS);

    /* Decode sector! */
    {
      u_int *dst = (void *)buf + info.sectorNum * SECTOR_SIZE;
      u_int *odd = sector->data[0];
      u_int *even = sector->data[1];
      short n = SECTOR_SIZE / sizeof(u_int) / 2 - 1;

      do {
        *dst++ = DecodeLong(*odd++, *even++, mask);
        *dst++ = DecodeLong(*odd++, *even++, mask);
      } while (--n >= 0);
    }

    /* Move to the next sector. */
    sector++;

    /* Is there a gap to skip after the sector? */
    if (info.gapDist == 1 && secnum > 1)
      sector = FindSectorHeader(sector);

    Assert((intptr_t)sector - (uintptr_t)f->encoded < RAW_TRACK_SIZE);
  } while (--secnum);

  return true;
}

static void DiskBlockInterrupt(__unused void *ptr) {
  TaskNotifyISR(INTF_DSKBLK);
}

static int FloppyRead(FileT *f, void *buf, u_int nbyte);
static int FloppySeek(FileT *f, int offset, int whence);
static void FloppyClose(FileT *f);

static FileOpsT FloppyOps = {
  .read = FloppyRead,
  .write = NoWrite,
  .seek = FloppySeek,
  .close = FloppyClose,
};

static MUTEX(FloppyMtx);

FileT *FloppyOpen(int num) {
  static FileT *f = NULL;

  MutexLock(&FloppyMtx);

  if (f == NULL) {
    Log("[Floppy] Initialising driver!\n");

    f = MemAlloc(sizeof(FileT), MEMF_PUBLIC|MEMF_CLEAR);
    f->ops = &FloppyOps;
    f->fdtmr = AcquireTimer(TIMER_CIAA_A);
    f->encoded = MemAlloc(RAW_TRACK_SIZE, MEMF_CHIP);
    f->decoded = MemAlloc(TRACK_SIZE, MEMF_PUBLIC);

    custom->dsksync = DSK_SYNC;
    custom->adkcon = ADKF_SETCLR | ADKF_MFMPREC | ADKF_WORDSYNC | ADKF_FAST;

    /* Default value of CIAB PRB value that selects controlled floppy drive. */
    f->prb = CIAF_DSKMOTOR | CIAF_DSKSTEP | CIAF_DSKDESEL;
    f->prb &= ~__BIT(CIAB_DSKSEL0 + num);
    ciab->ciaprb = f->prb;

    DisableDMA(DMAF_DISK);
    SetIntVector(INTB_DSKBLK, DiskBlockInterrupt, NULL);
    ClearIRQ(INTF_DSKBLK);
    EnableINT(INTF_DSKBLK);

    FloppyMotorOn(f);
    HeadsStepDirection(f, OUTWARDS);
    while (!HeadsAtTrack0())
      StepHeads(f);
    HeadsStepDirection(f, INWARDS);
    ChangeDiskSide(f, LOWER);
    f->trackNum = -1;
    f->trkInBuf = -1;

    Log("[Floppy] Drive DF%d ready!\n", num);
  }

  MutexUnlock(&FloppyMtx);

  return f;
}

static void FloppyClose(FileT *f) {
  FloppyMotorOff(f);
  DisableINT(INTF_DSKBLK);
  ClearIRQ(INTF_DSKBLK);
  ResetIntVector(INTB_DSKBLK);
  ReleaseTimer(f->fdtmr);
  MemFree(f->decoded);
  MemFree(f->encoded);
  MemFree(f);
}

static int FloppyRead(FileT *f, void *buf, u_int nbyte) {
  int left = nbyte;

  MutexLock(&FloppyMtx);

  Assume(f != NULL);
  Assume(f->pos >= 0 && f->pos <= FLOPPY_SIZE);

  Debug("$%p $%p %d+%d", f, buf, f->pos, nbyte);

  left = min(left, FLOPPY_SIZE - f->pos);

  while (left > 0) {
    short trknum, trkoff;
    int size;

    divmod16(f->pos, TRACK_SIZE, trknum, trkoff);

    if (trknum >= NTRACKS)
      break;

    if (trknum != f->trkInBuf) {
      FloppyTrackRead(f, trknum);
      if (!FloppyTrackDecode(f, trknum)) {
        Panic("[Floppy] Read error at track %d!", trknum);
      }
      f->trkInBuf = trknum;
    }

    /* Read to the end of track or less. */
    size = min(left, TRACK_SIZE - trkoff);

    memcpy(buf, f->decoded + trkoff, size);

    buf += size;
    left -= size;
    f->pos += size;
  }

  MutexUnlock(&FloppyMtx);

  return nbyte - left; /* how much did we read? */
}

static int FloppySeek(FileT *f, int offset, int whence) {
  if (whence == SEEK_CUR) {
    offset += f->pos;
    whence = SEEK_SET;
  }

  if (whence == SEEK_END) {
    offset = FLOPPY_SIZE + offset;
    whence = SEEK_SET;
  }

  if (whence == SEEK_SET) {
    /* New position is not within file. */
    if ((offset < 0) || (offset > FLOPPY_SIZE))
      return EINVAL;

    f->pos = offset;
    return offset;
  }

  return EINVAL;
}
