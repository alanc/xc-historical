/*
 * $XConsortium: $ 
 * $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ali/ali_driver.c,v 3.0 1994/10/30 04:21:48 dawes Exp $
 */

#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#define  XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"
#include "vga.h"

/*
 * Definitions for ALI chips
 */
#define PROG_CLK	 6	/* ALG3105 clock #6 is programmable */
#define NumClocks	16	/* number of Clocks supported */
#define BitsXfer	13	/* number of bits transfer for ALG3105 */
#define ClK65MHz    0x0868	/* 65MHz video clock generated by ALG3105 */
#define ClK75MHz    0x0A68	/* 75MHz video clock generated by ALG3105 */

#ifdef XF86VGA16
#define MONOVGA
#endif


#ifndef MONOVGA
/* #include "vga256.h" */

extern void speedupcfbFillRectSolidCopy();
extern void speedupcfbDoBitbltCopy();
extern void speedupcfbLineSS();
extern void speedupcfbSegmentSS();
extern void speedupcfbFillBoxSolid();
#endif

typedef struct {
  vgaHWRec std;			/* good old IBM VGA */
  unsigned char Reg3D4_19;	/* Avance Logic Extd Reg */
  unsigned char Reg3D4_1A;
  unsigned char Reg3D4_28;
  unsigned char Reg3D4_2A;
  unsigned char Reg3CE_0B;
  unsigned char Reg3CE_0C;
  unsigned char Reg3C6_cmd;
  unsigned char RSegSel;	/* bank-read register select: 3D6 */
  unsigned char WSegSel;	/* bank-write register select: 3D7 */
} vgaALIRec, *vgaALIPtr;


static Bool     ALIProbe();	/* most important routine */
static char *   ALIIdent();
static Bool     ALIClockSelect();
/* 
static Bool     LegendClockSelect();
 */
static void     ALIEnterLeave();
static Bool     ALIInit();	/* 2nd most important routine */
static void *   ALISave();
static void     ALIRestore();
static void     ALIAdjust();
extern void     ALISetRead();
extern void     ALISetWrite();
extern void     ALISetReadWrite();
static Bool	Check_Writing_GCP();
static void	lock();
static void	unlock();

vgaVideoChipRec ALI = {
	/* 
	 * Function pointers
	 */
	ALIProbe,
	ALIIdent,
	ALIEnterLeave,
	ALIInit,
	ALISave,
	ALIRestore,
	ALIAdjust,
	(void (*)())NoopDDA,
	(void (*)())NoopDDA,
	(void (*)())NoopDDA, /* ALIFbInit, */
	ALISetRead,
	ALISetWrite,
	ALISetReadWrite,
	/*
	 * size of the mapped memory window, usually 64k.
	 */
	0x10000,		
	/*
	 * size of a video memory bank for ALI chipset.
	 */
	0x10000,
	/*
	 * number of bits by which an address is shifted
	 * right to determine the bank number for that address.
	 */
	16,
	/*
	 * bitmask used to determine the address within a
	 * specific bank.
	 */
	0xFFFF,
	/*
	 * bottom and top addresses for reads inside a given bank.
	 */
	0x00000, 0x10000,
	/*
	 * And corresponding limits for writes.
	 */
	0x00000, 0x10000,
	/*
	 * Whether this chipset supports a single bank register or
	 * separate read and write bank registers.  Almost all chipsets
	 * support two banks, and two banks are almost always faster
	 * (Trident 8900C and 9000 are odd exceptions).
	 */
	TRUE, 	/* two banks */
	/*
	 * If the chipset requires vertical timing numbers to be divided
	 * by two for interlaced modes, set this to VGA_DIVIDE_VERT.
	 */
	VGA_NO_DIVIDE_VERT,
	/*
	 * This is a dummy initialization for the set of vendor/option flags
	 * that this driver supports.  It gets filled in properly in the
	 * probe function, if the probe succeeds (assuming the driver
	 * supports any such flags).
	 */
	{0,},
	/*
	 * This specifies how the virtual width is to be rounded.  The
	 * virtual width will be rounded down the nearest multiple of
	 * this value
	 */
	16,
	/* Added for v3.1 for compatibility */
	FALSE,
	0,
	0,
	FALSE,
	FALSE,
	NULL,
	1,
};

