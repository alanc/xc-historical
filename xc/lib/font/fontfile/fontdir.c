/* $XConsortium: fontdir.c,v 1.23 95/02/21 14:25:17 mor Exp $ */

/*

Copyright (c) 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#include    "fntfilst.h"
#include    <X11/keysym.h>

Bool
FontFileInitTable (table, size)
    FontTablePtr    table;
    int		    size;
{
    if (size)
    {
	table->entries = (FontEntryPtr) xalloc(sizeof(FontEntryRec) * size);
	if (!table->entries)
	    return FALSE;
    }
    else
	table->entries = 0;
    table->used = 0;
    table->size = size;
    table->sorted = FALSE;
    return TRUE;
}

FontFileFreeEntry (entry)
    FontEntryPtr    entry;
{
    FontScalableExtraPtr   extra;
    int i;

    if (entry->name.name)
	xfree(entry->name.name);

    switch (entry->type)
    {
    case FONT_ENTRY_SCALABLE:
	xfree (entry->u.scalable.fileName);
	extra = entry->u.scalable.extra;
	for (i = 0; i < extra->numScaled; i++)
	    if (extra->scaled[i].vals.ranges)
		free (extra->scaled[i].vals.ranges);
	xfree (extra->scaled);
	xfree (extra);
	break;
    case FONT_ENTRY_BITMAP:
	xfree (entry->u.bitmap.fileName);
	break;
    case FONT_ENTRY_ALIAS:
	xfree (entry->u.alias.resolved);
	break;
#ifdef NOTYET
    case FONT_ENTRY_BC:
	break;
#endif
    }
}

FontFileFreeTable (table)
    FontTablePtr    table;
{
    int	i;

    for (i = 0; i < table->used; i++)
	FontFileFreeEntry (&table->entries[i]);
    xfree (table->entries);
}

FontDirectoryPtr
FontFileMakeDir(dirName, size)
    char       *dirName;
    int         size;
{
    FontDirectoryPtr	dir;
    int			dirlen;
    int			needslash = 0;

    dirlen = strlen(dirName);
    if (dirName[dirlen - 1] != '/')
#ifdef NCD
    if (dirlen)     /* leave out slash for builtins */
#endif
	needslash = 1;
    dir = (FontDirectoryPtr) xalloc(sizeof *dir + dirlen + needslash + 1);
    if (!dir)
	return (FontDirectoryPtr)0;
    if (!FontFileInitTable (&dir->scalable, 0))
    {
	xfree (dir);
	return (FontDirectoryPtr)0;
    }
    if (!FontFileInitTable (&dir->nonScalable, size))
    {
	FontFileFreeTable (&dir->scalable);
	xfree (dir);
	return (FontDirectoryPtr)0;
    }
    dir->directory = (char *) (dir + 1);
    dir->dir_mtime = 0;
    dir->alias_mtime = 0;
    strcpy(dir->directory, dirName);
    if (needslash)
	strcat(dir->directory, "/");
    return dir;
}

FontFileFreeDir (dir)
    FontDirectoryPtr	dir;
{
    FontFileFreeTable (&dir->scalable);
    FontFileFreeTable (&dir->nonScalable);
    xfree(dir);
}

FontEntryPtr
FontFileAddEntry(table, prototype)
    FontTablePtr	table;
    FontEntryPtr	prototype;
{
    FontEntryPtr    entry;
    int		    newsize;

    /* can't add entries to a sorted table, pointers get broken! */
    if (table->sorted)
	return (FontEntryPtr) 0;    /* "cannot" happen */
    if (table->used == table->size) {
	newsize = table->size + 100;
	entry = (FontEntryPtr) xrealloc(table->entries,
					   newsize * sizeof(FontEntryRec));
	if (!entry)
	    return (FontEntryPtr)0;
	table->size = newsize;
	table->entries = entry;
    }
    entry = &table->entries[table->used];
    *entry = *prototype;
    entry->name.name = (char *) xalloc(prototype->name.length + 1);
    if (!entry->name.name)
	return (FontEntryPtr)0;
    memcpy (entry->name.name, prototype->name.name, prototype->name.length);
    entry->name.name[entry->name.length] = '\0';
    table->used++;
    return entry;
}

static int
FontFileNameCompare(a, b)
    char       *a,
               *b;
{
    FontEntryPtr    a_name = (FontEntryPtr) a,
		    b_name = (FontEntryPtr) b;

    return strcmp(a_name->name.name, b_name->name.name);
}

