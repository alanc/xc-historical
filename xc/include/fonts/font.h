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
#ifndef FONT_H
#define FONT_H

	/* data structures */
typedef struct _Font		*FontPtr;
typedef struct _FontInfo	*FontInfoPtr;
typedef struct _FontProp	*FontPropPtr;
typedef struct _ExtentInfo	*ExtentInfoPtr;
typedef struct _FontPathElement *FontPathElementPtr;
typedef struct _CharInfo	*CharInfoPtr;
typedef struct _FontNames	*FontNamesPtr;

#define NullCharInfo	((CharInfoPtr) 0)
#define NullFont	((FontPtr) 0)
#define NullFontInfo	((FontInfoPtr) 0)

	/* draw direction */
#define LeftToRight 0
#define RightToLeft 1
#define BottomToTop 2
#define TopToBottom 3
typedef int DrawDirection;

#define NO_SUCH_CHAR	-1


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

typedef char	*closure;

extern int	LoadFont (/*client, lenfname, pfontname, ppfont*/);
extern int	StartListFontsWithInfo (/*client, length, pattern, maxNames*/);
extern int	NextListFontsWithInfo (/*closure, FontInfoP, nameP, nameLengthP*/);
extern int	SetFontPath (/**/);
extern unsigned char	*GetFontPath ();
extern FontNamesPtr MakeFontNamesRecord (/* size */);

#endif /* FONT_H */