#define new ((vgaALIPtr)vgaNewVideoState)

#define ALI2301         0	/* ALI 2301 chip */
#define ALI2302         1	/* ALI 2302 chip */
#define ALI2308         2	/* ALI 2308 chip */
#define ALI2401         3	/* ALI 2401 chip */

static int ALIchipset;

/* static unsigned ALI_ExtPorts[] = {0x3D6, 0x3D7}; */
static unsigned ALI_ExtPorts[] = {0x3CE, 0x3D4, 0x3D6, 0x3D7};
static int Num_ALI_ExtPorts =
        (sizeof(ALI_ExtPorts)/sizeof(ALI_ExtPorts[0]));

/*
 * ALIIdent() - 
 *    for chip identification
 */

static char *
ALIIdent(n)
     int n;
{
  static char *chipsets[] = {"ali2301","ali2302","ali2308","ali2401"};

  if (n + 1 > sizeof(chipsets) / sizeof(char *))
    return(NULL);
  else
    return(chipsets[n]);
}


/*
 * ALIClockSelect --
 *      1. select one of the possible clocks in case of user does NOT 
 *         define "Clocks" parameters in the XF86Config file.
 *      2. ALI clock chip can't read data, so you have to use fix table 
 *         clock only.
 */

static Bool
ALIClockSelect(no)
     int no;
{
  static unsigned char save1, save2, save3, save4;
  unsigned char i, temp, temp1, clk_data;
  int freq;	/* clock frequency */

  switch(no)
  {
    case CLK_REG_SAVE:
      save1 = inb(0x3CC);				/* save clk_0 & clk_1 */
      outb(0x3CE, 0x0C); save2 = inb(0x3CF) | 0x10; 	/* save clk_2 & 8-map ON */
      outb(0x3CE, 0x0B); save4 = inb(0x3CF);    	/* save clk/2 bits */
      outb(0x3CE, 0x1F); save3 = inb(0x3CF);    	/* save clk_3 */
      break;

    case CLK_REG_RESTORE:
      outb(0x3C2, save1);			/* restore clk_0 & clk_1 */
      outw(0x3CE, 0x0C | (save2 << 8));		/* restore clk_2 & 8-map */
      outw(0x3CE, 0x0B | (save4 << 8));		/* restore clk/2 */
      outw(0x3CE, 0x1F | (save3 << 8));		/* restore clk_3 */
      break;

    default:	/* will be called number of times according to NumClocks */
      outb(0x3CE, 0x0B); temp = inb(0x3CF);
      (no <= 7) ? outb(0x3CF, temp | 0x01) : outb(0x3CF, temp & 0xFC);
      temp = inb(0x3CC);
      outb(0x3C2, (temp & 0xF3) | ((no << 2) & 0x0C));	/* no=0..3 for 3C2.2 & 3 */
      outw(0x3CE, 0x0C | ((no & 0x04) << 11));		/* no=4..7 for 3CE.C.5 */
      break;
  }

  return(TRUE);
}


#if 0
/*
 * toggle_3C2() --
 *    turn clock bit (3CC.3) ON and OFF.
 */
static void
toggle_3C2()
{
   static unsigned char tmp;

   tmp = inb(0x3CC);			/* get clock and data */
   outb(0x3C2, tmp | 0x08);		/* turn clock bit ON */
   outb(0x3C2, tmp & 0xF7);		/* turn clock bit OFF */
}
#endif



/*
 * ALIProbe --
 *      check up whether a ALI based board is installed
 */

