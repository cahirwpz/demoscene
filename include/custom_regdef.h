#ifndef __CUSTOM_REGDEF_H__
#define __CUSTOM_REGDEF_H__

#include <cdefs.h>

#ifndef __ASSEMBLER__

#include <types.h>

/* Complete map: https://www.winnicki.net/amiga/memmap/ */
struct Custom {
  uint16_t bltddat;
  uint16_t dmaconr;
  union {
    struct {
      uint16_t vposr;
      uint16_t vhposr;
    } s_vposr;
    uint32_t vposr_;
  } u_vposr;
  uint16_t dskdatr;
  uint16_t joy0dat;
  uint16_t joy1dat;
  uint16_t clxdat;
  uint16_t adkconr;
  uint16_t pot0dat;
  uint16_t pot1dat;
  uint16_t potinp;
  uint16_t serdatr;
  uint16_t dskbytr;
  uint16_t intenar;
  uint16_t intreqr;
  void *dskpt;
  uint16_t dsklen;
  uint16_t dskdat;
  uint16_t refptr;
  uint16_t vposw;
  uint16_t vhposw;
  uint16_t copcon;
  uint16_t serdat;
  uint16_t serper;
  uint16_t potgo;
  uint16_t joytest;
  uint16_t strequ;
  uint16_t strvbl;
  uint16_t strhor;
  uint16_t strlong;
  uint16_t bltcon0;
  uint16_t bltcon1;
  uint16_t bltafwm;
  uint16_t bltalwm;
  void *bltcpt;
  void *bltbpt;
  void *bltapt;
  void *bltdpt;
  uint16_t bltsize;
  uint8_t pad2d;
  uint8_t bltcon0l; /* low 8 bits of bltcon0, write only */
  uint16_t bltsizv;
  uint16_t bltsizh;
  uint16_t bltcmod;
  uint16_t bltbmod;
  uint16_t bltamod;
  uint16_t bltdmod;
  uint16_t pad34[4];
  uint16_t bltcdat;
  uint16_t bltbdat;
  uint16_t bltadat;
  uint16_t pad3b[3];
  uint16_t deniseid;
  uint16_t dsksync;
  uint32_t cop1lc;
  uint32_t cop2lc;
  uint16_t copjmp1;
  uint16_t copjmp2;
  uint16_t copins;
  uint16_t diwstrt;
  uint16_t diwstop;
  uint16_t ddfstrt;
  uint16_t ddfstop;
  uint16_t dmacon;  /* please use EnableDMA / DisableDMA macros */
  uint16_t clxcon;
  uint16_t intena_; /* please use EnableINT / DisableINT macros */
  uint16_t intreq_; /* please use ClearIRQ / CauseIRQ macros */
  uint16_t adkcon;
  struct AudChannel {
    uint16_t *ac_ptr;   /* ptr to start of waveform data */
    uint16_t ac_len;    /* length of waveform in words */
    uint16_t ac_per;    /* sample period */
    uint16_t ac_vol;    /* volume */
    uint16_t ac_dat;    /* sample pair */
    uint16_t ac_pad[2]; /* unused */
  } aud[4];
  void *bplpt[8];
  uint16_t bplcon0;
  uint16_t bplcon1;
  uint16_t bplcon2;
  uint16_t bplcon3;
  uint16_t bpl1mod;
  uint16_t bpl2mod;
  uint16_t bplcon4;
  uint16_t clxcon2;
  uint16_t bpldat[8];
  void *sprpt[8];
  struct SpriteDef {
    uint16_t pos;
    uint16_t ctl;
    uint16_t dataa;
    uint16_t datab;
  } spr[8];
  uint16_t color[32];
  uint16_t htotal;
  uint16_t hsstop;
  uint16_t hbstrt;
  uint16_t hbstop;
  uint16_t vtotal;
  uint16_t vsstop;
  uint16_t vbstrt;
  uint16_t vbstop;
  uint16_t sprhstrt;
  uint16_t sprhstop;
  uint16_t bplhstrt;
  uint16_t bplhstop;
  uint16_t hhposw;
  uint16_t hhposr;
  uint16_t beamcon0;
  uint16_t hsstrt;
  uint16_t vsstrt;
  uint16_t hcenter;
  uint16_t diwhigh;
  uint16_t padf3[11];
  uint16_t fmode;
};

