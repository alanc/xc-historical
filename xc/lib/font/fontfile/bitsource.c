/*
 * $XConsortium: bitsource.c,v 1.7 94/03/08 17:45:03 gildea Exp $
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

#include "fntfilst.h"

BitmapSourcesRec	FontFileBitmapSources;

Bool
FontFileRegisterBitmapSource (fpe)
    FontPathElementPtr	fpe;
{
    FontPathElementPtr	*new;
    int			i;
    int			newsize;

    for (i = 0; i < FontFileBitmapSources.count; i++)
	if (FontFileBitmapSources.fpe[i] == fpe)
	    return TRUE;
    if (FontFileBitmapSources.count == FontFileBitmapSources.size)
    {
	newsize = FontFileBitmapSources.size + 4;
	new = (FontPathElementPtr *) xrealloc (FontFileBitmapSources.fpe, newsize * sizeof *new);
	if (!new)
	    return FALSE;
	FontFileBitmapSources.size = newsize;
	FontFileBitmapSources.fpe = new;
    }
    FontFileBitmapSources.fpe[FontFileBitmapSources.count++] = fpe;
    return TRUE;
}

void
FontFileUnregisterBitmapSource (fpe)
    FontPathElementPtr	fpe;
{
    int	    i;

    for (i = 0; i < FontFileBitmapSources.count; i++)
	if (FontFileBitmapSources.fpe[i] == fpe)
	{
	    FontFileBitmapSources.count--;
	    if (FontFileBitmapSources.count == 0)
	    {
		FontFileBitmapSources.size = 0;
		xfree (FontFileBitmapSources.fpe);
		FontFileBitmapSources.fpe = 0;
	    }
	    else
	    {
	    	for (; i < FontFileBitmapSources.count; i++)
		    FontFileBitmapSources.fpe[i] = FontFileBitmapSources.fpe[i+1];
	    }
	    break;
	}
}

/*
 * Our set_path_hook: unregister all bitmap sources.
 * This is necessary because already open fonts will keep their FPEs
 * allocated, but they may not be on the new font path.
 * The bitmap sources in the new path will be registered by the init_func.
 */
void
FontFileEmptyBitmapSource()
{
    if (FontFileBitmapSources.count == 0)
	return;

    FontFileBitmapSources.count = 0;
    FontFileBitmapSources.size = 0;
    xfree (FontFileBitmapSources.fpe);
    FontFileBitmapSources.fpe = 0;
}

FontFileMatchBitmapSource (fpe, pFont, flags, entry, zeroPat, vals, format, fmask, noSpecificSize)
    FontPathElementPtr	fpe;
    FontPtr		*pFont;
    int			flags;
    FontEntryPtr	entry;
    FontNamePtr		zeroPat;
    FontScalablePtr	vals;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
    Bool		noSpecificSize;
{
    int			source;
    FontEntryPtr	zero;
    FontBitmapEntryPtr	bitmap;
    int			ret;
    FontDirectoryPtr	dir;
    FontScaledPtr	scaled;

    /*
     * Look through all the registered bitmap sources for
     * the same zero name as ours; entries along that one
     * can be scaled as desired.
     */
    ret = BadFontName;
    for (source = 0; source < FontFileBitmapSources.count; source++)
    {
    	if (FontFileBitmapSources.fpe[source] == fpe)
	    continue;
	dir = (FontDirectoryPtr) FontFileBitmapSources.fpe[source]->private;
	zero = FontFileFindNameInDir (&dir->scalable, zeroPat);
	if (!zero)
	    continue;
    	scaled = FontFileFindScaledInstance (zero, vals, noSpecificSize);
    	if (scaled)
    	{
	    if (scaled->pFont)
	    {
		*pFont = scaled->pFont;
		(*pFont)->fpe = FontFileBitmapSources.fpe[source];
		ret = Successful;
	    }
	    else if (scaled->bitmap)
	    {
		entry = scaled->bitmap;
		bitmap = &entry->u.bitmap;
		if (bitmap->pFont)
		{
		    *pFont = bitmap->pFont;
		    (*pFont)->fpe = FontFileBitmapSources.fpe[source];
		    ret = Successful;
		}
		else
		{
		    ret = FontFileOpenBitmap (
				FontFileBitmapSources.fpe[source],
				pFont, flags, entry, format, fmask);
		    if (ret == Successful && *pFont)
			(*pFont)->fpe = FontFileBitmapSources.fpe[source];
		}
	    }
	    else /* "cannot" happen */
	    {
		ret = BadFontName;
	    }
	    break;
    	}
    }
    return ret;
}
