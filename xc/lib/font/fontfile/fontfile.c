/*
 * $XConsortium: fontfile.c,v 1.15 93/09/17 18:26:40 gildea Exp $
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
/* $NCDId: @(#)fontfile.c,v 1.6 1991/07/02 17:00:46 lemke Exp $ */

#include    "fntfilst.h"

/*
 * Map FPE functions to renderer functions
 */

int
FontFileNameCheck (name)
    char    *name;
{
#ifndef NCD
    return *name == '/';
#else
    return ((strcmp(name, "built-ins") == 0) || (*name == '/'));
#endif
}

int
FontFileInitFPE (fpe)
    FontPathElementPtr	fpe;
{
    int			status;
    FontDirectoryPtr	dir;

    status = FontFileReadDirectory (fpe->name, &dir);
    if (status == Successful)
    {
	if (dir->nonScalable.used > 0)
	    if (!FontFileRegisterBitmapSource (fpe))
	    {
		FontFileFreeFPE (fpe);
		return AllocError;
	    }
	fpe->private = (pointer) dir;
    }
    return status;
}

/* ARGSUSED */
int
FontFileResetFPE (fpe)
    FontPathElementPtr	fpe;
{
    FontDirectoryPtr	dir;

    dir = (FontDirectoryPtr) fpe->private;
    if (FontFileDirectoryChanged (dir))
    {
	/* can't do it, so tell the caller to close and re-open */
	return FPEResetFailed;	
    }
    return Successful;
}

int
FontFileFreeFPE (fpe)
    FontPathElementPtr	fpe;
{
    FontFileUnregisterBitmapSource (fpe);
    FontFileFreeDir ((FontDirectoryPtr) fpe->private);
    return Successful;
}

