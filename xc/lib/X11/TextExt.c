/*
 * $XConsortium: XTextExt.c,v 1.5 89/06/07 12:04:25 jim Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 */


#include "Xlibint.h"

#define min_byte2 min_char_or_byte2
#define max_byte2 max_char_or_byte2

/* 
 * GetCharInfo1d - Return the charinfo struct for the indicated 8bit
 * character.  If the character is in the column and exists, then return the
 * appropriate metrics (note that fonts with common per-character metrics will
 * return min_bounds).  If none of these hold true, try again with the default
 * char.
 */

#define GetCharInfo1d(fs,col,def,cs) \
{ \
    cs = def; \
    if (col >= fs->min_byte2 && col <= fs->max_byte2) { \
	if (fs->per_char == NULL) { \
	    cs = &fs->min_bounds; \
	} else { \
	    cs = &fs->per_char[(col - fs->min_byte2)]; \
	    if (CI_NONEXISTCHAR(cs)) cs = def; \
	} \
    } \
}

#define GetDefaultCharInfo1d(fs,cs) \
  GetCharInfo1d (fs, fs->default_char, NULL, cs)


#define GetCharInfo2d(fs,row,col,def,cs) \
{ \
    cs = NULL; \
    if (row >= fs->min_byte1 && row <= fs->max_byte1 && \
	col >= fs->min_byte2 && col <= fs->max_byte2) { \
	if (fs->per_char == NULL) { \
	    cs = &fs->min_bounds; \
	} else { \
	    cs = &fs->per_char[((row - fs->min_byte1) * \
			        (fs->max_byte2 - fs->min_byte2 + 1)) + \
			       (col - fs->min_byte2)]; \
	    if (CI_NONEXISTCHAR(cs)) cs = NULL; \
        } \
    } \
}

#define GetDefaultCharInfo2d(fs,cs) \
{ \
    unsigned int r = (fs->default_char >> 8); \
    unsigned int c = (fs->default_char & 0xff); \
    GetCharInfo2d (fs, r, c, NULL, cs); \
}





/* 
 * GetRowZeroCharInfo2d - do the same thing as GetCharInfo1d, except that
 * the font has more than one row.  This is special case of more general
 * version used in XTextExt16.c since row == 0.  This is used when max_byte2 is
 * not zero.  A further optimization would do the check for min_byte1 being
 * zero ahead of time.
 */

#define GetRowZeroCharInfo2d(fs,col,def,cs) \
{ \
    cs = def; \
    if (fs->min_byte1 == 0 && \
	col >= fs->min_byte2 && col <= fs->max_byte2) { \
	if (fs->per_char == NULL) { \
	    cs = &fs->min_bounds; \
	} else { \
	    cs = &fs->per_char[(col - fs->min_byte2)]; \
	    if (CI_NONEXISTCHAR(cs)) cs = def; \
	} \
    } \
}


/*
 * XTextExtents - compute the extents of string given as a sequences of eight
 * bit bytes.  Since we know that the input characters will always be from the
 * first row of the font (i.e. byte1 == 0), we can do some optimizations beyond
 * what is done in XTextExtents16.
 */
XTextExtents (fs, string, nchars, dir, font_ascent, font_descent, overall)
    XFontStruct *fs;
    char *string;
    int nchars;
    int *dir, *font_ascent, *font_descent;  /* RETURN font information */
    register XCharStruct *overall;	/* RETURN character information */
{
    int i;				/* iterator */
    Bool singlerow = (fs->max_byte1 == 0);  /* optimization */
    int nfound = 0;			/* number of characters found */
    XCharStruct *def;			/* info about default char */
    unsigned char *us = (unsigned char *) string;  /* be 8bit clean */

    if (singlerow) {			/* optimization */
	GetDefaultCharInfo1d (fs, def);
    } else {
	GetDefaultCharInfo2d (fs, def);
    }

    *dir = fs->direction;
    *font_ascent = fs->ascent;
    *font_descent = fs->descent;

    /*
     * Iterate over the input string getting the appropriate * char struct.
     * The default (which may be null if there is no def_char) will be returned
     * if the character doesn't exist.  On the first time * through the loop,
     * assign the values to overall; otherwise, compute * the new values.
     */

    for (i = 0, us = (unsigned char *) string; i < nchars; i++, us++) {
	register unsigned uc = (unsigned) *us;	/* since about to do macro */
	register XCharStruct *cs;

	if (singlerow) {		/* optimization */
	    GetCharInfo1d (fs, uc, def, cs);
	} else {
	    GetRowZeroCharInfo2d (fs, uc, def, cs);
	}

	if (cs) {
	    if (nfound++ == 0) {
		*overall = *cs;
	    } else {
		overall->ascent = max (overall->ascent, cs->ascent);
		overall->descent = max (overall->descent, cs->descent);
		overall->lbearing = min (overall->lbearing, 
					 overall->width + cs->lbearing);
		overall->rbearing = max (overall->rbearing,
					 overall->width + cs->rbearing);
		overall->width += cs->width;
	    }
	}
    }

    /*
     * if there were no characters, then set everything to 0
     */
    if (nfound == 0) {
	overall->width = overall->ascent = overall->descent = 
	  overall->lbearing = overall->rbearing = 0;
    }

    return;
}


/*
 * XTextWidth - compute the width of a string of eightbit bytes.  This is a 
 * subset of XTextExtents.
 */
int XTextWidth (fs, string, count)
    XFontStruct *fs;
    char *string;
    int count;
{
    int i;				/* iterator */
    Bool singlerow = (fs->max_byte1 == 0);  /* optimization */
    XCharStruct *def;			/* info about default char */
    unsigned char *us = (unsigned char *) string;  /* be 8bit clean */
    int width = 0;			/* RETURN value */

    if (singlerow) {			/* optimization */
	GetDefaultCharInfo1d (fs, def);
    } else {
	GetDefaultCharInfo2d (fs, def);
    }

    /*
     * Iterate over all character in the input string; only consider characters
     * that exist.
     */
    for (i = 0, us = (unsigned char *) string; i < count; i++, us++) {
	register unsigned uc = (unsigned) *us;	/* since about to do macro */
	register XCharStruct *cs;

	if (singlerow) {		/* optimization */
	    GetCharInfo1d (fs, uc, def, cs);
	} else {
	    GetRowZeroCharInfo2d (fs, uc, def, cs);
	}

	if (cs) width += cs->width;
    }

    return width;
}
