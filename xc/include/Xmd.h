/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#ifndef XMD_H
#define XMD_H 1
/* $Header: Xmd.h,v 1.25 88/08/11 10:50:13 jim Exp $ */
/*
 *  Xmd.h: MACHINE DEPENDENT DECLARATIONS.
 */

#ifdef CRAY
#define WORD64
#define UNSIGNEDBITFIELDS
#endif


/*
 * ibm pcc doesn't understand pragmas.
 */
#if defined(ibm032) && defined(__HIGHC__)
pragma on(pointers_compatible);
pragma off(char_default_unsigned);
#endif

/*
 * Bitfield suffixes for the protocol structure elements, if you
 * need them.  Note that bitfields are not guarranteed to be signed
 * (or even unsigned) according to ANSI C.
 */
#ifdef WORD64
#define B32 :32
#define B16 :16
#else
#define B32
#define B16
#endif /* WORD64 */

#if defined(WORD64) && defined(UNSIGNEDBITFIELDS)
#define bitExtend(val,bit,mask) (((val) & (bit)) ? ((val) | (mask)) : (val))
#define cvtINT8toInt(val)   bitExtend (val, 0x0080, 0xffffffffffffff00)
#define cvtINT16toInt(val)  bitExtend (val, 0x8000, 0xffffffffffff0000)
#define cvtINT32toInt(val)  bitExtend (val, 0x8000, 0xffffffff00000000)
#define cvtINT8toLong(val)  cvtINT8ToInt(val)
#define cvtINT16toLong(val) cvtINT16ToInt(val)
#define cvtINT32toLong(val) cvtINT32ToInt(val)
#undef bitExtend
#else
#define cvtINT8toInt(val) (val)
#define cvtINT16toInt(val) (val)
#define cvtINT32toInt(val) (val)
#define cvtINT8toLong(val) (val)
#define cvtINT16toLong(val) (val)
#define cvtINT32toLong(val) (val)
#endif /* UNSIGNEDBITFIELDS */


typedef long           INT32;
typedef short          INT16;
typedef char           INT8;

typedef unsigned long CARD32;
typedef unsigned short CARD16;
typedef unsigned char  CARD8;

typedef unsigned long		BITS32;
typedef unsigned short		BITS16;
typedef unsigned char		BYTE;

typedef unsigned char            BOOL;

#endif /* XMD_H */