FontFileSortTable (table)
    FontTablePtr    table;
{
    if (!table->sorted) {
	qsort((char *) table->entries, table->used, sizeof(FontEntryRec),
	      FontFileNameCompare);
	table->sorted = TRUE;
    }
}

FontFileSortDir(dir)
    FontDirectoryPtr	dir;
{
    FontFileSortTable (&dir->scalable);
    FontFileSortTable (&dir->nonScalable);
    /* now that the table is fixed in size, swizzle the pointers */
    FontFileSwitchStringsToBitmapPointers (dir);
}

/*
  Given a Font Table, SetupWildMatch() sets up various pointers and state
  information so the table can be searched for name(s) that match a given
  fontname pattern -- which may contain wildcards.  Under certain
  circumstances, SetupWildMatch() will find the one table entry that
  matches the pattern.  If those circumstances do not pertain,
  SetupWildMatch() returns a range within the the table that should be
  searched for matching name(s).  With the information established by
  SetupWildMatch(), including state information in "private", the
  PatternMatch() procedure is then used to test names in the range for a
  match.
*/

#define isWild(c)   ((c) == XK_asterisk || (c) == XK_question)

static int
SetupWildMatch(table, pat, leftp, rightp, privatep)
    FontTablePtr    table;
    FontNamePtr	    pat;
    int		    *leftp,
		    *rightp;
    int		    *privatep;
{
    int         nDashes;
    char        c;
    char       *t;
    char       *firstWild;
    int         first;
    int         center,
                left,
                right;
    int         result;
    char	*name;

    name = pat->name;
    nDashes = pat->ndashes;
    firstWild = 0;
    t = name;
    while (c = *t++) {
	if (isWild(c)) {
	    if (!firstWild)
		firstWild = t - 1;
	}
    }
    left = 0;
    right = table->used;
    if (firstWild)
	*privatep = nDashes;
    else
	*privatep = -1;
    if (!table->sorted) {
	*leftp = left;
	*rightp = right;
	return -1;
    } else if (firstWild) {
	first = firstWild - name;
	while (left < right) {
	    center = (left + right) / 2;
	    result = strncmp(name, table->entries[center].name.name, first);
	    if (result == 0)
		break;
	    if (result < 0)
		right = center;
	    else
		left = center + 1;
	}
	*leftp = left;
	*rightp = right;
	return -1;
    } else {
	while (left < right) {
	    center = (left + right) / 2;
	    result = strcmp(name, table->entries[center].name.name);
	    if (result == 0)
		return center;
	    if (result < 0)
		right = center;
	    else
		left = center + 1;
	}
	*leftp = 1;
	*rightp = 0;
	return -1;
    }
}

static
PatternMatch(pat, patdashes, string, stringdashes)
    char       *pat;
    char       *string;
{
    char        c,
                t;

    if (stringdashes < patdashes)
	return 0;
    for (;;) {
	switch (c = *pat++) {
	case '*':
	    if (!(c = *pat++))
		return 1;
	    if (c == XK_minus) {
		patdashes--;
		for (;;) {
		    while ((t = *string++) != XK_minus)
			if (!t)
			    return 0;
		    stringdashes--;
		    if (PatternMatch(pat, patdashes, string, stringdashes))
			return 1;
		    if (stringdashes == patdashes)
			return 0;
		}
	    } else {
		for (;;) {
		    while ((t = *string++) != c) {
			if (!t)
			    return 0;
			if (t == XK_minus) {
			    if (stringdashes-- < patdashes)
				return 0;
			}
		    }
		    if (PatternMatch(pat, patdashes, string, stringdashes))
			return 1;
		}
	    }
	case '?':
	    if (*string++ == XK_minus)
		stringdashes--;
	    break;
	case '\0':
	    return (*string == '\0');
	case XK_minus:
	    if (*string++ == XK_minus) {
		patdashes--;
		stringdashes--;
		break;
	    }
	    return 0;
	default:
	    if (c == *string++)
		break;
	    return 0;
	}
    }
}

int
FontFileCountDashes (name, namelen)
    char    *name;
    int	    namelen;
{
    int	ndashes = 0;

    while (namelen--)
	if (*name++ == '\055')	/* avoid non ascii systems */
	    ++ndashes;
    return ndashes;
}

char *
FontFileSaveString (s)
    char    *s;
{
    char    *n;

    n = (char *) xalloc (strlen (s) + 1);
    if (!n)
	return 0;
    strcpy (n, s);
    return n;
}