/* ARGSUSED */
int
FontFileOpenFont (client, fpe, flags, name, namelen, format, fmask,
		  id, pFont, aliasName)
    pointer		client;
    FontPathElementPtr	fpe;
    int			flags;
    char		*name;
    int			namelen;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
    XID			id;
    FontPtr		*pFont;
    char		**aliasName;
{
    FontDirectoryPtr	dir;
    char		lowerName[MAXFONTNAMELEN];
    char		fileName[MAXFONTFILENAMELEN*2 + 1];
    FontNameRec		tmpName;
    FontEntryPtr	entry;
    FontScalableRec	vals;
    FontScalableEntryPtr   scalable;
    FontScaledPtr	scaled;
    FontBitmapEntryPtr	bitmap;
    FontAliasEntryPtr	alias;
    FontBCEntryPtr	bc;
    int			ret;
    Bool		noSpecificSize;
    int			nranges;
    fsRange		*ranges;
    
    if (namelen >= MAXFONTNAMELEN)
	return AllocError;
    dir = (FontDirectoryPtr) fpe->private;
    CopyISOLatin1Lowered (lowerName, name, namelen);
    lowerName[namelen] = '\0';
    tmpName.name = lowerName;
    tmpName.length = namelen;
    tmpName.ndashes = FontFileCountDashes (lowerName, namelen);
    /* Match XLFD patterns */
    ranges = FontParseRanges(lowerName, &nranges);

    if (!FontParseXLFDName (lowerName, &vals, FONT_XLFD_REPLACE_ZERO) ||
	!(tmpName.length = strlen (lowerName),
	  entry = FontFileFindNameInScalableDir (&dir->scalable, &tmpName,
						 &vals)))
    {
	CopyISOLatin1Lowered (lowerName, name, namelen);
	lowerName[namelen] = '\0';
	tmpName.name = lowerName;
	tmpName.length = namelen;
	tmpName.ndashes = FontFileCountDashes (lowerName, namelen);
	entry = FontFileFindNameInScalableDir (&dir->scalable, &tmpName, &vals);
	if (entry)
	{
	    strcpy(lowerName, entry->name.name);
	    tmpName.name = lowerName;
	    tmpName.length = entry->name.length;
	    tmpName.ndashes = entry->name.ndashes;
	}
    }
    if (entry)
    {
	noSpecificSize = FALSE;	/* TRUE breaks XLFD enhancements */
    	if (entry->type == FONT_ENTRY_SCALABLE &&
	    FontFileCompleteXLFD (&vals, &entry->u.scalable.extra->defaults))
	{
	    scalable = &entry->u.scalable;
	    if ((vals.values_supplied & PIXELSIZE_MASK) == PIXELSIZE_ARRAY ||
		(vals.values_supplied & POINTSIZE_MASK) == POINTSIZE_ARRAY ||
		(vals.values_supplied &
		 ~SIZE_SPECIFY_MASK & ~CHARSUBSET_SPECIFIED))
		scaled = 0;
	    else
	        scaled = FontFileFindScaledInstance (entry, &vals,
						     noSpecificSize);
	    /*
	     * A scaled instance can occur one of two ways:
	     *
	     *  Either the font has been scaled to this
	     *   size already, in which case scaled->pFont
	     *   will point at that font.
	     *
	     *  Or a bitmap instance in this size exists,
	     *   which is handled as if we got a pattern
	     *   matching the bitmap font name.
	     */
	    if (scaled)
	    {
		if (scaled->pFont)
		{
		    *pFont = scaled->pFont;
		    ret = Successful;
		}
		else if (scaled->bitmap)
		{
		    entry = scaled->bitmap;
		    bitmap = &entry->u.bitmap;
		    if (bitmap->pFont)
		    {
			*pFont = bitmap->pFont;
			ret = Successful;
		    }
		    else
		    {
			ret = FontFileOpenBitmap (fpe, pFont, flags, entry,
						  format, fmask);
		    }
		}
		else /* "cannot" happen */
		{
		    ret = BadFontName;
		}
	    }
	    else
	    {
		ret = FontFileMatchBitmapSource (fpe, pFont, flags, entry, &tmpName, &vals, format, fmask, noSpecificSize);
		if (ret != Successful)
		{
		    char origName[MAXFONTNAMELEN];

		    CopyISOLatin1Lowered (origName, name, namelen);
		    origName[namelen] = '\0';

		    /* Pass the original XLFD name in the vals
		       structure; the rasterizer is free to examine it
		       for hidden meanings.  This information will not
		       be saved in the scaled-instances table.  */

		    vals.xlfdName = origName;
		    vals.ranges = ranges;
		    vals.nranges = nranges;

		    strcpy (fileName, dir->directory);
		    strcat (fileName, scalable->fileName);
		    ret = (*scalable->renderer->OpenScalable) (fpe, pFont,
			   flags, entry, fileName, &vals, format, fmask);

		    /* In case rasterizer does something bad because of
		       charset subsetting... */
		    if (ret == Successful &&
			((*pFont)->info.firstCol > (*pFont)->info.lastCol ||
			 (*pFont)->info.firstRow > (*pFont)->info.lastRow))
		    {
			(*(*pFont)->unload_font)(*pFont);
			ret = BadFontName;
		    }
		    /* Save the instance */
		    if (ret == Successful)
		    {
		    	if (FontFileAddScaledInstance (entry, &vals,
						    *pFont, (char *) 0))
			    ranges = 0;
			else
			    (*pFont)->fpePrivate = (pointer) 0;
		    }
		}
	    }
	    if (ret == Successful)
	    {
		if (ranges) xfree(ranges);
		return ret;
	    }
	}
    }

    /* Match non-scalable pattern */
    CopyISOLatin1Lowered (lowerName, name, namelen);
    lowerName[namelen] = '\0';
    tmpName.name = lowerName;
    tmpName.length = namelen;
    tmpName.ndashes = FontFileCountDashes (lowerName, namelen);
    if (!(entry = FontFileFindNameInDir (&dir->nonScalable, &tmpName)) &&
	tmpName.ndashes == 14 &&
	FontParseXLFDName (lowerName, &vals, FONT_XLFD_REPLACE_ZERO))
    {
        tmpName.length = strlen(lowerName);
	entry = FontFileFindNameInDir (&dir->nonScalable, &tmpName);
    }
    if ((!entry || entry->type != FONT_ENTRY_ALIAS) && ranges)
	xfree(ranges);
    if (entry)
    {
	static char aliasname[MAXFONTNAMELEN];
	int len;
	switch (entry->type) {
	case FONT_ENTRY_BITMAP:
	    bitmap = &entry->u.bitmap;
	    if (bitmap->pFont)
	    {
	    	*pFont = bitmap->pFont;
	    	ret = Successful;
	    }
	    else
	    {
		ret = FontFileOpenBitmap (fpe, pFont, flags, entry, format, fmask);
	    }
	    break;
	case FONT_ENTRY_ALIAS:
	    alias = &entry->u.alias;
	    *aliasName = alias->resolved;
	    if ((len = strlen(*aliasName)) <= MAXFONTNAMELEN &&
		FontFileCountDashes (*aliasName, len) == 14)
	    {
		FontScalableRec	tmpVals;

		/* If we're aliasing a scalable name, transfer values
		   from the name into the destination alias, multiplying
		   by matrices that appear in the alias. */

		CopyISOLatin1Lowered (lowerName, entry->name.name,
				      entry->name.length);
		lowerName[entry->name.length] = '\0';

		if (FontParseXLFDName(lowerName, &tmpVals,
				      FONT_XLFD_REPLACE_NONE) &&
		    !tmpVals.values_supplied &&
		    FontParseXLFDName(*aliasName, &tmpVals,
				      FONT_XLFD_REPLACE_NONE))
		{
		    double *matrix = 0, tempmatrix[4];
		    int nameok = 1;

		    vals.nranges = nranges;
		    vals.ranges = ranges;

		    /* Use a matrix iff exactly one is defined */
		    if ((tmpVals.values_supplied & PIXELSIZE_MASK) ==
			PIXELSIZE_ARRAY &&
			!(tmpVals.values_supplied & POINTSIZE_MASK))
			matrix = tmpVals.pixel_matrix;
		    else if ((tmpVals.values_supplied & POINTSIZE_MASK) ==
			     POINTSIZE_ARRAY &&
			     !(tmpVals.values_supplied & PIXELSIZE_MASK))
			matrix = tmpVals.point_matrix;

		    /* If matrix given in the alias, compute new point
		       and/or pixel matrices */
		    if (matrix)
		    {
			/* Complete the XLFD name to avoid potential
			   gotchas */
			if (FontFileCompleteXLFD(&vals, &vals))
			{
			    double hypot();
			    tempmatrix[0] =
				vals.point_matrix[0] * matrix[0] +
				vals.point_matrix[1] * matrix[2];
			    tempmatrix[1] =
				vals.point_matrix[0] * matrix[1] +
				vals.point_matrix[1] * matrix[3];
			    tempmatrix[2] =
				vals.point_matrix[2] * matrix[0] +
				vals.point_matrix[3] * matrix[2];
			    tempmatrix[3] =
				vals.point_matrix[2] * matrix[1] +
				vals.point_matrix[3] * matrix[3];
			    vals.point_matrix[0] = tempmatrix[0];
			    vals.point_matrix[1] = tempmatrix[1];
			    vals.point_matrix[2] = tempmatrix[2];
			    vals.point_matrix[3] = tempmatrix[3];

			    tempmatrix[0] =
				vals.pixel_matrix[0] * matrix[0] +
				vals.pixel_matrix[1] * matrix[2];
			    tempmatrix[1] =
				vals.pixel_matrix[0] * matrix[1] +
				vals.pixel_matrix[1] * matrix[3];
			    tempmatrix[2] =
				vals.pixel_matrix[2] * matrix[0] +
				vals.pixel_matrix[3] * matrix[2];
			    tempmatrix[3] =
				vals.pixel_matrix[2] * matrix[1] +
				vals.pixel_matrix[3] * matrix[3];
			    vals.pixel_matrix[0] = tempmatrix[0];
			    vals.pixel_matrix[1] = tempmatrix[1];
			    vals.pixel_matrix[2] = tempmatrix[2];
			    vals.pixel_matrix[3] = tempmatrix[3];

			    vals.values_supplied =
				(vals.values_supplied &
				 ~(PIXELSIZE_MASK | POINTSIZE_MASK)) |
				PIXELSIZE_ARRAY | POINTSIZE_ARRAY;

			    nameok = hypot(vals.point_matrix[0],
					   vals.point_matrix[1]) > EPS &&
				     hypot(vals.point_matrix[2],
					   vals.point_matrix[3]) > EPS &&
				     hypot(vals.pixel_matrix[0],
					   vals.pixel_matrix[1]) > EPS &&
				     hypot(vals.pixel_matrix[2],
					   vals.pixel_matrix[3]) > EPS;
			}
			else
			    nameok = 0;
		    }

		    CopyISOLatin1Lowered (aliasname, *aliasName, len + 1);
		    if (nameok && FontParseXLFDName(aliasname, &vals,
					  FONT_XLFD_REPLACE_VALUE))
			/* Return a version of the aliasname that has
			   had the vals stuffed into it.  To avoid
			   memory leak, this alias name lives in a
			   static buffer.  The caller appears to require
			   the buffer only very briefly, avoiding
			   contention from non-reentrancy.  */
			    *aliasName = aliasname;
		}
	    }
	    if (ranges) xfree(ranges);
	    ret = FontNameAlias;
	    break;
	case FONT_ENTRY_BC:
	    bc = &entry->u.bc;
	    entry = bc->entry;
	    ret = (*scalable->renderer->OpenScalable)
		    (fpe, pFont, flags, entry, &bc->vals, format, fmask);
	    break;
	default:
	    ret = BadFontName;
	}
    }
    else
    {
	ret = BadFontName;
    }
    return ret;
}

