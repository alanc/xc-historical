/*
 * $XConsortium: stipplemips.s,v 1.4 90/11/30 15:25:41 keith Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * MIPS assembly code for optimized text rendering.
 *
 * Other stippling could be done in assembly, but the payoff is
 * not nearly as large.  Mostly because large areas are heavily
 * optimized already.
 */

#ifdef MIPSEL
# define BitsR		sll
# define BitsL		srl
# define FourBits(dest,bits)	and	dest, bits, 0xf
#else
# define BitsR	srl
# define BitsL	sll
# define FourBits(dest,bits)	srl	dest, bits, 28
#endif

/* reordering instructions would be fatal here */
	.set	noreorder

	
/*
 * stipplestack(addr, stipple, value, stride, Count, Shift)
 *               4       5       6      7     16(sp) 20(sp)
 *
 *  Apply successive 32-bit stipples starting at addr, addr+stride, ...
 *
 *  Used for text rendering, but only when no data could be lost
 *  when the stipple is shifted left by Shift bits
 */
/* arguments */
#define addr	$4
#define stipple	$5
#define value	$6
#define stride	$7
#define Count	16($sp)
#define Shift	20($sp)

/* local variables */
#define count	$14
#define shift	$13
#define atemp	$12
#define bits	$11
#define lshift	$9
#define sbase	$8
#define stemp	$2

#define ForEachLine	$200
#define NextLine	$201
#define NextLine1	$202
#define CaseBegin	$203
#define ForEachBits	$204
#define ForEachBits1	$205
#define NextBits	$206

#ifdef TETEXT
#define	stipplestack	stipplestackte
#endif

	.globl	stipplestack
	.ent	stipplestack 2
stipplestack:
	.frame	$sp, 0, $31
	lw	count, Count			/* fetch stack params */
	la	sbase,CaseBegin			/* load up switch table */
	lw	shift, Shift
	li	lshift, 4			/* compute offset within */
	subu	lshift, lshift, shift		/*  stipple of remaining bits */
#ifndef MIPSEL
	addu	shift, shift, 24		/* first shift for MSB */
#endif
	/* do ... while (--count > 0); */
ForEachLine:
	lw	bits, 0(stipple)		/* get stipple bits */
	move	atemp, addr			/* set up for this line */
#ifdef TETEXT
	/* Terminal emulator fonts are expanded and have many 0 rows */
	beqz	bits, NextLine			/* skip out early on 0 */
#endif
	addu	addr, addr, stride		/* step for the loop */
#ifdef MIPSEL					/* get first bits */
	sll	stemp, bits, lshift
#else
	srl	stemp, bits, shift
#endif
	and	stemp, stemp, 0xf0		/* compute first branch */
	addu	stemp, stemp, sbase		/*  ... */
	j	stemp				/*  ... */
	BitsL	bits, bits, lshift		/* set remaining bits */

ForEachBits:
	addu	atemp, atemp, 4
ForEachBits1:
	FourBits(stemp, bits)			/* compute jump for */
	sll	stemp, stemp, 4			/*  next four bits */
	addu	stemp, stemp, sbase		/*  ... */
	j	stemp				/*  ... */
	BitsL	bits, bits, 4			/* step for remaining bits */
CaseBegin:
	bnez	bits, ForEachBits1		/* 0 */
	addu	atemp, atemp, 4
	b	NextLine1
 	addu	count, count, -1

	bnez	bits, ForEachBits		/* 1 */
	sb	value, 0(atemp)
	b	NextLine1
 	addu	count, count, -1
					
	bnez	bits, ForEachBits		/* 2 */
	sb	value, 1(atemp)
	b	NextLine1
 	addu	count, count, -1
					
	bnez	bits, ForEachBits		/* 3 */
	sh	value, 0(atemp)
	b	NextLine1
 	addu	count, count, -1
					
	bnez	bits, ForEachBits		/* 4 */
	sb	value, 2(atemp)
	b	NextLine1
 	addu	count, count, -1
					
	sb	value, 0(atemp)			/* 5 */
	b	NextBits
	sb	value, 2(atemp)
	nop
					
	sb	value, 1(atemp)			/* 6 */
	b	NextBits
	sb	value, 2(atemp)
	nop
					
	bnez	bits, ForEachBits		/* 7 */
	swl	value, 2(atemp)
	b	NextLine1
 	addu	count, count, -1
					
	bnez	bits, ForEachBits		/* 8 */
	sb	value, 3(atemp)
	b	NextLine1
 	addu	count, count, -1
					
	sb	value, 0(atemp)			/* 9 */
	b	NextBits
	sb	value, 3(atemp)
	nop
					
	sb	value, 1(atemp)			/* a */
	b	NextBits
	sb	value, 3(atemp)
	nop
					
	sh	value, 0(atemp)			/* b */
	b	NextBits
	sb	value, 3(atemp)
	nop
					
	bnez	bits, ForEachBits		/* c */
	sh	value, 2(atemp)
	b	NextLine1
 	addu	count, count, -1
					
	sb	value, 0(atemp)			/* d */
	b	NextBits
	sh	value, 2(atemp)
	nop
					
	bnez	bits, ForEachBits		/* e */
	swr	value, 1(atemp)
	b	NextLine1
 	addu	count, count, -1
					
	bnez	bits, ForEachBits		/* f */
	sw	value, 0(atemp)
	b	NextLine1
 	addu	count, count, -1

	/*
 	 * code for cases where there isn't room to
 	 * do the loop test inside the case
	 */
NextBits:
	bnez	bits,ForEachBits1
	addu	atemp, atemp, 4
NextLine:
	addu	count, count, -1
NextLine1:
	bnez	count, ForEachLine		/* test for next line */
	addu	stipple, stipple, 4		/* step stipple */
	j	$31				/* return */
	nop
	.end	stipplestack