#define vposr u_vposr.s_vposr.vposr
#define vhposr u_vposr.s_vposr.vhposr
#define vposr_ u_vposr.vposr_

#else

#include <asm.h>

#define custom _L(_custom)

#define dmaconr 0x002
#define dmacon 0x096
#define intenar 0x01c
#define intena 0x09a
#define intreqr 0x01e
#define intreq 0x09c
#define serdatr 0x018
#define serdat 0x030

#endif

/* defines for intena(r) register
 * https://www.winnicki.net/amiga/memmap/INTENA.html */
#define INTB_SETCLR 15  /* Set/Clear control bit. Determines if bits */
                        /* written with a one get set or cleared. Bits */
                        /* written with a zero are always unchanged */
#define INTB_INTEN 14   /* Master interrupt (enable only) */
#define INTB_EXTER 13   /* External interrupt */
#define INTB_DSKSYNC 12 /* Disk re-SYNChronized */
#define INTB_RBF 11     /* Serial port Receive Buffer Full */
#define INTB_AUD3 10    /* Audio channel 3 block finished */
#define INTB_AUD2 9     /* Audio channel 2 block finished */
#define INTB_AUD1 8     /* Audio channel 1 block finished */
#define INTB_AUD0 7     /* Audio channel 0 block finished */
#define INTB_BLIT 6     /* Blitter finished */
#define INTB_VERTB 5    /* Start of Vertical Blank */
#define INTB_COPER 4    /* Coprocessor */
#define INTB_PORTS 3    /* I/O Ports and timers */
#define INTB_SOFTINT 2  /* Software interrupt request */
#define INTB_DSKBLK 1   /* Disk Block done */
#define INTB_TBE 0      /* Serial port Transmit Buffer Empty */

#define INTF(x) __BIT(INTB_##x)

#define INTF_SETCLR INTF(SETCLR)
#define INTF_INTEN INTF(INTEN)
#define INTF_EXTER INTF(EXTER)
#define INTF_DSKSYNC INTF(DSKSYNC)
#define INTF_RBF INTF(RBF)
#define INTF_AUD3 INTF(AUD3)
#define INTF_AUD2 INTF(AUD2)
#define INTF_AUD1 INTF(AUD1)
#define INTF_AUD0 INTF(AUD0)
#define INTF_BLIT INTF(BLIT)
#define INTF_VERTB INTF(VERTB)
#define INTF_COPER INTF(COPER)
#define INTF_PORTS INTF(PORTS)
#define INTF_SOFTINT INTF(SOFTINT)
#define INTF_DSKBLK INTF(DSKBLK)
#define INTF_TBE INTF(TBE)

#define INTF_ALL 0x3FFF

/* defines for beamcon0 register
 * https://www.winnicki.net/amiga/memmap/BEAMCON0.html */
#define VARVBLANK __BIT(12)  /* Variable vertical blank enable */
#define LOLDIS __BIT(11)     /* long line disable */
#define CSCBLANKEN __BIT(10) /* redirect composite sync */
#define VARVSYNC __BIT(9)    /* Variable vertical sync enable */
#define VARHSYNC __BIT(8)    /* Variable horizontal sync enable */
#define VARBEAM __BIT(7)     /* variable beam counter enable */
#define DISPLAYDUAL __BIT(6) /* use UHRES pointer and standard pointers */
#define DISPLAYPAL __BIT(5)  /* set decodes to generate PAL display */
#define VARCSYNC __BIT(4)    /* Variable composite sync enable */
#define CSBLANK __BIT(3)     /* Composite blank out to CSY* pin */
#define CSYNCTRUE __BIT(2)   /* composite sync true signal */
#define VSYNCTRUE __BIT(1)   /* vertical sync true */
#define HSYNCTRUE __BIT(0)   /* horizontal sync true */

/* defines for bplcon0 register
 * https://www.winnicki.net/amiga/memmap/BPLCON0.html */
#define BPLCON0_ECSENA __BIT(0) /* Some bits from BPLCON3 are inhibited,
                                 * when the value is 0. (ECS/AGA-only) */