/* ARGSUSED */
FontFileCloseFont (fpe, pFont)
    FontPathElementPtr	fpe;
    FontPtr		pFont;
{
    FontEntryPtr    entry;

    if (entry = (FontEntryPtr) pFont->fpePrivate) {
	switch (entry->type) {
	case FONT_ENTRY_SCALABLE:
	    FontFileRemoveScaledInstance (entry, pFont);
	    break;
	case FONT_ENTRY_BITMAP:
	    entry->u.bitmap.pFont = 0;
	    break;
	default:
	    /* "cannot" happen */
	    break;
	}
	pFont->fpePrivate = 0;
    }
    (*pFont->unload_font) (pFont);
}

FontFileOpenBitmap (fpe, pFont, flags, entry, format, fmask)
    FontPathElementPtr	fpe;
    int			flags;
    FontEntryPtr	entry;
    FontPtr		*pFont;
{
    FontBitmapEntryPtr	bitmap;
    char		fileName[MAXFONTFILENAMELEN*2+1];
    int			ret;
    FontDirectoryPtr	dir;

    dir = (FontDirectoryPtr) fpe->private;
    bitmap = &entry->u.bitmap;
    strcpy (fileName, dir->directory);
    strcat (fileName, bitmap->fileName);
    ret = (*bitmap->renderer->OpenBitmap) 
			(fpe, pFont, flags, entry, fileName, format, fmask);
    if (ret == Successful)
    {
	bitmap->pFont = *pFont;
	(*pFont)->fpePrivate = (pointer) entry;
    }
    return ret;
}

