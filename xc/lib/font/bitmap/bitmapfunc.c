/*
 * $XConsortium$
 *
 * Copyright 1991 Massachusetts Institute of Technology
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

#include    "fontfilest.h"
#include    "bitmap.h"

typedef struct _BitmapFileFunctions {
    int         (*ReadFont) ( /* pFont, file, bit, byte, glyph, scan */ );
    int         (*ReadInfo) ( /* pFontInfo, file */ );
}           BitmapFileFunctionsRec, *BitmapFileFunctionsPtr;

extern int  pcfReadFont(), pcfReadFontInfo();
extern int  snfReadFont(), snfReadFontInfo();
extern int  bdfReadFont(), bdfReadFontInfo();
int BitmapOpenBitmap ();
extern int BitmapOpenScalable ();
int BitmapGetInfo ();

/*
 * these two arrays must be in the same order
 */
static BitmapFileFunctionsRec readers[] = {
    pcfReadFont, pcfReadFontInfo,
    snfReadFont, snfReadFontInfo,
    bdfReadFont, bdfReadFontInfo,
};

static FontRendererRec	renderers[] = {
    ".pcf", 4,
    BitmapOpenBitmap, BitmapOpenScalable, BitmapGetInfo, 0,
    ".snf", 4,
    BitmapOpenBitmap, BitmapOpenScalable, BitmapGetInfo, 0,
    ".bdf", 4,
    BitmapOpenBitmap, BitmapOpenScalable, BitmapGetInfo, 0,
};

BitmapOpenBitmap (fpe, ppFont, flags, entry, fileName, format, fmask)
    FontPathElementPtr	fpe;
    FontPtr		*ppFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
{
    FILE       *file;
    FontPtr     pFont;
    int         i;
    int         ret;
    int         bit,
                byte,
                glyph,
                scan,
		image;

    /*
     * compute offset into renderers array - same offset is
     * useful in the file functions array
     */
    i = entry->u.bitmap.renderer - renderers;
    file = fopen(fileName, "r");
    if (!file)
	return BadFontName;
    pFont = (FontPtr) xalloc(sizeof(FontRec));
    if (!pFont) {
	fclose(file);
	return AllocError;
    }
    pFont->refcnt = 0;

    /* set up default values */
    FontDefaultFormat(&bit, &byte, &glyph, &scan);
    /* get any changes made from above */
    ret = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);

    ret = (*readers[i].ReadFont) (pFont, file, bit, byte, glyph, scan);

    fclose(file);
    if (ret != Successful)
	xfree(pFont);
    else
	*ppFont = pFont;
    return ret;
}

BitmapGetInfo (pFontInfo, fileName)
    FontInfoPtr	    pFontInfo;
    char	    *fileName;
{
    FILE    *file;
    int	    i;
    int	    ret;
    FontRendererPtr renderer;

    renderer = FontFileMatchRenderer (fileName);
    if (!renderer)
	return BadFontName;
    i = renderer - renderers;
    file = fopen (fileName, "r");
    if (!file)
	return BadFontName;
    ret = (*readers[i].ReadInfo) (pFontInfo, file);
    fclose (file);
    return ret;
}

#define numRenderers	(sizeof renderers / sizeof renderers[0])

BitmapRegisterFontFileFunctions ()
{
    int	    i;

    for (i = 0; i < numRenderers; i++)
	FontFileRegisterRenderer (&renderers[i]);
}