FontEntryPtr
FontFileFindNameInScalableDir(table, pat, vals)
    FontTablePtr    table;
    FontNamePtr	    pat;
    FontScalablePtr	vals;
{
    int         i,
                start,
                stop,
                res,
                private;
    FontNamePtr	name;

    if ((i = SetupWildMatch(table, pat, &start, &stop, &private)) >= 0)
	return &table->entries[i];
    for (i = start; i < stop; i++) {
	name = &table->entries[i].name;
	res = PatternMatch(pat->name, private, name->name, name->ndashes);
	if (res > 0)
	{
	    /* Check to see if enhancements requested are available */
	    if (vals)
	    {
		int vs = vals->values_supplied;
		int cap;

		if (table->entries[i].type == FONT_ENTRY_SCALABLE)
		    cap = table->entries[i].u.scalable.renderer->capabilities;
		else if (table->entries[i].type == FONT_ENTRY_ALIAS)
		    cap = ~0;	/* Calling code will have to see if true */
		else
		    cap = 0;
		if (((vs & PIXELSIZE_MASK) == PIXELSIZE_ARRAY ||
		     (vs & POINTSIZE_MASK) == POINTSIZE_ARRAY) &&
		    !(cap & CAP_MATRIX) ||
		    (vs & CHARSUBSET_SPECIFIED) &&
		    !(cap & CAP_CHARSUBSETTING))
		    continue;
	    }
	    return &table->entries[i];
	}
	if (res < 0)
	    break;
    }
    return (FontEntryPtr)0;
}

FontEntryPtr
FontFileFindNameInDir(table, pat)
    FontTablePtr    table;
    FontNamePtr	    pat;
{
    return FontFileFindNameInScalableDir(table, pat, (FontScalablePtr)0);
}

FontFileFindNamesInScalableDir(table, pat, max, names, vals,
			       alias_behavior, newmax)
    FontTablePtr    table;
    FontNamePtr	    pat;
    int		    max;
    FontNamesPtr    names;
    FontScalablePtr	vals;
    int		    alias_behavior;
    int		   *newmax;
{
    int		    i,
		    start,
		    stop,
		    res,
		    private;
    int		    ret = Successful;
    FontEntryPtr    fname;
    FontNamePtr	    name;

    if (max <= 0)
	return Successful;
    if ((i = SetupWildMatch(table, pat, &start, &stop, &private)) >= 0) {
	if (alias_behavior == NORMAL_ALIAS_BEHAVIOR ||
	    table->entries[i].type != FONT_ENTRY_ALIAS)
	{
	    name = &table->entries[i].name;
	    if (newmax) *newmax = max - 1;
	    return AddFontNamesName(names, name->name, name->length);
	}
	start = i;
	stop = i + 1;
    }
    for (i = start, fname = &table->entries[start]; i < stop; i++, fname++) {
	res = PatternMatch(pat->name, private, fname->name.name, fname->name.ndashes);
	if (res > 0) {
	    if (vals)
	    {
		int vs = vals->values_supplied;
		int cap;

		if (fname->type == FONT_ENTRY_SCALABLE)
		    cap = fname->u.scalable.renderer->capabilities;
		else if (fname->type == FONT_ENTRY_ALIAS)
		    cap = ~0;	/* Calling code will have to see if true */
		else
		    cap = 0;
		if (((vs & PIXELSIZE_MASK) == PIXELSIZE_ARRAY ||
		     (vs & POINTSIZE_MASK) == POINTSIZE_ARRAY) &&
		    !(cap & CAP_MATRIX) ||
		    (vs & CHARSUBSET_SPECIFIED) &&
		    !(cap & CAP_CHARSUBSETTING))
		    continue;
	    }

	    if ((alias_behavior & IGNORE_SCALABLE_ALIASES) &&
		fname->type == FONT_ENTRY_ALIAS)
	    {
		FontScalableRec	tmpvals;
		if (FontParseXLFDName (fname->name.name, &tmpvals,
				       FONT_XLFD_REPLACE_NONE) &&
		    !(tmpvals.values_supplied & SIZE_SPECIFY_MASK))
		    continue;
	    }

	    ret = AddFontNamesName(names, fname->name.name, fname->name.length);
	    if (ret != Successful)
		goto bail;

	    /* If alias_behavior is LIST_ALIASES_AND_TARGET_NAMES, mark
	       this entry as an alias by negating its length and follow
	       it by the resolved name */
	    if ((alias_behavior & LIST_ALIASES_AND_TARGET_NAMES) &&
		fname->type == FONT_ENTRY_ALIAS)
	    {
		names->length[names->nnames - 1] =
		    -names->length[names->nnames - 1];
		ret = AddFontNamesName(names, fname->u.alias.resolved,
				       strlen(fname->u.alias.resolved));
		if (ret != Successful)
		    goto bail;
	    }

	    if (--max <= 0)
		break;
	} else if (res < 0)
	    break;
    }
  bail: ;
    if (newmax) *newmax = max;
    return ret;
}