FontFileGetInfoBitmap (fpe, pFontInfo, entry)
    FontPathElementPtr	fpe;
    FontInfoPtr		pFontInfo;
    FontEntryPtr	entry;
{
    FontBitmapEntryPtr	bitmap;
    char		fileName[MAXFONTFILENAMELEN*2+1];
    int			ret;
    FontDirectoryPtr	dir;

    dir = (FontDirectoryPtr) fpe->private;
    bitmap = &entry->u.bitmap;
    strcpy (fileName, dir->directory);
    strcat (fileName, bitmap->fileName);
    ret = (*bitmap->renderer->GetInfoBitmap) (fpe, pFontInfo, entry, fileName);
    return ret;
}

/* ARGSUSED */
FontFileListFonts (client, fpe, pat, len, max, names)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pat;
    int         len;
    int         max;
    FontNamesPtr names;
{
    FontDirectoryPtr	dir;
    char		lowerChars[MAXFONTNAMELEN], zeroChars[MAXFONTNAMELEN];
    FontNameRec		lowerName;
    FontNameRec		zeroName;
    FontNamesPtr	scaleNames;
    FontScalableRec	vals, zeroVals, tmpVals;
    int			i;
    int			oldnnames;
    fsRange		*ranges;
    int			nranges;

    if (len >= MAXFONTNAMELEN)
	return AllocError;
    dir = (FontDirectoryPtr) fpe->private;
    CopyISOLatin1Lowered (lowerChars, pat, len);
    lowerChars[len] = '\0';
    lowerName.name = lowerChars;
    lowerName.length = len;
    lowerName.ndashes = FontFileCountDashes (lowerChars, len);
    ranges = FontParseRanges(lowerChars, &nranges);
    /* Match XLFD patterns */
    strcpy (zeroChars, lowerChars);
    if (lowerName.ndashes == 14 &&
	FontParseXLFDName (zeroChars, &vals, FONT_XLFD_REPLACE_ZERO))
    {
	oldnnames = names->nnames;
	scaleNames = MakeFontNamesRecord (0);
	if (!scaleNames)
	{
	    if (ranges) xfree(ranges);
	    return AllocError;
	}
	zeroName.name = zeroChars;
	zeroName.length = strlen (zeroChars);
	zeroName.ndashes = lowerName.ndashes;
	FontFileFindNamesInScalableDir (&dir->scalable, &zeroName, max,
					scaleNames, &vals);
	/* Look for scalable aliases.  Because we do not track down the
	   font to which this alias resolves, we have no way of knowing
	   what enhanced XLFD capabilities are supported.  So we punt
	   and disallow any enhanced fields in aliases (the dirty work
	   is done in FontFileFindNamesInScalableDir()).  A more robust
	   implementation would track down the target of this alias and
	   allow enhancements into the alias name based on the
	   capabilities of the target font */
	FontFileFindNamesInScalableDir (&dir->nonScalable, &zeroName,
					max - scaleNames->nnames, scaleNames,
					&vals);
	for (i = 0; i < scaleNames->nnames; i++)
	{
	    FontParseXLFDName (scaleNames->names[i], &zeroVals, FONT_XLFD_REPLACE_NONE);
	    tmpVals = vals;
	    if (FontFileCompleteXLFD (&tmpVals, &zeroVals))
	    {
		strcpy (zeroChars, scaleNames->names[i]);
		if ((vals.values_supplied & PIXELSIZE_MASK) ||
		    !(vals.values_supplied & PIXELSIZE_WILDCARD))
		{
		    tmpVals.values_supplied =
			(tmpVals.values_supplied & ~PIXELSIZE_MASK) |
			(vals.values_supplied & PIXELSIZE_MASK);
		    tmpVals.pixel_matrix[0] = vals.pixel_matrix[0];
		    tmpVals.pixel_matrix[1] = vals.pixel_matrix[1];
		    tmpVals.pixel_matrix[2] = vals.pixel_matrix[2];
		    tmpVals.pixel_matrix[3] = vals.pixel_matrix[3];
		}
		if ((vals.values_supplied & POINTSIZE_MASK) ||
		    !(vals.values_supplied & POINTSIZE_WILDCARD))
		{
		    tmpVals.values_supplied =
			(tmpVals.values_supplied & ~POINTSIZE_MASK) |
			(vals.values_supplied & POINTSIZE_MASK);
		    tmpVals.point_matrix[0] = vals.point_matrix[0];
		    tmpVals.point_matrix[1] = vals.point_matrix[1];
		    tmpVals.point_matrix[2] = vals.point_matrix[2];
		    tmpVals.point_matrix[3] = vals.point_matrix[3];
		}
		if (vals.width <= 0)
		    tmpVals.width = 0;
		if (vals.x == 0)
		    tmpVals.x = 0;
		if (vals.y == 0)
		    tmpVals.y = 0;
		tmpVals.ranges = ranges;
		tmpVals.nranges = nranges;
		FontParseXLFDName (zeroChars, &tmpVals, FONT_XLFD_REPLACE_VALUE);
		(void) AddFontNamesName (names, zeroChars, strlen (zeroChars));
	    }
	}
	FreeFontNames (scaleNames);
	max -= names->nnames - oldnnames;
    }
    else
    {
    	oldnnames = names->nnames;
    	FontFileFindNamesInScalableDir (&dir->scalable, &lowerName, max,
					names, &vals);
	max -= names->nnames - oldnnames;
    }
    if (ranges) xfree(ranges);
    return FontFileFindNamesInDir (&dir->nonScalable, &lowerName, max, names);
}