static Bool
ALIProbe()
{
  unsigned char temp;

  /*
   * Set up I/O ports to be used by this card
   */
  xf86ClearIOPortList(vga256InfoRec.scrnIndex);
  xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
  xf86AddIOPorts(vga256InfoRec.scrnIndex, Num_ALI_ExtPorts, ALI_ExtPorts);

  if (vga256InfoRec.chipset)
    {
#if 0
      /*
       * If chipset from XF86Config matches...
       */
      if (!StrCaseCmp(vga256InfoRec.chipset, "ali2"))
      {
        ErrorF("\nali is no longer valid.  Use one of\n");
        ErrorF("the names listed by the -showconfig option\n");
        return (FALSE);
      }
#endif
      if (!StrCaseCmp(vga256InfoRec.chipset, ALIIdent(0)))
              ALIchipset = ALI2301;
      else if (!StrCaseCmp(vga256InfoRec.chipset, ALIIdent(1)))
              ALIchipset = ALI2302;
      else if (!StrCaseCmp(vga256InfoRec.chipset, ALIIdent(2)))
              ALIchipset = ALI2308;
      else if (!StrCaseCmp(vga256InfoRec.chipset, ALIIdent(3)))
              ALIchipset = ALI2401;
      else
              return(FALSE);
      ALIEnterLeave(ENTER);
    }
  else
    {
      unsigned char origVal, newVal;
      char *ALIName;
      char *TreatAs = NULL;

      ALIEnterLeave(ENTER);
      /*
       *First, we disable the "key" and try to test 3CEh index 0Bh bits 4&5.
       *If those two bits can be read/write, it will not be Avance's chip.
       */
      temp = inb(vgaIOBase+4);
      outb(vgaIOBase+4, 0x1A); origVal = inb(vgaIOBase+5);
      outb(vgaIOBase+5, origVal & 0xef);
      if (Check_Writing_GCP())
        {
          ALIEnterLeave(LEAVE);
          return(FALSE);
        }
      outb(vgaIOBase+5, origVal | 0x10);
      if (!Check_Writing_GCP())
        {
          ALIEnterLeave(LEAVE);
          return(FALSE);
        }
       ALIchipset = ALI2301;
       ALIName = "ALI2301";
       ErrorF("%s Avance chipset version: 0x%02x (%s)\n",
                XCONFIG_PROBED, temp, ALIName);

    }

  /*
   * Detect how much display memory is installed
   */
  if (!vga256InfoRec.videoRam)
  {
      unsigned char config;

      outb(vgaIOBase+0x04, 0x1E); config = inb(vgaIOBase+0x05); /* 3X4.1E */

      switch(config & 0x03)    /* 3X4.1E.1 & 3X4.1E.0 */
      {
         case 0: vga256InfoRec.videoRam = 256; break;	/* 00 = 256K */
         case 1: vga256InfoRec.videoRam = 512; break;	/* 01 = 512K */
         case 2: vga256InfoRec.videoRam = 1024; break;	/* 10 = 1 MB */
         case 3: vga256InfoRec.videoRam = 2048; break;	/* 11 = 2 MB */
         default: 
  	   ErrorF ("More than 2MB display memory is installed!!!");
	   break;
       }
  }

	/*
         * User does NOT specify "Clocks" in XF86Config
	 */
  	if (!vga256InfoRec.clocks)	
    	{
		/*
		 * This utility function will probe for the clock values.
		 * It is passed the number of supported clocks, and a
		 * pointer to the clock-select function.
		 */
      		vgaGetClocks(NumClocks, ALIClockSelect);
    	}

	/*
	 * Last we fill in the remaining data structures.  We specify
	 * the chipset name, using the Ident() function and an appropriate
	 * index.  We set a boolean for whether or not this driver supports
	 * banking for the Monochrome server.  And we set up a list of all
	 * the vendor flags that this driver can make use of.
	 */
        vga256InfoRec.chipset = ALIIdent(ALIchipset);
  	vga256InfoRec.bankedMono = TRUE;

  	return(TRUE);
}


/*
 * ALIEnterLeave --
 *      enable/disable io-mapping
 */

static void 
ALIEnterLeave(enter)
     Bool enter;
{
  unsigned char temp;

  if (enter)
    {
      xf86EnableIOPorts(vga256InfoRec.scrnIndex);

      vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;	/* color or mono mode? */ 
      outb(vgaIOBase+4, 0x1A); temp = inb(vgaIOBase+5); /* unlock ALI special */
      outb(vgaIOBase+5, temp | 0x10);		/* turn on 3X4.1A.4 */
      outb(vgaIOBase+4, 0x11); temp = inb(vgaIOBase+5);
      outb(vgaIOBase+5, temp & 0x7F);		/* unlock CRTC regs. index 0..7 */

    }
  else
    {
      outb(vgaIOBase+4, 0x11); temp = inb(vgaIOBase + 5);
      outb(vgaIOBase+5, temp | 0x80);
      outb(vgaIOBase+4, 0x1A); temp = inb(vgaIOBase + 5);
      outb(vgaIOBase+5, temp & 0xef);        /* relock ALI special */

      xf86DisableIOPorts(vga256InfoRec.scrnIndex);
    }
}