#define BPLCON0_ERSY __BIT(1)   /* External resync */
#define BPLCON0_LACE __BIT(2)   /* Interlace enable */
#define BPLCON0_LPEN __BIT(3)   /* Light pen enable */
#define BPLCON0_SHRES __BIT(6)  /* Super hi-res mode (35ns) */
#define BPLCON0_GAUD __BIT(8)   /* Genlock audio enable */
#define BPLCON0_COLOR __BIT(9)  /* Enables color burst output signal */
#define BPLCON0_DBLPF __BIT(10) /* Double playfield */
#define BPLCON0_HOMOD __BIT(11) /* Hold and modify mode */
#define BPLCON0_BPU(x) ((((x) & 7) << 12) | (((x) & 8) << 1))
#define BPLCON0_HIRES __BIT(15) /* Hi-res mode (70ns) */

/* defines for bplcon1 register
 * https://www.winnicki.net/amiga/memmap/BPLCON1.html */
/* AGA: Playfield 1 & 2 horizontal scroll values (0..255) in 35ns pixels */
#define BPLCON1_PF2H_AGA(x) \
  ((((x) & 0xc0) << 8) | (((x) & 0x03) << 12) | (((x) & 0x3f) << 2))
#define BPLCON1_PF1H_AGA(x) \
  ((((x) & 0xc0) << 4) | (((x) & 0x03) << 8) | (((x) & 0x3f) >> 2))
/* OCS: Playfield 1 & 2 horicontal scroll values (0..15) in 140ns pixels */
#define BPLCON1_PF2H(x) (((x) & 15) << 4)
#define BPLCON1_PF1H(x) ((x) & 15)

/* defines for bplcon2 register
 * https://www.winnicki.net/amiga/memmap/BPLCON2.html */
#define BPLCON2_ZDCTEN __BIT(10)   /* colormapped genlock bit */
#define BPLCON2_ZDBPEN __BIT(11)   /* use bitplane as genlock bits */
#define BPLCON2_ZDBPSEL(x) (((x) & 7) << 12) /* select one of 8 pitplanes
                                              * in ZDBPEN genlock mode */
#define BPLCON2_KILLEHB __BIT(9)   /* Disables extra half brite mode */
#define BPLCON2_PF2PRI __BIT(6)    /* Gives PF2 priority over PF1 */
/* Playfield 1 priority over sprites */
#define BPLCON2_PF1P_BOTTOM 4 /* PF1 below SP0-7 */
#define BPLCON2_PF1P_SP67 3   /* PF1 above SP6-7 and below SP0-5 */
#define BPLCON2_PF1P_SP47 2   /* PF1 above SP4-7 and below SP0-3 */
#define BPLCON2_PF1P_SP27 1   /* PF1 above SP2-7 and below SP0-1 */
#define BPLCON2_PF1P_SP07 0   /* PF1 above SP0-7 */
/* Playfield 2 priority over sprites */
#define BPLCON2_PF2P_BOTTOM (4 << 3) /* PF2 below SP0-7 */
#define BPLCON2_PF2P_SP67 (3 << 3)   /* PF2 above SP6-7 and below SP0-5 */
#define BPLCON2_PF2P_SP47 (2 << 3)   /* PF2 above SP4-7 and below SP0-3 */
#define BPLCON2_PF2P_SP27 (1 << 3)   /* PF2 above SP2-7 and below SP0-1 */
#define BPLCON2_PF2P_SP07 (0 << 3)   /* PF2 above SP0-7 */

/* defines for bplcon3 register
 * https://www.winnicki.net/amiga/memmap/BPLCON3.html */
#define BPLCON3_EXTBLNKEN __BIT(0) /* Causes BLANK output to be programmable
                                    * instead of reflecting internal fixed
                                    * decodes (ECSENA=1) */
#define BPLCON3_BRDSPRT __BIT(1)   /* Enables sprites outside the display
                                    * window (ECSENA=1) */
#define BPLCON3_ZDCLKEN __BIT(2)   /* ZD pin outputs a 14mhz clock (ECSENA=1) */
#define BPLCON3_BRDNTRAN __BIT(4)  /* Border area is transparent (ECSENA=1) */
#define BPLCON3_BRDNBLNK __BIT(5)  /* Border area is blanked (ECSENA=1) */
#define BPLCON3_SPRES(x) (((x) & 3) << 6) /* Determine resolution of all
                                           * sprites 0:default, 1:lores,
                                           * 2:hires, 3:shres */
#define BPLCON3_LOCT __BIT(9)      /* Causes write to COLORx registers to write
                                    * to lower order bits of RGB components */
#define BPLCON3_PF2OF(x) (((x) & 7) << 10) /* Bitplane color table offset when
                                            * PF2 has priority in DBLPF mode */