typedef struct _LFWIData {
    FontNamesPtr    names;
    int		    current;
} LFWIDataRec, *LFWIDataPtr;

FontFileStartListFontsWithInfo(client, fpe, pat, len, max, privatep)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pat;
    int         len;
    int         max;
    pointer    *privatep;
{
    LFWIDataPtr	data;
    int		ret;

    data = (LFWIDataPtr) xalloc (sizeof *data);
    if (!data)
	return AllocError;
    data->names = MakeFontNamesRecord (0);
    if (!data->names)
    {
	xfree (data);
	return AllocError;
    }
    ret = FontFileListFonts (client, fpe, pat, len, max, data->names);
    if (ret != Successful)
    {
	FreeFontNames (data->names);
	xfree (data);
	return ret;
    }
    data->current = 0;
    *privatep = (pointer) data;
    return Successful;
}

/* ARGSUSED */
static int
FontFileListOneFontWithInfo (client, fpe, namep, namelenp, pFontInfo)
    pointer		client;
    FontPathElementPtr	fpe;
    char		**namep;
    int			*namelenp;
    FontInfoPtr		*pFontInfo;
{
    FontDirectoryPtr	dir;
    char		lowerName[MAXFONTNAMELEN];
    char		fileName[MAXFONTFILENAMELEN*2 + 1];
    FontNameRec		tmpName;
    FontEntryPtr	entry;
    FontScalableRec	vals;
    FontScalableEntryPtr   scalable;
    FontScaledPtr	scaled;
    FontBitmapEntryPtr	bitmap;
    FontAliasEntryPtr	alias;
    int			ret;
    char		*name = *namep;
    int			namelen = *namelenp;
    Bool		noSpecificSize;
    
    if (namelen >= MAXFONTNAMELEN)
	return AllocError;
    dir = (FontDirectoryPtr) fpe->private;
    CopyISOLatin1Lowered (lowerName, name, namelen);
    lowerName[namelen] = '\0';
    tmpName.name = lowerName;
    tmpName.length = namelen;
    tmpName.ndashes = FontFileCountDashes (lowerName, namelen);
    /* Match XLFD patterns */
    if (tmpName.ndashes == 14 &&
	FontParseXLFDName (lowerName, &vals, FONT_XLFD_REPLACE_ZERO))
    {
	tmpName.length = strlen (lowerName);
	entry = FontFileFindNameInScalableDir (&dir->scalable, &tmpName, &vals);
	noSpecificSize = FALSE;	/* TRUE breaks XLFD enhancements */
    	if (entry && entry->type == FONT_ENTRY_SCALABLE &&
	    FontFileCompleteXLFD (&vals, &entry->u.scalable.extra->defaults))
	{
	    scalable = &entry->u.scalable;
	    scaled = FontFileFindScaledInstance (entry, &vals, noSpecificSize);
	    /*
	     * A scaled instance can occur one of two ways:
	     *
	     *  Either the font has been scaled to this
	     *   size already, in which case scaled->pFont
	     *   will point at that font.
	     *
	     *  Or a bitmap instance in this size exists,
	     *   which is handled as if we got a pattern
	     *   matching the bitmap font name.
	     */
	    if (scaled)
	    {
		if (scaled->pFont)
		{
		    *pFontInfo = &scaled->pFont->info;
		    ret = Successful;
		}
		else if (scaled->bitmap)
		{
		    entry = scaled->bitmap;
		    bitmap = &entry->u.bitmap;
		    if (bitmap->pFont)
		    {
			*pFontInfo = &bitmap->pFont->info;
			ret = Successful;
		    }
		    else
		    {
			ret = FontFileGetInfoBitmap (fpe, *pFontInfo, entry);
		    }
		}
		else /* "cannot" happen */
		{
		    ret = BadFontName;
		}
	    }
	    else
	    {
#ifdef NOTDEF
		/* no special case yet */
		ret = FontFileMatchBitmapSource (fpe, pFont, flags, entry, &vals, format, fmask, noSpecificSize);
		if (ret != Successful)
#endif
		{
		    char origName[MAXFONTNAMELEN];
		    fsRange *ranges;

		    CopyISOLatin1Lowered (origName, name, namelen);
		    origName[namelen] = '\0';
		    vals.xlfdName = origName;
		    vals.ranges = FontParseRanges(origName, &vals.nranges);
		    ranges = vals.ranges;
		    /* Make a new scaled instance */
	    	    strcpy (fileName, dir->directory);
	    	    strcat (fileName, scalable->fileName);
	    	    ret = (*scalable->renderer->GetInfoScalable)
			(fpe, *pFontInfo, entry, &tmpName, fileName, &vals);
		    if (ranges) xfree(ranges);
		}
	    }
	    if (ret == Successful) return ret;
	}
	CopyISOLatin1Lowered (lowerName, name, namelen);
	tmpName.length = namelen;
    }
    /* Match non XLFD pattern */
    if (entry = FontFileFindNameInDir (&dir->nonScalable, &tmpName))
    {
	switch (entry->type) {
	case FONT_ENTRY_BITMAP:
	    bitmap = &entry->u.bitmap;
	    if (bitmap->pFont)
	    {
	    	*pFontInfo = &bitmap->pFont->info;
	    	ret = Successful;
	    }
	    else
	    {
		ret = FontFileGetInfoBitmap (fpe, *pFontInfo, entry);
	    }
	    break;
	case FONT_ENTRY_ALIAS:
	    alias = &entry->u.alias;
	    *(char **)pFontInfo = name;
	    *namelenp = strlen (*namep = alias->resolved);
	    ret = FontNameAlias;
	    break;
	case FONT_ENTRY_BC:
#ifdef NOTYET
	    /* no LFWI for this yet */
	    bc = &entry->u.bc;
	    entry = bc->entry;
	    /* Make a new scaled instance */
    	    strcpy (fileName, dir->directory);
    	    strcat (fileName, scalable->fileName);
	    ret = (*scalable->renderer->GetInfoScalable)
		    (fpe, *pFontInfo, entry, tmpName, fileName, &bc->vals);
#endif
	    break;
	default:
	    ret = BadFontName;
	}
    }
    else
    {
	ret = BadFontName;
    }
    return ret;
}