int
FontFileFindNamesInDir(table, pat, max, names)
    FontTablePtr    table;
    FontNamePtr	    pat;
    int		    max;
    FontNamesPtr    names;
{
    return FontFileFindNamesInScalableDir(table, pat, max, names,
					  (FontScalablePtr)0,
					  NORMAL_ALIAS_BEHAVIOR, (int *)0);
}

Bool
FontFileMatchName(name, length, pat)
    char	*name;
    int		length;
    FontNamePtr	pat;
{
    /* Perform a fontfile-type name match on a single name */
    FontTableRec table;
    FontEntryRec entries[1];

    /* Dummy up a table */
    table.used = 1;
    table.size = 1;
    table.sorted = TRUE;
    table.entries = entries;
    entries[0].name.name = name;
    entries[0].name.length = length;
    entries[0].name.ndashes = FontFileCountDashes(name, length);

    return FontFileFindNameInDir(&table, pat) != (FontEntryPtr)0;
}

/*
 * Add a font file to a directory.  This handles bitmap and
 * scalable names both
 */

Bool
FontFileAddFontFile (dir, fontName, fileName)
    FontDirectoryPtr	dir;
    char		*fontName;
    char		*fileName;
{
    FontEntryRec	    entry;
    FontScalableRec	    vals, zeroVals;
    FontRendererPtr	    renderer;
    FontEntryPtr	    existing;
    FontScalableExtraPtr    extra;
    FontEntryPtr	    bitmap, scalable;
    Bool		    isscale;

    renderer = FontFileMatchRenderer (fileName);
    if (!renderer)
	return FALSE;
    entry.name.length = strlen (fontName);
    if (entry.name.length > MAXFONTNAMELEN)
	entry.name.length = MAXFONTNAMELEN;
    entry.name.name = fontName;
    CopyISOLatin1Lowered (entry.name.name, fontName, entry.name.length);
    entry.name.ndashes = FontFileCountDashes (entry.name.name, entry.name.length);
    entry.name.name[entry.name.length] = '\0';
    /*
     * Add a bitmap name if the incoming name isn't an XLFD name, or
     * if it isn't a scalable name (i.e. non-zero scalable fields)
     *
     * If name of bitmapped font contains XLFD enhancements, do not add
     * a scalable version of the name... this can lead to confusion and
     * ambiguity between the font name and the field enhancements.
     */
    isscale = entry.name.ndashes == 14 &&
	      FontParseXLFDName(entry.name.name,
				&vals, FONT_XLFD_REPLACE_NONE) &&
	      (vals.values_supplied & PIXELSIZE_MASK) != PIXELSIZE_ARRAY &&
	      (vals.values_supplied & POINTSIZE_MASK) != POINTSIZE_ARRAY &&
	      !(vals.values_supplied & ENHANCEMENT_SPECIFY_MASK);
    if (!isscale || (vals.values_supplied & SIZE_SPECIFY_MASK))
    {
      /* If the fontname says it is nonScalable, make sure that the
       * renderer supports OpenBitmap and GetInfoBitmap.
       */
      if (renderer->OpenBitmap && renderer->GetInfoBitmap)
      {
	entry.type = FONT_ENTRY_BITMAP;
	entry.u.bitmap.renderer = renderer;
	entry.u.bitmap.pFont = NullFont;
	if (!(entry.u.bitmap.fileName = FontFileSaveString (fileName)))
	    return FALSE;
	if (!(bitmap = FontFileAddEntry (&dir->nonScalable, &entry)))
	{
	    xfree (entry.u.bitmap.fileName);
	    return FALSE;
	}
      }
    }
    /*
     * Parse out scalable fields from XLFD names - a scalable name
     * just gets inserted, a scaled name has more things to do.
     */
    if (isscale)
    {
      /* If the fontname says it is scalable, make sure that the
       * renderer supports OpenScalable and GetInfoScalable.
       */
      if (renderer->OpenScalable && renderer->GetInfoScalable)
      {
	if (vals.values_supplied & SIZE_SPECIFY_MASK)
	{
	    bzero((char *)&zeroVals, sizeof(zeroVals));
	    zeroVals.x = vals.x;
	    zeroVals.y = vals.y;
	    zeroVals.values_supplied = PIXELSIZE_SCALAR | POINTSIZE_SCALAR;
	    FontParseXLFDName (entry.name.name, &zeroVals,
			       FONT_XLFD_REPLACE_VALUE);
	    entry.name.length = strlen (entry.name.name);
	    existing = FontFileFindNameInDir (&dir->scalable, &entry.name);
	    if (existing)
	    {
		if ((vals.values_supplied & POINTSIZE_MASK) ==
			POINTSIZE_SCALAR &&
		    (int)(vals.point_matrix[3] * 10) == GetDefaultPointSize())
		{
		    existing->u.scalable.extra->defaults = vals;

		    xfree (existing->u.scalable.fileName);
		    if (!(existing->u.scalable.fileName = FontFileSaveString (fileName)))
			return FALSE;
		}
		FontFileCompleteXLFD(&vals, &vals);
		FontFileAddScaledInstance (existing, &vals, NullFont,
					   bitmap->name.name);
		return TRUE;
	    }
	}
	if (!(entry.u.scalable.fileName = FontFileSaveString (fileName)))
	    return FALSE;
	extra = (FontScalableExtraPtr) xalloc (sizeof (FontScalableExtraRec));
	if (!extra)
	{
	    xfree (entry.u.scalable.fileName);
	    return FALSE;
	}
	bzero((char *)&extra->defaults, sizeof(extra->defaults));
	if ((vals.values_supplied & POINTSIZE_MASK) == POINTSIZE_SCALAR &&
	    (int)(vals.point_matrix[3] * 10) == GetDefaultPointSize())
	    extra->defaults = vals;
	else
	{
	    /* XXX fix interfaces */
	    struct resolution {
		CARD16	x B16;
		CARD16	y B16;
		CARD16	point_size B16;
	    } *resolution, *GetClientResolutions();
	    int num;

	    extra->defaults.point_matrix[0] =
		extra->defaults.point_matrix[3] =
		    (double)GetDefaultPointSize() / 10.0;
	    extra->defaults.point_matrix[1] =
		extra->defaults.point_matrix[2] = 0.0;
	    extra->defaults.values_supplied =
		POINTSIZE_SCALAR | PIXELSIZE_UNDEFINED;
	    extra->defaults.width = -1;
	    if (vals.x <= 0 || vals.y <= 0)
	    {
	        resolution = GetClientResolutions (&num);
	        if (resolution && num > 0)
	        {
	    	    extra->defaults.x = resolution->x;
	    	    extra->defaults.y = resolution->y;
	        }
	        else
	        {
		    extra->defaults.x = 75;
		    extra->defaults.y = 75;
	        }
	     }
	     else 
	     {
		extra->defaults.x = vals.x;
		extra->defaults.y = vals.y;
	     }
	     FontFileCompleteXLFD (&extra->defaults, &extra->defaults);
	}
	extra->numScaled = 0;
	extra->sizeScaled = 0;
	extra->scaled = 0;
	extra->private = 0;
	entry.type = FONT_ENTRY_SCALABLE;
	entry.u.scalable.renderer = renderer;
	entry.u.scalable.extra = extra;
	if (!(scalable = FontFileAddEntry (&dir->scalable, &entry)))
	{
	    xfree (extra);
	    xfree (entry.u.scalable.fileName);
	    return FALSE;
	}
	if (vals.values_supplied & SIZE_SPECIFY_MASK)
	{
	    FontFileCompleteXLFD(&vals, &vals);
	    FontFileAddScaledInstance (scalable, &vals, NullFont,
				       bitmap->name.name);
	}
      }
    }
    return TRUE;
}

Bool
FontFileAddFontAlias (dir, aliasName, fontName)
    FontDirectoryPtr	dir;
    char		*aliasName;
    char		*fontName;
{
    FontEntryRec	entry;

    entry.name.length = strlen (aliasName);
    CopyISOLatin1Lowered (aliasName, aliasName, entry.name.length);
    entry.name.name = aliasName;
    entry.name.ndashes = FontFileCountDashes (entry.name.name, entry.name.length);
    entry.type = FONT_ENTRY_ALIAS;
    if (!(entry.u.alias.resolved = FontFileSaveString (fontName)))
	return FALSE;
    if (!FontFileAddEntry (&dir->nonScalable, &entry))
    {
	xfree (entry.u.alias.resolved);
	return FALSE;
    }
    return TRUE;
}
