/* $XConsortium: MacFontFuncs,v 1.0 94/01/01 00:00:00 rws Exp $ */
/***********************************************************************
Copyright 1991 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

***********************************************************************/
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices or Digital
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR 
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS 
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF 
 * THIS SOFTWARE.
 *
 * Author: Dave Lemke, Network Computing Devices, Inc
 *
 */
/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/
#include	"fntfilst.h"
extern Boolean CGetOutlinePreferred();
extern Boolean CGetPreserveGlyph();

static int
MacFontOpenBitmap (fpe, ppFont, flags, entry, fileName, format, fmask)
    FontPathElementPtr	fpe;
    FontPtr		*ppFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
{
    FontPtr	pFont;
    FontScalableRec	vals;
    int		ret;
    int		bit, byte, glyph, scan, image;

    pFont = (FontPtr) xalloc(sizeof(FontRec));
    if (!pFont) return AllocError;
    pFont->refcnt = 0;
    pFont->maxPrivate = -1;
    pFont->devPrivates = (pointer *) 0;


    if (!FontParseXLFDName(entry->name.name, &vals, FONT_XLFD_REPLACE_NONE))
	return BadFontName; /* XXX how about non-XLFD names? */

    /* set up default values */
    FontDefaultFormat(&bit, &byte, &glyph, &scan);
    /* get any changes made from above */
    ret = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);

    ret = MacFontRenderFont(pFont, entry, &vals, bit, byte, glyph, scan);

    if (ret != Successful)
        xfree(pFont);
    else
        *ppFont = pFont;
    return ret;
}

static int
MacFontOpenScalable (fpe, ppFont, flags, entry, fileName, vals, format, fmask)
    FontPathElementPtr	fpe;
    FontPtr		*ppFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    FontScalablePtr	vals;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
{
    FontPtr	pFont;
    int		ret;
    int		bit, byte, glyph, scan, image;

    pFont = (FontPtr) xalloc(sizeof(FontRec));
    if (!pFont) return AllocError;
    pFont->refcnt = 0;
    pFont->maxPrivate = -1;
    pFont->devPrivates = (pointer *) 0;


    /* set up default values */
    FontDefaultFormat(&bit, &byte, &glyph, &scan);
    /* get any changes made from above */
    ret = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);

    ret = MacFontRenderFont(pFont, entry, vals, bit, byte, glyph, scan);

    if (ret != Successful)
        xfree(pFont);
    else
        *ppFont = pFont;
    return ret;
}

static int
MacFontOpenTrueTypeScalable (fpe, ppFont, flags, entry, fileName, vals, format, fmask)
    FontPathElementPtr	fpe;
    FontPtr		*ppFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    FontScalablePtr	vals;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
{
    int ret;
    Boolean	saveOutlinePreferred, savePreserveGlyph;

    saveOutlinePreferred = CGetOutlinePreferred();
    CSetOutlinePreferred(true);
	savePreserveGlyph = CGetPreserveGlyph();
	CSetPreserveGlyph(true);
    ret = MacFontOpenScalable (fpe, ppFont, flags, entry, fileName, vals, 
							   format, fmask);
	CSetPreserveGlyph(savePreserveGlyph);
    CSetOutlinePreferred(saveOutlinePreferred);
    return ret;
}

static int
MacFontOpenBitmapScalable (fpe, ppFont, flags, entry, fileName, vals, format, 
						   fmask)
    FontPathElementPtr	fpe;
    FontPtr		*ppFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    FontScalablePtr	vals;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
{
    int ret;
    Boolean	saveOutlinePreferred, savePreserveGlyph;

    saveOutlinePreferred = CGetOutlinePreferred();
    CSetOutlinePreferred(false);
	savePreserveGlyph = CGetPreserveGlyph();
	CSetPreserveGlyph(false);
    ret = MacFontOpenScalable (fpe, ppFont, flags, entry, fileName, vals, 
							   format, fmask);
	CSetPreserveGlyph(savePreserveGlyph);
    CSetOutlinePreferred(saveOutlinePreferred);
    return ret;
}

static int
MacFontBadFontName()
{
    return BadFontName;
}

static int
MacFontGetInfoBitmap (fpe, pFontInfo, entry, fontName, fileName, vals)
    FontPathElementPtr  fpe;
    FontInfoPtr         pFontInfo;
    FontEntryPtr        entry;
    FontNamePtr         fontName;
    char                *fileName;
    FontScalablePtr     vals;
{
    FontPtr		pFont;
    int			flags = 0;
    fsBitmapFormat	format = 0;
    fsBitmapFormatMask	fmask = 0;
	int			ret;

	ret = MacFontOpenBitmap(fpe, &pFont, flags, entry, fileName, format, fmask);
	if (ret == Successful) {
		*pFontInfo = pFont->info;
		pFont->info.props = 0;
		pFont->info.isStringProp = 0;
		pFont->info.nprops = 0;
		MacFontUnloadFont(pFont);
	}
	return ret;
}

static int
MacFontGetInfoBitmapScalable (fpe, pFontInfo, entry, fontName, fileName, vals)
    FontPathElementPtr  fpe;
    FontInfoPtr         pFontInfo;
    FontEntryPtr        entry;
    FontNamePtr         fontName;
    char                *fileName;
    FontScalablePtr     vals;
{
    FontPtr		pFont;
    int			flags = 0;
    fsBitmapFormat	format = 0;
    fsBitmapFormatMask	fmask = 0;
	int			ret;

	ret = MacFontOpenBitmapScalable (fpe, &pFont, flags, entry, fileName, vals,
									 format, fmask);
	if (ret == Successful) {
		*pFontInfo = pFont->info;
		pFont->info.props = 0;
		pFont->info.isStringProp = 0;
		pFont->info.nprops = 0;
		MacFontUnloadFont(pFont);
	}
	return ret;
}

static int
MacFontGetInfoTrueTypeScalable (fpe, pFontInfo, entry, fontName, fileName, vals)
    FontPathElementPtr  fpe;
    FontInfoPtr         pFontInfo;
    FontEntryPtr        entry;
    FontNamePtr         fontName;
    char                *fileName;
    FontScalablePtr     vals;
{
    FontPtr		pFont;
    int			flags = 0;
    fsBitmapFormat	format = 0;
    fsBitmapFormatMask	fmask = 0;
	int			ret;

	ret = MacFontOpenTrueTypeScalable(fpe, &pFont, flags, entry, fileName, vals,
									 format, fmask);
	if (ret == Successful) {
		*pFontInfo = pFont->info;
		pFont->info.props = 0;
		pFont->info.isStringProp = 0;
		pFont->info.nprops = 0;
		MacFontUnloadFont(pFont);
	}
	return ret;
}

static FontRendererRec renderer[] = {
    ".mac", 4, 
    MacFontOpenBitmap, MacFontOpenBitmapScalable, 
	MacFontGetInfoBitmap, MacFontGetInfoBitmapScalable,  0,
    ".tt",  3, 
    MacFontBadFontName, MacFontOpenTrueTypeScalable, 
	MacFontBadFontName, MacFontGetInfoTrueTypeScalable,  0,
};

void MacBlockHandler();
void MacWakeupHandler();
    
MacFontRegisterFontFileFunctions()
{
    MacFontStandardProps();

    FontFileRegisterRenderer (&renderer[0]);
    FontFileRegisterRenderer (&renderer[1]);

    RegisterBlockAndWakeupHandlers(MacBlockHandler, MacWakeupHandler, 0);
    InitMacWorld(); /* XXX move to osinit.c */
}