FontFileListNextFontWithInfo(client, fpe, namep, namelenp, pFontInfo,
			     numFonts, private)
    pointer		client;
    FontPathElementPtr	fpe;
    char		**namep;
    int			*namelenp;
    FontInfoPtr		*pFontInfo;
    int			*numFonts;
    pointer		private;
{
    LFWIDataPtr	data = (LFWIDataPtr) private;
    int		ret;
    char	*name;
    int		namelen;

    if (data->current == data->names->nnames)
    {
	FreeFontNames (data->names);
	xfree (data);
	return BadFontName;
    }
    name = data->names->names[data->current];
    namelen = data->names->length[data->current];
    ret = FontFileListOneFontWithInfo (client, fpe, &name, &namelen, pFontInfo);
    if (ret == BadFontName)
	ret = AllocError;
    *namep = name;
    *namelenp = namelen;
    ++data->current;
    *numFonts = data->names->nnames - data->current;
    return ret;
}

typedef int (*IntFunc) ();
static int  font_file_type;

FontFileRegisterFpeFunctions()
{
    static Bool beenhere = FALSE;

    if (!beenhere) {
	FontFileRegisterFontFileFunctions ();
	beenhere = TRUE;
    }
    font_file_type = RegisterFPEFunctions(FontFileNameCheck,
					  FontFileInitFPE,
					  FontFileFreeFPE,
					  FontFileResetFPE,
					  FontFileOpenFont,
					  FontFileCloseFont,
					  FontFileListFonts,
					  FontFileStartListFontsWithInfo,
					  FontFileListNextFontWithInfo,
					  (IntFunc) 0,
					  (IntFunc) 0,
					  (IntFunc) 0);
}
