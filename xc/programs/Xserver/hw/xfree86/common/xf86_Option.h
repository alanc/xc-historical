/* $XConsortium: xf86_Option.h,v 1.4 95/01/06 20:57:42 kaleb Exp kaleb $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86_Option.h,v 3.18 1995/01/15 10:33:13 dawes Exp $ */
/*
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of David Wexelblat not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  David Wexelblat makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */


#ifndef _XF86_OPTION_H
#define _XF86_OPTION_H

/*
 * Structures and macros for handling option flags.
 */
#define MAX_OFLAGS	160
#define FLAGBITS	sizeof(unsigned long)
typedef struct {
	unsigned long flag_bits[MAX_OFLAGS/FLAGBITS];
} OFlagSet;

#define OFLG_SET(f,p)	((p)->flag_bits[(f)/FLAGBITS] |= (1 << ((f)%FLAGBITS)))
#define OFLG_CLR(f,p)	((p)->flag_bits[(f)/FLAGBITS] &= ~(1 << ((f)%FLAGBITS)))
#define OFLG_ISSET(f,p)	((p)->flag_bits[(f)/FLAGBITS] & (1 << ((f)%FLAGBITS)))
#define OFLG_ZERO(p)	memset((char *)(p), 0, sizeof(*(p)))

/*
 * Option flags.  Define these in numeric order.
 */
/* SVGA clock-related options */
#define OPTION_LEGEND		 0  /* Legend board with 32 clocks */
#define OPTION_SWAP_HIBIT	 1  /* WD90Cxx-swap high-order clock sel bit */
#define OPTION_8CLKS		 2  /* Probe 8 clocks instead of 4 (PVGA1) */
#define OPTION_16CLKS		 3  /* probe for 16 clocks instead of 8 */
#define OPTION_PROBE_CLKS	 4  /* Force clock probe for cards where a
				       set of preset clocks is used */
#define OPTION_HIBIT_HIGH	 5  /* Initial state of high order clock bit */
#define OPTION_HIBIT_LOW	 6
#define OPTION_UNDOC_CLKS	 7  /* ATI undocumented clocks */

/* Laptop display options */
#define OPTION_INTERN_DISP	10  /* Laptops - enable internal display (WD)*/
#define OPTION_EXTERN_DISP	11  /* Laptops - enable external display (WD)*/
#define OPTION_CLGD6225_LCD	12  /* Option to avoid setting the DAC to */
				   /* white on a clgd6225 with the LCD */
				   /* enabled */

/* Memory options */
#define OPTION_FAST_DRAM	20 /* fast DRAM (for ET4000, S3, AGX) */
#define OPTION_MED_DRAM		21 /* medium speed DRAM (for S3, AGX) */
#define OPTION_SLOW_DRAM	22 /* slow DRAM (for Cirrus, S3, AGX) */
#define OPTION_NO_MEM_ACCESS	23 /* Unable to access video ram directly */
#define OPTION_NOLINEAR_MODE	24 /* chipset has broken linear access mode */
#define OPTION_INTEL_GX		25 /* Linear fb on an Intel GX/Pro (Mach32) */
#define OPTION_NO_2MB_BANKSEL	26 /* For cirrus cards with 512kx8 memory */
#define OPTION_FIFO_CONSERV	27 /* (cirrus) (agx) */
#define OPTION_FIFO_AGGRESSIVE	28 /* (cirrus) (agx) */
#define OPTION_MMIO		29 /* Use MMIO for Cirrus 543x */
#define OPTION_LINEAR		30 /* Use linear fb for Cirrus */
#define OPTION_FIFO_MODERATE  	31 /* (agx) */

/* Accel/cursor features */
#define OPTION_NOACCEL		40 /* Disable accel support in SVGA server */
#define OPTION_HW_CURSOR	41 /* Turn on HW cursor */
#define OPTION_SW_CURSOR	42 /* Turn off HW cursor (Mach32) */
#define OPTION_NO_BITBLT	43 /* Disable hardware bitblt (cirrus) */
#define OPTION_FAVOUR_BITBLT	44 /* Favour use of BitBLT (cirrus) */

/* RAMDAC options */
#define OPTION_BT485_CURS	50 /* Override Bt485 RAMDAC probe */
#define OPTION_TI3020_CURS	51 /* Use 3020 RAMDAC cursor (default) */
#define OPTION_NO_TI3020_CURS	52 /* Override 3020 RAMDAC cursor use */
#define OPTION_DAC_8_BIT	53 /* 8-bit DAC operation */
#define OPTION_SYNC_ON_GREEN	54 /* Set Sync-On-Green in RAMDAC */
#define OPTION_BT482_CURS       55 /* Use Bt482 RAMDAC cursor */
#define OPTION_S3_964_BT485_VCLK	56 /* probe/invert VCLK for 964 + Bt485 */

