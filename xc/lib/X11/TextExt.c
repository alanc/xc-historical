#define NEED_REPLIES

#include "Xlibint.h"

#define min_byte2 min_char_or_byte2
#define max_byte2 max_char_or_byte2


/*
 * GetCS - Return the charinfo struct for the indicated 8bit character.  If
 * the character is in the column and exists, then return the appropriate
 * metrics (note that fonts with common per-character metrics will return
 * min_bounds).  If none of these hold true, try again with the default char. 
 */

static XCharStruct *InitGetGS (fs)
    XFontStruct *fs;
{
    register XCharStruct *cs;
    unsigned int col = fs->default_char;

    if (col >= fs->min_byte2 && col <= fs->max_byte2) {
	if (fs->per_char == NULL) return &fs->min_bounds;
	cs = &fs->per_char[(col - fs->min_byte2)];
	if (! CI_NONEXISTCHAR(cs)) return cs;
    }

    return NULL;
}


static XCharStruct *GetCS (fs, col, def)
    register XFontStruct *fs;
    unsigned int col;
    XCharStruct *def;
{
    register XCharStruct *cs;

    if (col >= fs->min_byte2 && col <= fs->max_byte2) {
	if (fs->per_char == NULL) return &fs->min_bounds;
	cs = &fs->per_char[(col - fs->min_byte2)];
	if (! CI_NONEXISTCHAR(cs)) return cs;
    }

    return def;
}


/*
 * GetCS2d - do the same thing as GetCS, except that the font has more than
 * one row.
 */

static XCharStruct *InitGetCS (fs)
    XFontStruct *fs;
{
    unsigned int row = (fs->default_char >> 8);
    unsigned int col = (fs->default_char & 0xff);

    if (row >= fs->min_byte1 && row <= fs->max_byte1 &&
	col >= fs->min_byte2 && col <= fs->max_byte2) {
	if (fs->per_char == NULL) return &fs->min_bounds;
	cs = &fs->per_char[(row - fs->min_byte1) * numCols + 
			   (col - fs->min_byte2)];
	if (! CI_NONEXISTCHAR(cs)) return cs;
    }

    return NULL;
}


static XCharStruct *GetCS2d (fs, row, col, def)
    register XFontStruct *fs;
    unsigned int row, col;
    XCharStruct *def;
{
    register XCharStruct *cs;
    int numRows = fs->max_byte1 - fs->min_byte1 + 1;
    int numCols = fs->max_byte2 - fs->min_byte2 + 1;

    if (row >= fs->min_byte1 && row <= fs->max_byte1 &&
	col >= fs->min_byte2 && col <= fs->max_byte2) {
	if (fs->per_char == NULL) return &fs->min_bounds;
	cs = &fs->per_char[(row - fs->min_byte1) * numCols + 
			   (col - fs->min_byte2)];
	if (! CI_NONEXISTCHAR(cs)) return cs;
    }

    return def;
}


XTextExtents (fs, string, nchars, dir, font_ascent, font_descent, overall)
    XFontStruct *fs;
    char *string;
    int nchars;
    int *dir, *font_ascent, *font_descent;  /* RETURN font information */
    register XCharStruct *overall;	/* RETURN character information */
{
    int i;				/* iterator */
    int nfound = 0;			/* number of characters found */
    XCharStruct *getglyph() = ((fs->max_byte1 == 0) ? GetCS : GetCS2d);
    XCharStruct *def = (*((fs->max_byte1 == 0) ? GetCS : GetCS2d)) (fs);
    unsigned char *ustring = (unsigned char *) string;

    *dir = fontstruct->direction;
    *font_ascent = fontstruct->ascent;
    *font_descent = fontstruct->descent;

    /*
     * iterate over all character in the input string
     */
    for (i = 0; i < nchars; i++) {
	register XCharStruct *cs = (*getglyph) (&fs, (unsigned) *ustring, def);

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


int XTextWidth (fs, string, count)
    XFontStruct *fs;
    char *string;
    int count;
{
    int i;				/* iterator */
    int nfound = 0;			/* number of characters found */
    XCharStruct *getglyph() = ((fs->max_byte1 == 0) ? GetCS : GetCS2d);
    XCharStruct *def = (*((fs->max_byte1 == 0) ? GetCS : GetCS2d)) (fs);
    unsigned char *ustring = (unsigned char *) string;
    int width = 0;

    /*
     * iterate over all character in the input string
     */
    for (i = 0; i < nchars; i++) {
	register XCharStruct *cs = (*getglyph) (&fs, (unsigned) *ustring, def);

	if (cs) width += cs->width;
    }

    return width;
}

	    