/*
 * ALIRestore --
 *      restore a video mode
 */

static void
ALIRestore(restore)
  vgaALIPtr restore;
{
  unsigned char i;
  unsigned char temp;

  /* du_all_regs("ALIRestore():"); */
  outb(0x3CE, 0x0F); temp = inb(0x3CF);
  outb(0x3CF, temp | 0x04);	/* 3CE.F.2 = 1 for 1R,1W bank selection */
  outb(0x3D6, 0x00); /* select segment 3D6 (Read) bank 0 */
  outb(0x3D7, 0x00); /* select segment 3D7 (write) bank 0 */

  vgaHWRestore((vgaHWPtr)restore);

  outw(vgaIOBase+4, (restore->Reg3D4_19 << 8) | 0x19);	/* 3D4.19.6 & 3D4.19.7 */
  outw(vgaIOBase+4, (restore->Reg3D4_1A << 8) | 0x1A);
  /* DT */
  unlock();
  outw(0x3CE, (restore->Reg3CE_0B << 8) | 0x0B);
  outw(0x3CE, (restore->Reg3CE_0C << 8) | 0x0C);

  outw(vgaIOBase+4, (restore->Reg3D4_28 << 8) | 0x28);	/* Vert. Extd Regs */
  outw(vgaIOBase+4, (restore->Reg3D4_2A << 8) | 0x2A);	/* Horz. Extd Regs */

  inb(0x3C8);
  inb(0x3C6); inb(0x3C6); inb(0x3C6); inb(0x3C6);
  outb(0x3C6, restore->Reg3C6_cmd);

  outb(0x3D6, restore->RSegSel);	/* Read Bank */
  outb(0x3D7, restore->WSegSel);	/* Write Bank */

}



/*
 * ALISave --
 *      save the current video mode
 */

static void *
ALISave(save)
     vgaALIPtr save;
{
  unsigned char             i;
  unsigned char             temp, temp1, temp2;

  /*
   * we need this here , cause we MUST disable the ROM SYNC feature
   */
  outb(0x3CE, 0x0C); temp = inb(0x3CF);
  temp1 = inb(0x3D6);  
  temp2 = inb(0x3D7);
  outb(0x3D6, 0x00); /* segment select */

  save = (vgaALIPtr)vgaHWSave((vgaHWPtr)save, sizeof(vgaALIRec));
  save->RSegSel = temp1;
  save->WSegSel = temp2;

  outb(vgaIOBase+4, 0x19); save->Reg3D4_19 = inb(vgaIOBase+5);
  outb(vgaIOBase+4, 0x1A); save->Reg3D4_1A = inb(vgaIOBase+5);
  outb(vgaIOBase+4, 0x28); save->Reg3D4_28 = inb(vgaIOBase+5);
  outb(vgaIOBase+4, 0x2A); save->Reg3D4_2A = inb(vgaIOBase+5);
  save->Reg3CE_0C = temp;
  outb(0x3CE, 0x0B); save->Reg3CE_0B  = inb(0x3CF);

  inb(0x3C8);
  inb(0x3C6); inb(0x3C6); inb(0x3C6); inb(0x3C6);
  save->Reg3C6_cmd = inb(0x3C6);

  /* du_all_regs("ALISave():"); */

  return ((void *) save);
}



/*
 * ALIInit --
 *      Handle the initialization of the VGAs registers
 */

