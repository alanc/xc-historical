/*
 * $XConsortium: ibmColor.c,v 1.1 91/05/10 08:59:17 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

/*
 * skyHdwr.c - initialize hardware registers
 */

#include <sys/types.h>
#include <sys/hft.h>
#include <sys/entdisp.h>

#include "X.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "cursorstr.h"
#include "ibmScreen.h"
#include "ibmTrace.h"

#include "skyHdwr.h"
#include "skyReg.h"

int skyHdwrInit(index)
int index ;
{

   TRACE(("skyHdwrInit\n"));

   SKYWAY_SINDEX_REG(index) = 0x5600;/* disable the cursor in the DAC   */
   SKYWAY_SINDEX_REG(index) = 0x6c04;
   SKYWAY_SINDEX_REG(index) = 0x5061; /* set CRTC to prepare for reset  */
   SKYWAY_SINDEX_REG(index) = 0x5060; /* set CRTC to reset              */

   SKYWAY_MEM_REG(index) = 0x0b ;   /* Motorola order, 8 bits/pixel */
   SKYWAY_INT_REG(index) = 0x00 ;
   SKYWAY_INS_REG(index) = 0xff ;    /* clear all interrupt status bits */
   SKYWAY_VMC_REG(index) = 0x00 ;    /* disable virtual memory interrupts */
   SKYWAY_VMS_REG(index) = 0xff ;    /* clear virtual memory interrupts */

   /* native motorola mode ; memory decoding enabled */

   SKYWAYSetMode(index,0x0c);

   /* set the memory configuration register to 64 bit serializer width,
      256K x 4 module type, and 64 bit physical width for 64 bit wide
      serializer                                                        */

   SKYWAY_SINDEX_REG(index) = 0x009e;

   /* initialize the DAC                                                */

SKYWAY_SINDEX_REG(index) = 0x6006; /* Put '01000011'b into the Brooktree */
SKYWAY_SINDEX_REG(index) = 0x6443; /* command register to initialize it. */
SKYWAY_SINDEX_REG(index) = 0x6004; /* Turn on all bits in the read mask. */
SKYWAY_SINDEX_REG(index) = 0x64FF;
SKYWAY_SINDEX_REG(index) = 0x6005;      /*      60  INDX REG = 05 */
SKYWAY_SINDEX_REG(index) = 0x6400;      /*      64  BLNK MSK = 00 */
SKYWAY_SINDEX_REG(index) = 0x6007;      /*      60  INDX REG = 07 */
SKYWAY_SINDEX_REG(index) = 0x6400;      /*      64  TEST REG = 00 */

   /* The following values are taken from the Addendum to the SKYWAY1
    Video Subsystem Hardware Workbook dated October 3, 1988.    However */
   /* some values are taken from swstew.c, and don't match the addendum */

SKYWAY_SINDEX_REG(index) = 0x5402;/* Clock Frequency must be set to 0x02 */
SKYWAY_SINDEX_REG(index) = 0x3600;/* turn off the sprite control register */
SKYWAY_SINDEX_REG(index) = 0x6400;/* use Brooktree Palette DAC control  */
	                        /* register to turn off palette mask    */
SKYWAY_SINDEX_REG(index) = 0x5103;/* 8 bits/pixel, x1 scaling factors   */
/* pass two start */
SKYWAY_SINDEX_REG(index) = 0x0102;/* Set 0.25 clock increment on        */
SKYWAY_SINDEX_REG(index) = 0x16db;
SKYWAY_SINDEX_REG(index) = 0x18ac;
SKYWAY_SINDEX_REG(index) = 0x1a93;
SKYWAY_SINDEX_REG(index) = 0x1cc5;
SKYWAY_SINDEX_REG(index) = 0x1e06;
SKYWAY_SINDEX_REG(index) = 0x2a04;
/* pass tow end */

SKYWAY_SINDEX_REG(index) = 0x10db;/* 1760 pixels per scan line          */
SKYWAY_SINDEX_REG(index) = 0x129f;/* 1280 pixels in active picture area */
SKYWAY_SINDEX_REG(index) = 0x149f;/* 1280 is end of picture border area */
SKYWAY_SINDEX_REG(index) = 0x201f;/* vertical total regs set to 0x41f,  */
SKYWAY_SINDEX_REG(index) = 0x2104;/* which is 1056 lines                */
SKYWAY_SINDEX_REG(index) = 0x22ff;/* vertical display end registers are */
SKYWAY_SINDEX_REG (index)= 0x2303;/* set to 0x3ff, which is 1024 lines  */
SKYWAY_SINDEX_REG(index) = 0x24ff;/* vertical blanking start registers  */
SKYWAY_SINDEX_REG(index) = 0x2503;/* are set to 0x03ff (1024 lines)     */
SKYWAY_SINDEX_REG(index) = 0x261f;/* vertical blanking end registers are */
SKYWAY_SINDEX_REG(index) = 0x2704;/* set to 0x04ff (1056 lines)         */
SKYWAY_SINDEX_REG(index) = 0x2904;/* are set to 0x0402 (1027 lines)     */
SKYWAY_SINDEX_REG(index) = 0x4000;/* set the Start Address registers to */
SKYWAY_SINDEX_REG(index) = 0x4100;/* define the start address of the    */
SKYWAY_SINDEX_REG(index) = 0x4200;/* active picture to address 0        */
SKYWAY_SINDEX_REG(index) = 0x43a0;/* buffer pitch registers set to 1280 */
SKYWAY_SINDEX_REG(index) = 0x4400;/* pixels per scan line               */
SKYWAY_SINDEX_REG(index) = 0x64ff;/* turn on palette mask               */
SKYWAY_SINDEX_REG(index) = 0x5063;/* set Display Mode 1 register to:    */
	                        /*      Normal operation                */
	                        /*      No clock error                  */
	                        /*      Non interlaced                  */
	                        /*      Video Feature Interface disabled*/
	                        /*      Composite syncs enabled         */
	                        /*      + Vertical, - Horizontal        */
	                        /*         sync polarity                */

   return( 0 );

}

static struct { unsigned short r,g,b;} installed[256];

skySetColor(number,red,green,blue,index)
    register unsigned   number, red, green, blue;
    int index ;
{

    TRACE(("SkywaySetColor(%d,%d,%d,%d,%d)\n",number,red,green,blue,index));

    installed[number].r = red;
    installed[number].g = green;
    installed[number].b = blue;

    /* I don't know what this does, but it  */
    /* prevents blinking from occurring     */

    SKYWAY_SINDEX_REG(index) = (SPINDEXLO << 8 ) | 0x07 ;
    /* turn off mask */
    SKYWAY_SINDEX_REG(index) = (PALETTEMASK << 8 ) | 0x00 ;

    SKYWAYSetColorIndex(index,number) ;
    SKYWAYSetRGBColor(index,red,green,blue) ;

   return;

}
