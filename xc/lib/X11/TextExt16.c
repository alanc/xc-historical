/*
 * $XConsortium: XTextExt16.c,v 1.1 89/06/07 12:24:30 jim Exp $
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
 * XTextExtents16 - compute the extents of string given as a sequence of 
 * XChar2bs.
 */
XTextExtents16 (fs, string, nchars, dir, font_ascent, font_descent, overall)
    XFontStruct *fs;
    XChar2b *string;
    int nchars;
    int *dir, *font_ascent, *font_descent;  /* RETURN font information */
    register XCharStruct *overall;	/* RETURN character information */
{
    int i;				/* iterator */
    Bool singlerow = (fs->max_byte1 == 0);  /* optimization */
    int nfound = 0;			/* number of characters found */
    XCharStruct *def;			/* info about default char */

    GetDefaultCharInfo2d (fs, def);

    *dir = fs->direction;
    *font_ascent = fs->ascent;
    *font_descent = fs->descent;

    /*
     * Iterate over the input string getting the appropriate * char struct.
     * The default (which may be null if there is no def_char) will be returned
     * if the character doesn't exist.  On the first time * through the loop,
     * assign the values to overall; otherwise, compute * the new values.
     */

    for (i = 0; i < nchars; i++, string++) {
	register XCharStruct *cs;
	unsigned int r = (unsigned int) string->byte1;	/* watch for macros */
	unsigned int c = (unsigned int) string->byte2;	/* watch for macros */

	if (singlerow && r == 0) {
	    GetCharInfo1d (fs, c, def, cs);
	} else {
	    GetCharInfo2d (fs, r, c, def, cs);
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
 * XTextWidth16 - compute the width of sequence of XChar2bs.  This is a 
 * subset of XTextExtents16.
 */
int XTextWidth16 (fs, string, count)
    XFontStruct *fs;
    XChar2b *string;
    int count;
{
    int i;				/* iterator */
    Bool singlerow = (fs->max_byte1 == 0);  /* optimization */
    XCharStruct *def;			/* info about default char */
    int width = 0;			/* RETURN value */

    GetDefaultCharInfo2d (fs, def);

    /*
     * Iterate over all character in the input string; only consider characters
     * that exist.
     */
    for (i = 0; i < count; i++, string++) {
	register XCharStruct *cs;
	unsigned int r = (unsigned int) string->byte1;	/* watch for macros */
	unsigned int c = (unsigned int) string->byte2;	/* watch for macros */

	if (singlerow && r == 0) {
	    GetCharInfo1d (fs, c, def, cs);
	} else {
	    GetCharInfo2d (fs, r, c, def, cs);
	}

	if (cs) width += cs->width;
    }

    return width;
}
