/************************************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/

/* $XConsortium: bdfutils.c,v 1.5 92/03/26 17:38:18 gildea Exp $ */

#include <ctype.h>

#include <stdio.h>
#include "fontfilest.h"
#include "fontstruct.h"
/* use bitmap structure */
#include "bitmap.h"
#include "bdfint.h"

/***====================================================================***/

void
bdfError(message, a0, a1, a2, a3, a4, a5)
    char       *message;
    pointer     a0,
                a1,
                a2,
                a3,
                a4,
                a5;
{
    fprintf(stderr, "BDF Error: ");
    fprintf(stderr, message, a0, a1, a2, a3, a4, a5);
}

/***====================================================================***/

void
bdfWarning(message, a0, a1, a2, a3, a4, a5)
    char       *message;
    pointer     a0,
                a1,
                a2,
                a3,
                a4,
                a5;
{
    fprintf(stderr, "BDF Warning: ");
    fprintf(stderr, message, a0, a1, a2, a3, a4, a5);
}

/***====================================================================***/

/*ARGSUSED*/
void
bdfInformation(pFile, message, a0, a1, a2, a3, a4, a5)
    char       *message;
    pointer     a0,
                a1,
                a2,
                a3,
                a4,
                a5;
{
    fprintf(stderr, "BDF Information: ");
    fprintf(stderr, message, a0, a1, a2, a3, a4, a5);
}

/***====================================================================***/

/*
 * read the next (non-comment) line and keep a count for error messages.
 * Returns buf, or NULL if EOF.
 */

unsigned char *
bdfGetLine(file, buf, len)
    FontFilePtr file;
    unsigned char *buf;
    int         len;
{
    int         c;
    unsigned char *b;

    for (;;) {
	b = buf;
	while ((c = FontFileGetc(file)) != FontFileEOF) {
	    if (c == '\r')
		continue;
	    if (c == '\n')
		break;
	    if (b - buf >= (len - 1))
		break;
	    *b++ = c;
	}
	*b = '\0';
	if (c == FontFileEOF)
	    return NULL;
	if (b != buf && !bdfIsPrefix(buf, "COMMENT"))
	    break;
    }
    return buf;
}

/***====================================================================***/

Atom
bdfForceMakeAtom(str, size)
    register char *str;
    register int *size;
{
    register int len = strlen(str);
    extern Atom	MakeAtom();

    if (size != NULL)
	*size += len + 1;
    return MakeAtom(str, len, TRUE);
}

/***====================================================================***/

/*
 * Handle quoted strings.
 */

Atom
bdfGetPropertyValue(s)
    char       *s;
{
    register char *p,
               *pp;
    char *orig_s = s;
    Atom        atom;

    /* strip leading white space */
    while (*s && (*s == ' ' || *s == '\t'))
	s++;
    if (*s == 0) {
	return bdfForceMakeAtom(s, NULL);
    }
    if (*s != '"') {
	pp = s;
	/* no white space in value */
	for (pp = s; *pp; pp++)
	    if (*pp == ' ' || *pp == '\t' || *pp == '\015' || *pp == '\n') {
		*pp = 0;
		break;
	    }
	return bdfForceMakeAtom(s, NULL);
    }
    /* quoted string: strip outer quotes and undouble inner quotes */
    s++;
    pp = p = (char *) xalloc((unsigned) strlen(s) + 1);
    while (*s) {
	if (*s == '"') {
	    if (*(s + 1) != '"') {
		*p++ = 0;
		atom = bdfForceMakeAtom(pp, NULL);
		xfree(pp);
		return atom;
	    } else {
		s++;
	    }
	}
	*p++ = *s++;
    }
    xfree (pp);
    bdfError("unterminated quoted string property: %s\n", orig_s);
    return None;
}

/***====================================================================***/

/*
 * return TRUE if string is a valid integer
 */
int
bdfIsInteger(str)
    char       *str;
{
    char        c;

    c = *str++;
    if (!(isdigit(c) || c == '-' || c == '+'))
	return (FALSE);

    while (c = *str++)
	if (!isdigit(c))
	    return (FALSE);

    return (TRUE);
}

/***====================================================================***/

/*
 * make a byte from the first two hex characters in glyph picture
 */

unsigned char
bdfHexByte(s)
    char       *s;
{
    unsigned char b = 0;
    register char c;
    int         i;

    for (i = 2; i; i--) {
	c = *s++;
	if ((c >= '0') && (c <= '9'))
	    b = (b << 4) + (c - '0');
	else if ((c >= 'A') && (c <= 'F'))
	    b = (b << 4) + 10 + (c - 'A');
	else if ((c >= 'a') && (c <= 'f'))
	    b = (b << 4) + 10 + (c - 'a');
	else
	    bdfError("bad hex char '%c'", c);
    }
    return b;
}

/***====================================================================***/

/*
 * check for known special property values
 */

static char *SpecialAtoms[] = {
    "FONT_ASCENT",
#define BDF_FONT_ASCENT	0
    "FONT_DESCENT",
#define BDF_FONT_DESCENT 1
    "DEFAULT_CHAR",
#define BDF_DEFAULT_CHAR 2
    "POINT_SIZE",
#define BDF_POINT_SIZE 3
    "RESOLUTION",
#define BDF_RESOLUTION 4
    "X_HEIGHT",
#define BDF_X_HEIGHT 5
    "WEIGHT",
#define BDF_WEIGHT 6
    "QUAD_WIDTH",
#define BDF_QUAD_WIDTH 7
    "FONT",
#define BDF_FONT 8
    "RESOLUTION_X",
#define BDF_RESOLUTION_X 9
    "RESOLUTION_Y",
#define BDF_RESOLUTION_Y 10
    0,
};

Bool
bdfSpecialProperty(pFont, prop, isString, bdfState)
    FontPtr     pFont;
    FontPropPtr prop;
    char        isString;
    bdfFileState *bdfState;
{
    char      **special;
    char       *name;

    name = NameForAtom(prop->name);
    for (special = SpecialAtoms; *special; special++)
	if (!strcmp(name, *special))
	    break;

    switch (special - SpecialAtoms) {
    case BDF_FONT_ASCENT:
	if (!isString) {
	    pFont->info.fontAscent = prop->value;
	    bdfState->haveFontAscent = TRUE;
	}
	return TRUE;
    case BDF_FONT_DESCENT:
	if (!isString) {
	    pFont->info.fontDescent = prop->value;
	    bdfState->haveFontDescent = TRUE;
	}
	return TRUE;
    case BDF_DEFAULT_CHAR:
	if (!isString) {
	    pFont->info.defaultCh = prop->value;
	    bdfState->haveDefaultCh = TRUE;
	}
	return TRUE;
    case BDF_POINT_SIZE:
	bdfState->pointSizeProp = prop;
	return FALSE;
    case BDF_RESOLUTION:
	bdfState->resolutionProp = prop;
	return FALSE;
    case BDF_X_HEIGHT:
	bdfState->xHeightProp = prop;
	return FALSE;
    case BDF_WEIGHT:
	bdfState->weightProp = prop;
	return FALSE;
    case BDF_QUAD_WIDTH:
	bdfState->quadWidthProp = prop;
	return FALSE;
    case BDF_FONT:
	bdfState->fontProp = prop;
	return FALSE;
    case BDF_RESOLUTION_X:
	bdfState->resolutionXProp = prop;
	return FALSE;
    case BDF_RESOLUTION_Y:
	bdfState->resolutionYProp = prop;
	return FALSE;
    default:
	return FALSE;
    }
}