/* Vendor specific options */
#define OPTION_SPEA_MERCURY	70 /* pixmux for SPEA Mercury (S3) */
#define OPTION_NUMBER_NINE	71 /* pixmux for #9 with Bt485 (S3) */
#define OPTION_STB_PEGASUS	72 /* pixmux for STB Pegasus (S3) */
#define OPTION_ELSA_W1000PRO	73 /* pixmux for ELSA Winner 1000PRO (S3) */
#define OPTION_ELSA_W2000PRO	74 /* pixmux for ELSA Winner 2000PRO (S3) */
#define OPTION_DIAMOND		75 /* Diamond boards (S3) */

/* Misc options */
#define OPTION_CSYNC		90 /* Composite sync */
#define OPTION_SECONDARY	91 /* Use secondary address (HGC1280) */
#define OPTION_PCI_HACK		92 /* (S3) */
#define OPTION_POWER_SAVER	93 /* Power-down screen saver */

/* Debugging options */
#define OPTION_SHOWCACHE	100 /* Allow cache to be seen (S3) */
#define OPTION_FB_DEBUG		101 /* Linear fb debug for S3 */

/* Some AGX Tuning/Debugging options -- several are for development testing */
#define OPTION_8_BIT_BUS        110 /* Force 8-bit CPU interface - MR1:0 */
#define OPTION_WAIT_STATE       111 /* Force 1 bus wait state - MR1:1<-1 */
#define OPTION_NO_WAIT_STATE    112 /* Force no bus wait state - MR:1<-0 */
#define OPTION_CRTC_DELAY       113 /* Select XGA Mode Delay - MR1:3 */
#define OPTION_VRAM_128         114 /* VRAM shift every 128 cycles - MR2:0 */
#define OPTION_VRAM_256         115 /* VRAM shift every 256 cycles - MR2:0 */
#define OPTION_REFRESH_20       116 /* # clocks between scr refreshs - MR3:5 */
#define OPTION_REFRESH_25       117 /* # clocks between scr refreshs - MR3:5 */
#define OPTION_VLB_A            118 /* VESA VLB transaction type A   - MR7:2 */
#define OPTION_VLB_B            119 /* VESA VLB transaction type B   - MR7:2 */
#define OPTION_SPRITE_REFRESH   120 /* Sprite refresh every hsync    - MR8:4 */
#define OPTION_SCREEN_REFRESH   121 /* Screen refresh during blank   - MR8:5 */
#define OPTION_VRAM_DELAY_LATCH	122 /* Delay Latch                   - MR7:3 */
#define OPTION_VRAM_DELAY_RAS	123 /* Delay RAS signal              - MR7:4 */
#define OPTION_VRAM_EXTEND_RAS  124 /* Extend the RAS signal         - MR8:6 */
#define OPTION_ENGINE_DELAY     125 /* Wait state for some VLB's     - MR5:3 */

#define CLOCK_OPTION_PROGRAMABLE 0 /* has a programable clock */
#define CLOCK_OPTION_ICD2061A	 1 /* use ICD 2061A programable clocks      */
#define CLOCK_OPTION_SC11412     3 /* use SC11412 programmable clocks */
#define CLOCK_OPTION_S3GENDAC    4 /* use S3 Gendac programmable clocks */
#define CLOCK_OPTION_TI3025      5 /* use TI3025 programmable clocks */
#define CLOCK_OPTION_ICS2595     6 /* use TI3025 programmable clocks */
#define CLOCK_OPTION_CIRRUS      7 /* use Cirrus programmable clocks */
#define CLOCK_OPTION_CH8391      8 /* use Chrontel 8391 programmable clocks */

/*
 * Table to map option strings to tokens.
 */
typedef struct {
  char *name;
  int  token;
} OptFlagRec, *OptFlagPtr;

