/*
 * $XConsortium: fontscale.c,v 1.2 91/05/30 19:05:46 keith Exp $
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

Bool
FontFileAddScaledInstance (entry, vals, pFont, bitmapName)
    FontEntryPtr		entry;
    FontScalablePtr		vals;
    FontPtr			pFont;
    char			*bitmapName;
{
    FontScalableEntryPtr    scalable;
    FontScalableExtraPtr    extra;
    FontScaledPtr	    new;
    int			    newsize;

    scalable = &entry->u.scalable;
    extra = scalable->extra;
    if (extra->numScaled == extra->sizeScaled)
    {
	newsize = extra->sizeScaled + 4;
	new = (FontScaledPtr) xrealloc (extra->scaled,
			    newsize * sizeof (FontScaledRec));
	if (!new)
	    return FALSE;
	extra->sizeScaled = newsize;
	extra->scaled = new;
    }
    new = &extra->scaled[extra->numScaled++];
    new->vals = *vals;
    new->pFont = pFont;
    new->bitmap = (FontEntryPtr) bitmapName;
    if (pFont)
	pFont->fpePrivate = (pointer) entry;
    return TRUE;
}

/* Must call this after the directory is sorted */

FontFileSwitchStringsToBitmapPointers (dir)
    FontDirectoryPtr	dir;
{
    int	    s;
    int	    b;
    int	    i;
    FontEntryPtr	    scalable;
    FontEntryPtr	    nonScalable;
    FontScaledPtr	    scaled;
    FontScalableExtraPtr    extra;
    
    scalable = dir->scalable.entries;
    nonScalable = dir->nonScalable.entries;
    for (s = 0; s < dir->scalable.used; s++)
    {
	extra = scalable[s].u.scalable.extra;
	scaled = extra->scaled;
	for (i = 0; i < extra->numScaled; i++)
	    for (b = 0; b < dir->nonScalable.used; b++)
		if (nonScalable[b].name.name == (char *) scaled[i].bitmap)
		    scaled[i].bitmap = &nonScalable[b];
    }
}

void
FontFileRemoveScaledInstance (entry, pFont)
    FontEntryPtr	entry;
    FontPtr		pFont;
{
    FontScalableEntryPtr    scalable;
    FontScalableExtraPtr    extra;
    int			    i;

    scalable = &entry->u.scalable;
    extra = scalable->extra;
    for (i = 0; i < extra->numScaled; i++)
    {
	if (extra->scaled[i].pFont == pFont)
	{
	    extra->numScaled--;
	    for (; i < extra->numScaled; i++)
		extra->scaled[i] = extra->scaled[i+1];
	}
    }
}

Bool
FontFileCompleteXLFD (vals, def)
    FontScalablePtr	vals;
    FontScalablePtr	def;
{
    int		best;
    /*
     * If two of the three vertical scale values are specified, compute the
     * third.  If all three are specified, make sure they are consistent
     * (within a pixel)
     */

#define Close(a,b,d)  ((b) - (d) <= (a) && (a) <= (b) + (d))

    if (vals->point > 0 && vals->y > 0) {
	best = (vals->point * vals->y * 10) / 7227;
	if (vals->pixel <= 0)
	    vals->pixel = best;
	else if (!Close(vals->pixel, best, 1))
	    return FALSE;
    } else if (vals->pixel > 0 && vals->y > 0) {
	best = (vals->pixel * 7227) / (vals->y * 10);
	if (vals->point <= 0)
	    vals->point = best;
	else if (!Close(vals->point, best, 1))
	    return FALSE;
    } else if (vals->point > 0 && vals->pixel > 0) {
	best = (vals->pixel * 7227) / (vals->point * 10);
	if (vals->y <= 0)
	    vals->y = best;
	else if (!Close(vals->y, best, 1))
	    return FALSE;
    } else {
	if (vals->y <= 0)
	    vals->y = def->y;
	if (vals->pixel > 0)
	    vals->point = (vals->pixel * 7227) / (vals->y * 10);
	else if (vals->point > 0)
	    vals->pixel = (vals->point * vals->y * 10) / 7227;
	else {
	    vals->point = def->point;
	    vals->pixel = def->pixel;
	}
    }
    if (vals->x <= 0)
	vals->x = vals->y;

    if (vals->width < 0)
	vals->width = 0;
    return TRUE;
}

static Bool
MatchScalable (a, b)
    FontScalablePtr	a, b;
{
    return  a->x == b->x &&
	    a->y == b->y &&
	    (a->width == b->width || a->width == 0 || b->width == 0) &&
	    (a->pixel == b->pixel || a->point == b->point);
}

FontScaledPtr
FontFileFindScaledInstance (entry, vals)
    FontEntryPtr	entry;
    FontScalablePtr	vals;
{
    FontScalableEntryPtr    scalable;
    FontScalableExtraPtr    extra;
    int			    i;

    scalable = &entry->u.scalable;
    extra = scalable->extra;
    /* See if we've scaled to this value yet */
    for (i = 0; i < extra->numScaled; i++)
    {
	if (MatchScalable (&extra->scaled[i].vals, vals))
	    return &extra->scaled[i];
    }
    return 0;
}
