/* $Header$ */
/*
 * The Sun X drivers are a product of Sun Microsystems, Inc. and are provided
 * for unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify these drivers without charge, but are not authorized
 * to license or distribute them to anyone else except as part of a product or
 * program developed by the user.
 * 
 * THE SUN X DRIVERS ARE PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND
 * INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE
 * PRACTICE.
 *
 * The Sun X Drivers are provided with no support and without any obligation
 * on the part of Sun Microsystems, Inc. to assist in their use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THE SUN X
 * DRIVERS OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * ==========================================================================
 * Converted to Color Frame Buffer by smarks@sun, April-May 1987.  The "bit 
 * numbering" in the doc below really means "byte numbering" now.
 * ==========================================================================
 */

/*
   these tables are used by several macros in the cfb code.

   the vax numbers everything left to right, so bit indices on the
screen match bit indices in longwords.  the pc-rt and Sun number
bits on the screen the way they would be written on paper,
(i.e. msb to the left), and so a bit index n on the screen is
bit index 32-n in a longword

   see also cfbmaskbits.h
*/
#include	<X.h>
#include	<Xmd.h>

#if	(BITMAP_BIT_ORDER == MSBFirst)
/* NOTE:
the first element in starttab could be 0xffffffff.  making it 0
lets us deal with a full first word in the middle loop, rather
than having to do the multiple reads and masks that we'd
have to do if we thought it was partial.
*/
int cfbstarttab[] =
    {
	0x00000000,
	0x00FFFFFF,
	0x0000FFFF,
	0x000000FF
    };

int cfbendtab[] =
    {
	0x00000000,
	0xFF000000,
	0xFFFF0000,
	0xFFFFFF00
    };

/* a hack, for now, since the entries for 0 need to be all
   1 bits, not all zeros.
   this means the code DOES NOT WORK for segments of length
   0 (which is only a problem in the horizontal line code.)
*/
int cfbstartpartial[] =
    {
	0xFFFFFFFF,
	0x00FFFFFF,
	0x0000FFFF,
	0x000000FF
    };

int cfbendpartial[] =
    {
	0xFFFFFFFF,
	0xFF000000,
	0xFFFF0000,
	0xFFFFFF00
    };
#else		/* (BITMAP_BIT_ORDER == LSBFirst) */
/* NOTE:
the first element in starttab could be 0xffffffff.  making it 0
lets us deal with a full first word in the middle loop, rather
than having to do the multiple reads and masks that we'd
have to do if we thought it was partial.
*/
int cfbstarttab[] = 
	{
	0x00000000,
	0xFFFFFF00,
	0xFFFF0000,
	0xFF000000
	};

int cfbendtab[] = 
	{
	0x00000000,
	0x000000FF,
	0x0000FFFF,
	0x00FFFFFF
	};

/* a hack, for now, since the entries for 0 need to be all
   1 bits, not all zeros.
   this means the code DOES NOT WORK for segments of length
   0 (which is only a problem in the horizontal line code.)
*/
int cfbstartpartial[] = 
	{
	0xFFFFFFFF,
	0xFFFFFF00,
	0xFFFF0000,
	0xFF000000
	};

int cfbendpartial[] = 
	{
	0xFFFFFFFF,
	0x000000FF,
	0x0000FFFF,
	0x00FFFFFF
	};
#endif	(BITMAP_BIT_ORDER == MSBFirst)


/* used for masking bits in bresenham lines
   mask[n] is used to mask out all but bit n in a longword (n is a
screen position).
   rmask[n] is used to mask out the single bit at position n (n
is a screen posiotion.)
*/

#if	(BITMAP_BIT_ORDER == MSBFirst)
int cfbmask[] =
    {
	0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
    }; 
int cfbrmask[] = 
    {
	0x00FFFFFF, 0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00
    };
#else	/* (BITMAP_BIT_ORDER == LSBFirst) */
int cfbmask[] =
    {
	0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
    }; 
int cfbrmask[] = 
    {
	0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF, 0x00FFFFFF
    };
#endif	(BITMAP_BIT_ORDER == MSBFirst)

#ifdef	vax
#undef	VAXBYTEORDER
#endif
