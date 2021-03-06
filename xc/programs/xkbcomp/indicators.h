/* $XConsortium: indicators.h,v 1.1 94/04/08 15:31:47 erik Exp $ */
/************************************************************
 Copyright (c) 1994 by Silicon Graphics Computer Systems, Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of Silicon Graphics not be 
 used in advertising or publicity pertaining to distribution 
 of the software without specific prior written permission.
 Silicon Graphics makes no representation about the suitability 
 of this software for any purpose. It is provided "as is"
 without any express or implied warranty.
 
 SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
 SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 THE USE OR PERFORMANCE OF THIS SOFTWARE.

 ********************************************************/

#ifndef INDICATORS_H
#define INDICATORS_H 1

extern	void	ClearIndicatorMapInfo(
#if NeedFunctionPrototypes
    Display *		/* dpy */,
    XkbFileLEDInfo *	/* info */
#endif
);


extern XkbFileLEDInfo *AddIndicatorMap(
#if NeedFunctionPrototypes
    XkbFileLEDInfo *	/* newLED */,
    XkbFileLEDInfo *	/* oldLEDs */,
    unsigned		/* merge */,
    Bool		/* report */
#endif
);

extern int	SetIndicatorMapField(
#if NeedFunctionPrototypes
    XkbFileLEDInfo *	/* led */,
    XkbDescPtr		/* xkb */,
    char *		/* field */,
    ExprDef *		/* arrayNdx */,
    ExprDef *		/* value */,
    unsigned		/* merge */
#endif
);

extern XkbFileLEDInfo *HandleIndicatorMapDef(
#if NeedFunctionPrototypes
    IndicatorMapDef *	/* stmt */,
    XkbDescPtr		/* xkb */,
    XkbFileLEDInfo *	/* dflt */,
    XkbFileLEDInfo *	/* oldLEDs */,
    unsigned 		/* mergeMode */
#endif
);

extern Bool CopyIndicatorMapDefs(
#if NeedFunctionPrototypes
    XkbFileInfo *	/* result */,
    XkbFileLEDInfo *	/* leds */
#endif
);

extern Bool BindIndicators(
#if NeedFunctionPrototypes
    XkbFileInfo *	/* result */,
    Bool		/* force */
#endif
);

#endif /* INDICATORS_H */