#ifdef INIT_OPTIONS
OptFlagRec xf86_OptionTab[] = {
  { "legend",		OPTION_LEGEND },
  { "swap_hibit",	OPTION_SWAP_HIBIT },
  { "8clocks",		OPTION_8CLKS },
  { "16clocks",		OPTION_16CLKS },
  { "probe_clocks",	OPTION_PROBE_CLKS },
  { "hibit_high",	OPTION_HIBIT_HIGH },
  { "hibit_low",	OPTION_HIBIT_LOW },
  { "undoc_clocks",	OPTION_UNDOC_CLKS },

  { "intern_disp",	OPTION_INTERN_DISP },
  { "extern_disp",	OPTION_EXTERN_DISP },
  { "clgd6225_lcd",	OPTION_CLGD6225_LCD },

  { "fast_dram",	OPTION_FAST_DRAM },
  { "med_dram",		OPTION_MED_DRAM },
  { "slow_dram",	OPTION_SLOW_DRAM },
  { "nomemaccess",	OPTION_NO_MEM_ACCESS },
  { "nolinear",		OPTION_NOLINEAR_MODE },
  { "intel_gx",		OPTION_INTEL_GX },
  { "no_2mb_banksel",	OPTION_NO_2MB_BANKSEL },
  { "fifo_conservative",OPTION_FIFO_CONSERV },
  { "fifo_moderate",    OPTION_FIFO_MODERATE },
  { "fifo_aggressive",	OPTION_FIFO_AGGRESSIVE },
  { "mmio",		OPTION_MMIO },
  { "linear",		OPTION_LINEAR },

  { "noaccel",		OPTION_NOACCEL },
  { "hw_cursor",	OPTION_HW_CURSOR },
  { "sw_cursor",	OPTION_SW_CURSOR },
  { "no_bitblt",	OPTION_NO_BITBLT },
  { "favour_bitblt",	OPTION_FAVOUR_BITBLT },
  { "favor_bitblt",	OPTION_FAVOUR_BITBLT },

  { "bt485_curs",	OPTION_BT485_CURS },
  { "ti3020_curs",	OPTION_TI3020_CURS },
  { "no_ti3020_curs",	OPTION_NO_TI3020_CURS },
  { "dac_8_bit",	OPTION_DAC_8_BIT },
  { "sync_on_green",    OPTION_SYNC_ON_GREEN },
  { "bt482_curs",	OPTION_BT482_CURS },
  { "s3_964_bt485_vclk",OPTION_S3_964_BT485_VCLK },

  { "spea_mercury",	OPTION_SPEA_MERCURY },
  { "number_nine",	OPTION_NUMBER_NINE },
  { "stb_pegasus",	OPTION_STB_PEGASUS },
  { "elsa_w1000pro",	OPTION_ELSA_W1000PRO },
  { "elsa_w1000isa",	OPTION_ELSA_W1000PRO }, /* These are treated the same */
  { "elsa_w2000pro",	OPTION_ELSA_W2000PRO },
  { "diamond",		OPTION_DIAMOND },

  { "composite",	OPTION_CSYNC },
  { "secondary",	OPTION_SECONDARY },
  { "pci_hack",		OPTION_PCI_HACK },
  { "power_saver",	OPTION_POWER_SAVER },

  { "showcache",	OPTION_SHOWCACHE },
  { "fb_debug",		OPTION_FB_DEBUG },

  { "8_bit_bus",        OPTION_8_BIT_BUS },
  { "wait_state",       OPTION_WAIT_STATE },
  { "no_wait_state",    OPTION_NO_WAIT_STATE },
  { "crtc_delay",       OPTION_CRTC_DELAY },
  { "vram_128",         OPTION_VRAM_128 },
  { "vram_256",         OPTION_VRAM_256 },
  { "refresh_20",       OPTION_REFRESH_20 },
  { "refresh_25",       OPTION_REFRESH_25 },
  { "vlb_a",            OPTION_VLB_A },
  { "vlb_b",            OPTION_VLB_B },
  { "sprite_refresh",   OPTION_SPRITE_REFRESH },
  { "screen_refresh",   OPTION_SPRITE_REFRESH },
  { "vram_delay_latch", OPTION_VRAM_DELAY_LATCH },
  { "vram_delay_ras",   OPTION_VRAM_DELAY_RAS },
  { "vram_extend_ras",  OPTION_VRAM_EXTEND_RAS },
  { "engine_delay",     OPTION_ENGINE_DELAY },

  { "",			-1 },
};

OptFlagRec xf86_ClockOptionTab [] = {
  { "icd2061a",		CLOCK_OPTION_ICD2061A },  /* generic ICD2061A */
  { "ics9161a",		CLOCK_OPTION_ICD2061A },  /* ICD2061A compatible */
  { "dcs2824",		CLOCK_OPTION_ICD2061A },  /* ICD2061A compatible */
  { "sc11412", 		CLOCK_OPTION_SC11412 },   /* Sierra SC11412 */
  { "s3gendac",		CLOCK_OPTION_S3GENDAC },  /* S3 gendac */
  { "s3_sdac",		CLOCK_OPTION_S3GENDAC },  /* S3 SDAC */
  { "ics5300",		CLOCK_OPTION_S3GENDAC },  /* S3 gendac compatible */
  { "ics5342",		CLOCK_OPTION_S3GENDAC },  /* S3 SDAC compatible */
  { "ti3025",		CLOCK_OPTION_TI3025 },    /* TI3025 */
  { "ics2595",		CLOCK_OPTION_ICS2595 },   /* ICS2595 */
  { "cirrus",		CLOCK_OPTION_CIRRUS }, 	  /* Cirrus built-in */
  { "ch8391",		CLOCK_OPTION_CH8391 }, 	  /* Chrontel 8391  */
  { "",			-1 },
};

#else
extern OptFlagRec xf86_OptionTab[];
extern OptFlagRec xf86_ClockOptionTab[];
#endif

#endif /* _XF86_OPTION_H */

