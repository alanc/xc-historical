/* $XConsortium: dixfont.h,v 1.4 91/02/14 19:35:57 keith Exp $ */
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

#ifndef DIXFONT_H
#define DIXFONT_H 1

#include    <font.h>

#define NullDIXFontProp ((DIXFontPropPtr)0)

typedef struct _DIXFontProp *DIXFontPropPtr;

#ifndef R4_FONT_STRUCTURES

#define	FontAliasType	0x1000

#define	StillWorking	-2
#define	Suspended	-1

/* OpenFont flags */
#define FontLoadInfo	0x0001
#define FontLoadProps	0x0002
#define FontLoadMetrics	0x0004
#define FontLoadBitmaps	0x0008
#define FontLoadAll	0x000f
#define	FontOpenSync	0x0010

extern void UseFPE();
extern void FreeFPE();
extern void QueueFontWakeup();
extern void RemoveFontWakeup();
extern int FontWakeup();

#else

typedef struct _R4Font *FontPtr;

/*
 * this type is for people who want to talk about the font encoding
 */

typedef enum {
    Linear8Bit, TwoD8Bit, Linear16Bit, TwoD16Bit
}           FontEncoding;

#endif

#ifdef NOTDEF
typedef struct _FontData *FontDataPtr;

extern int  OpenFont();
extern unsigned char *GetFontPath();
extern int  SetFontPath();
#endif

#endif				/* DIXFONT_H */