#define BPLCON3_BANK(x) (((x) & 7) << 13)  /* Selects one of eight color banks */

/* defines for bplcon4 register
 * https://www.winnicki.net/amiga/memmap/BPLCON4.html */
/* This 8 bit field is XOR`ed with the 8 bit plane color address,
 * thereby altering the color address sent to the color table.
 * Default value: 0x00 */
#define BPLCON4_BPLAM(x) ((x) << 8)
/* 4 Bit field provides the 4 high order color table address bits
 * for even sprites: SPR0,SPR2,SPR4,SPR6.
 * Default value: 0b0001 */
#define BPLCON4_ESPRM(x) (((x) & 15) << 4)
/* 4 Bit field provides the 4 high order color table address bits
 * for odd sprites: SPR1,SPR3,SPR5,SPR7.
 * Default value: 0b0001 */
#define BPLCON4_OSPRM(x) ((x) & 15)

/* defines for dmaconr register (read-only)
 * bits 0-8 correspnd to dmaconw definitions
 * https://www.winnicki.net/amiga/memmap/DMACON.html */
#define DMAF_BLTDONE 0x4000
#define DMAF_BLTNZERO 0x2000

#define DMAB_AUD0 0
#define DMAB_AUD1 1
#define DMAB_AUD2 2
#define DMAB_AUD3 3
#define DMAB_DISK 4
#define DMAB_SPRITE 5
#define DMAB_BLITTER 6
#define DMAB_COPPER 7
#define DMAB_RASTER 8
#define DMAB_MASTER 9
#define DMAB_BLITHOG 10
#define DMAB_BLTDONE 14
#define DMAB_BLTNZERO 13
#define DMAB_SETCLR 15

/* defines for dmacon register (write-only)
 * https://www.winnicki.net/amiga/memmap/DMACON.html */
#define DMAF(x) __BIT(DMAB_##x)

#define DMAF_SETCLR DMAF(SETCLR)
#define DMAF_AUD0 DMAF(AUD0)
#define DMAF_AUD1 DMAF(AUD1)
#define DMAF_AUD2 DMAF(AUD2)
#define DMAF_AUD3 DMAF(AUD3)
#define DMAF_DISK DMAF(DISK)
#define DMAF_SPRITE DMAF(SPRITE)
#define DMAF_BLITTER DMAF(BLITTER)
#define DMAF_COPPER DMAF(COPPER)
#define DMAF_RASTER DMAF(RASTER)
#define DMAF_MASTER DMAF(MASTER)
#define DMAF_BLITHOG DMAF(BLITHOG)

#define DMAF_AUDIO 0x000F
#define DMAF_ALL 0x01FF

/* defines for adkcon register
 * https://www.winnicki.net/amiga/memmap/ADKCON.html */
#define ADKB_SETCLR 15   /* standard set/clear bit */
#define ADKB_PRECOMP1 14 /* two bits of precompensation */
#define ADKB_PRECOMP0 13
#define ADKB_MFMPREC 12  /* use mfm style precompensation */
#define ADKB_UARTBRK 11  /* force uart output to zero */
#define ADKB_WORDSYNC 10 /* enable DSKSYNC register matching */
#define ADKB_MSBSYNC 9   /* (Apple GCR Only) sync on MSB for reading */
#define ADKB_FAST 8      /* 1 -> 2 us/bit (mfm), 2 -> 4 us/bit (gcr) */
#define ADKB_USE3PN 7    /* use aud chan 3 to modulate period of ?? */
#define ADKB_USE2P3 6    /* use aud chan 2 to modulate period of 3 */
#define ADKB_USE1P2 5    /* use aud chan 1 to modulate period of 2 */
#define ADKB_USE0P1 4    /* use aud chan 0 to modulate period of 1 */
#define ADKB_USE3VN 3    /* use aud chan 3 to modulate volume of ?? */
#define ADKB_USE2V3 2    /* use aud chan 2 to modulate volume of 3 */
#define ADKB_USE1V2 1    /* use aud chan 1 to modulate volume of 2 */
#define ADKB_USE0V1 0    /* use aud chan 0 to modulate volume of 1 */

#define ADKF(x) __BIT(ADKB_##x)