static Bool
ALIInit(mode)
     DisplayModePtr mode;
{
  unsigned char             temp, temp1;

  /* du_all_regs("ALIInit():" ); */
  if (!vgaHWInit(mode,sizeof(vgaALIRec)))
    return(FALSE);

/*  ErrorF ("vga256InfoRec.virtualX = %d\n", vga256InfoRec.virtualX); */
  temp = vga256InfoRec.virtualX >> 3;	/* in byte mode */
  temp1 = inb(vgaIOBase+4);	/* save 3x4 index */
  outb(vgaIOBase+4, 0x1C);	/* get 3x5.1C */
  if (!(inb(vgaIOBase+5) & 1))  temp <<= 1;	/* support 512K DRAM? */
  outb(vgaIOBase+4, 0x19);	/* interlaced mode? */
  if (!(inb(vgaIOBase+5) & 1))  temp >>= 1;	/* non-interlaced? */
  outb(vgaIOBase+4, temp1);	/* restore 3x4 index */

  new->std.CRTC[19] = temp; 	/* 3x5.13 = Offset Register */
  new->std.CRTC[20] = 0x40;	/* Index 0x14 */
  new->std.CRTC[23] = 0xA3;	/* Index 0x17 */
  new->Reg3D4_19 = (mode->Flags & V_INTERLACE) ? 0x03 : 0x02;
  new->Reg3D4_1A = 0x90;	/* 0x80; PCI Ultra chip & unlock */
  new->Reg3D4_28 = 0x00;
  new->Reg3D4_2A = 0x00;
  new->Reg3CE_0B = (mode->Clock <= 7) ? 0x01 : 0x00;	/* if clock < 40MHz set /2 */
  if (mode->Flags & V_INTERLACE) new->Reg3CE_0B = 0x38;
  new->Reg3CE_0C = 0x14;	/* set 8-maps & 8-bit transfer */
  new->Reg3CE_0C |= ((unsigned char) mode->Clock & 0x04) << 3; /* set clock vclk2 */
  new->Reg3C6_cmd= 0x02;
  /* du_all_regs("ALIInit():" ); */

  return(TRUE);
}



/*
 * ALIAdjust --
 *      adjust the current video frame to display the mouse cursor 
 *	and virtual screen range
 */

static void
ALIAdjust(x, y)
     int x, y;
{
  int Base;

  Base = (y * vga256InfoRec.virtualX + x + 3) >> 3;
  outw(vgaIOBase+4, (Base & 0x00FF00) | 0x0C);
  outw(vgaIOBase+4, ((Base & 0x00FF) << 8) | 0x0D);

/* Turn on the extended register 3x4.20 (bit 0 - bit 2) when the address
   exceeds 0xFFFF to fix the 640x480 screen scrolling back problem. */

  outw(vgaIOBase + 4, ((Base & 0x070000) >> 8) | 0x20);
}



/*
 * Check_Writing_GCP
 *  exit: 'TRUE' if bits 4 & 5 of 3CEh index 0Bh are read/writable.
 */

static Bool
Check_Writing_GCP()
{
  unsigned char temp,temp1,temp2,temp3;

  temp = inb(0x3CE);	/* get 3CE index */
  outb(0x3CE, 0x0B); temp1 = inb(0x3CF);
  temp2 = temp1 & 0x30;
  outb(0x3CF, (temp2 ^ 0x30));
  temp3 = ((inb(0x3CF) & 0x30) ^ 0x30);
  outb(0x3CF, temp1); 
  outb(0x3CE, temp);	/* restore 3CE index */
  if (temp2 == temp3)  return (TRUE);
  return(FALSE);
}


/*
 * unlock() -
 *   unlock some ALI registers thru 3X4.1A.4
 */
static void
unlock()
{
   unsigned char temp, temp1;

   temp = inb(vgaIOBase+4);		/* save index */
   outb(vgaIOBase+4, 0x1A); 
   temp1 = inb(vgaIOBase+5); 		/* unlock ALI special */
   outb(vgaIOBase+5, temp1 | 0x10);	/* turn on 3X4.1A.4 */
   outb(vgaIOBase+4, temp);		/* restore index */
}

#if 0
/*
 * lock() -
 *   lock some ALI registers thru 3X4.1A.4
 */
static void
lock()
{
   unsigned char temp, temp1;

   temp = inb(vgaIOBase+4);		/* save index */
   outb(vgaIOBase+4, 0x1A); 
   temp1 = inb(vgaIOBase+5); 		/* unlock ALI special */
   outb(vgaIOBase+5, temp1 & 0xEF);	/* turn off 3X4.1A.4 */
   outb(vgaIOBase+4, temp);		/* restore index */
}
#endif