#define ADKF_SETCLR ADKF(SETCLR)
#define ADKF_PRECOMP1 ADKF(PRECOMP1)
#define ADKF_PRECOMP0 ADKF(PRECOMP0)
#define ADKF_MFMPREC ADKF(MFMPREC)
#define ADKF_UARTBRK ADKF(UARTBRK)
#define ADKF_WORDSYNC ADKF(WORDSYNC)
#define ADKF_MSBSYNC ADKF(MSBSYNC)
#define ADKF_FAST ADKF(FAST)
#define ADKF_USE3PN ADKF(USE3PN)
#define ADKF_USE2P3 ADKF(USE2P3)
#define ADKF_USE1P2 ADKF(USE1P2)
#define ADKF_USE0P1 ADKF(USE0P1)
#define ADKF_USE3VN ADKF(USE3VN)
#define ADKF_USE2V3 ADKF(USE2V3)
#define ADKF_USE1V2 ADKF(USE1V2)
#define ADKF_USE0V1 ADKF(USE0V1)

#define ADKF_PRE000NS 0                               /* 000 ns of precomp */
#define ADKF_PRE140NS (ADKF_PRECOMP0)                 /* 140 ns of precomp */
#define ADKF_PRE280NS (ADKF_PRECOMP1)                 /* 280 ns of precomp */
#define ADKF_PRE560NS (ADKF_PRECOMP0 | ADKF_PRECOMP1) /* 560 ns of precomp */

/* defines for dsklen register
 * https://www.winnicki.net/amiga/memmap/DSKLEN.html */
#define DSK_DMAEN __BIT(15)
#define DSK_WRITE __BIT(14)

/* defines for dsksync register
 * https://www.winnicki.net/amiga/memmap/DSKSYNC.html */
#define DSK_SYNC 0x4489

/* defines for serdatr register
 * https://www.winnicki.net/amiga/memmap/SERDATR.html */
#define SERDATF_OVRUN __BIT(15) /* Serial port receiver overrun */
#define SERDATF_RBF __BIT(14)   /* Serial port receive buffer full */
#define SERDATF_TBE __BIT(13)   /* Serial port transmit buffer empty */
#define SERDATF_TSRE __BIT(12)  /* Serial port transmit shift reg. empty */

/* defines for potgor register
 * https://www.winnicki.net/amiga/memmap/POTGO.html */
#define OUTRY __BIT(15) /* Output enable for bit 14 (1=output) */
#define DATRY __BIT(14) /* Data for port 2, pin 9 */
#define OUTRX __BIT(13) /* Output enable for bit 12 */
#define DATRX __BIT(12) /* Data for port 2, pin 5 */
#define OUTLY __BIT(11) /* Output enable for bit 10 */
#define DATLY __BIT(10) /* Data for port 1, pin 9 (right mouse button) */
#define OUTLX __BIT(9)  /* Output enable for bit 8 */
#define DATLX __BIT(8)  /* Data for port 1, pin 5 (middle mouse button) */

/* defines for vposr register
 * https://www.winnicki.net/amiga/memmap/VPOSR.html */
#define CHIPID_NTSC 0x10
#define CHIPID_PAL 0x00
#define CHIPID_AGNUS 0x00
#define CHIPID_FAT_AGNUS 0x20
#define CHIPID_FAT_AGNUS_V5 0x21
#define CHIPID_ALICE 0x22
#define CHIPID_ALICE_V3 0x23
#define CHIPID_MASK 0x7f

/* defines for fmode register (AGA-only)
 * https://www.winnicki.net/amiga/memmap/FMODE.html */
#define FMODE_SSCAN2 __BIT(15) /* Global enable for sprite scan-doubling */
#define FMODE_BSCAN2 __BIT(14) /* Enables the use of 2nd P/F modulus on an
                                * alternate line basis to support bitplane
                                * scan-doubling. */
#define FMODE_SPAGEM __BIT(3) /* Sprite page mode (double CAS) */
#define FMODE_SPR32 __BIT(2)  /* Sprite 32 bit wide mode */
#define FMODE_BPAGEM __BIT(1) /* Bitplane Page Mode (double CAS) */
#define FMODE_BLP32 __BIT(0)  /* Bitplane 32 bit wide mode */

/* defines for copcon register
 * https://www.winnicki.net/amiga/memmap/COPCON.html */
#define COPCON_CDANG __BIT(0) /* Allows Copper access to all registers */

#endif /* !__CUSTOM_REGDEF_H__ */
