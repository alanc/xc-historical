/* $XConsortium: bdfread.c,v 1.21 94/03/17 16:38:59 gildea Exp $ */

/************************************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
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

/*

Copyright (c) 1994  X Consortium

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

#include <ctype.h>
#include "fntfilst.h"
/* use bitmap structure */
#include "bitmap.h"
#include "bdfint.h"

#define INDICES 256
#define MAXENCODING 0xFFFF
#define BDFLINELEN  1024

extern int  bitmapGetGlyphs(), bitmapGetMetrics();
extern int  bitmapGetBitmaps(), bitmapGetExtents();
void        bdfUnloadFont();
static Bool bdfPadToTerminal();
extern int  bdfFileLineNum;

/***====================================================================***/

static Bool
bdfReadBitmap(pCI, file, bit, byte, glyph, scan, sizes)
    CharInfoPtr pCI;
    FontFilePtr file;
    int         bit,
                byte,
                glyph,
                scan;
    int         sizes[GLYPHPADOPTIONS];
{
    int         widthBits,
                widthBytes,
                widthHexChars;
    int         height,
                row;
    int         i,
                inLineLen,
                nextByte;
    Bool        badbits;
    unsigned char *pInBits,
               *picture,
               *line = NULL;
    char        lineBuf[BDFLINELEN];

    widthBits = GLYPHWIDTHPIXELS(pCI);
    height = GLYPHHEIGHTPIXELS(pCI);

    widthBytes = BYTES_PER_ROW(widthBits, glyph);
    if (widthBytes * height > 0) {
	picture = (unsigned char *) xalloc(widthBytes * height);
	if (!picture)
	    goto BAILOUT;
    } else
	picture = NULL;
    pCI->bits = (char *) picture;

    if (sizes) {
	for (i = 0; i < GLYPHPADOPTIONS; i++)
	    sizes[i] += BYTES_PER_ROW(widthBits, (1 << i)) * height;
    }
    badbits = FALSE;
    nextByte = 0;
    widthHexChars = BYTES_PER_ROW(widthBits, 1);

/* 5/31/89 (ef) -- hack, hack, hack.  what *am* I supposed to do with */
/*		0 width characters? */

    for (row = 0; row < height; row++) {
	line = bdfGetLine(file, lineBuf, BDFLINELEN);
	if (!line)
	    break;

	if (widthBits == 0) {
	    if ((!line) || (bdfIsPrefix(line, "ENDCHAR")))
		break;
	    else
		continue;
	}
	pInBits = line;
	inLineLen = strlen((char *) pInBits);

	if (inLineLen & 1) {
	    bdfError("odd number of characters in hex encoding\n");
	    line[inLineLen++] = '0';
	    line[inLineLen] = '\0';
	}
	inLineLen >>= 1;
	i = inLineLen;
	if (i > widthHexChars)
	    i = widthHexChars;
	for (; i > 0; i--, pInBits += 2)
	    picture[nextByte++] = bdfHexByte(pInBits);

	/* pad if line is too short */
	if (inLineLen < widthHexChars) {
	    for (i = widthHexChars - inLineLen; i > 0; i--)
		picture[nextByte++] = 0;
	} else {
	    unsigned char mask;

	    mask = 0xff << (8 - (widthBits & 0x7));
	    if (mask && picture[nextByte - 1] & ~mask) {
		picture[nextByte - 1] &= mask;
		badbits = TRUE;
	    } else if (inLineLen > widthHexChars)
		badbits = TRUE;
	}

	if (widthBytes > widthHexChars) {
	    i = widthBytes - widthHexChars;
	    while (i-- > 0)
		picture[nextByte++] = 0;
	}
    }

    if ((line && (!bdfIsPrefix(line, "ENDCHAR"))) || (height == 0))
	line = bdfGetLine(file, lineBuf, BDFLINELEN);

    if ((!line) || (!bdfIsPrefix(line, "ENDCHAR"))) {
	bdfError("missing 'ENDCHAR'\n");
	goto BAILOUT;
    }
    if (nextByte != height * widthBytes) {
	bdfError("bytes != rows * bytes_per_row (%d != %d * %d)\n",
		 nextByte, height, widthBytes);
	goto BAILOUT;
    }
    if (picture != NULL) {
	if (bit == LSBFirst)
	    BitOrderInvert(picture, nextByte);
	if (bit != byte) {
	    if (scan == 2)
		TwoByteSwap(picture, nextByte);
	    else if (scan == 4)
		FourByteSwap(picture, nextByte);
	}
    }
    return (TRUE);
BAILOUT:
    if (picture)
	xfree(picture);
    pCI->bits = NULL;
    return (FALSE);
}

/***====================================================================***/

static Bool
bdfSkipBitmap(file, height)
    FontFilePtr file;
    int         height;
{
    unsigned char *line;
    int         i = 0;
    char        lineBuf[BDFLINELEN];

    do {
	line = bdfGetLine(file, lineBuf, BDFLINELEN);
	i++;
    } while (line && !bdfIsPrefix(line, "ENDCHAR") && i <= height);

    if (i > 1 && line && !bdfIsPrefix(line, "ENDCHAR")) {
	bdfError("Error in bitmap, missing 'ENDCHAR'\n");
	return (FALSE);
    }
    return (TRUE);
}

/***====================================================================***/


static Bool
bdfReadCharacters(file, pFont, pState, bit, byte, glyph, scan)
    FontFilePtr file;
    FontPtr     pFont;
    bdfFileState *pState;
    int         bit,
                byte,
                glyph,
                scan;
{
    unsigned char *line;
    register CharInfoPtr ci;
    int         i,
                ndx,
                nchars,
                nignored;
    unsigned int char_row, char_col;
    int         numEncodedGlyphs = 0;
    CharInfoPtr *bdfEncoding[256];
    BitmapFontPtr  bitmapFont;
    BitmapExtraPtr bitmapExtra;
    CARD32     *bitmapsSizes;
    char        lineBuf[BDFLINELEN];
    int         nencoding;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    bitmapExtra = (BitmapExtraPtr) bitmapFont->bitmapExtra;

    if (bitmapExtra) {
	bitmapsSizes = bitmapExtra->bitmapsSizes;
	for (i = 0; i < GLYPHPADOPTIONS; i++)
	    bitmapsSizes[i] = 0;
    } else
	bitmapsSizes = NULL;

    bzero(bdfEncoding, sizeof(bdfEncoding));
    bitmapFont->metrics = NULL;
    ndx = 0;

    line = bdfGetLine(file, lineBuf, BDFLINELEN);

    if ((!line) || (sscanf((char *) line, "CHARS %d", &nchars) != 1)) {
	bdfError("bad 'CHARS' in bdf file\n");
	return (FALSE);
    }
    if (nchars < 1) {
	bdfError("invalid number of CHARS in BDF file\n");
	return (FALSE);
    }
    ci = (CharInfoPtr) xalloc(nchars * sizeof(CharInfoRec));
    if (!ci) {
	bdfError("Couldn't allocate pCI (%d*%d)\n", nchars,
		 sizeof(CharInfoRec));
	goto BAILOUT;
    }
    bitmapFont->metrics = ci;

    if (bitmapExtra) {
	bitmapExtra->glyphNames = (Atom *) xalloc(nchars * sizeof(Atom));
	if (!bitmapExtra->glyphNames) {
	    bdfError("Couldn't allocate glyphNames (%d*%d)\n",
		     nchars, sizeof(Atom));
	    goto BAILOUT;
	}
    }
    if (bitmapExtra) {
	bitmapExtra->sWidths = (int *) xalloc(nchars * sizeof(int));
	if (!bitmapExtra->sWidths) {
	    bdfError("Couldn't allocate sWidth (%d *%d)\n",
		     nchars, sizeof(int));
	    return FALSE;
	}
    }
    line = bdfGetLine(file, lineBuf, BDFLINELEN);
    pFont->info.firstRow = 256;
    pFont->info.lastRow = 0;
    pFont->info.firstCol = 256;
    pFont->info.lastCol = 0;
    nignored = 0;
    for (ndx = 0; (ndx < nchars) && (line) && (bdfIsPrefix(line, "STARTCHAR"));) {
	int         t;
	int         wx;		/* x component of width */
	int         wy;		/* y component of width */
	int         bw;		/* bounding-box width */
	int         bh;		/* bounding-box height */
	int         bl;		/* bounding-box left */
	int         bb;		/* bounding-box bottom */
	int         enc,
	            enc2;	/* encoding */
	unsigned char *p;	/* temp pointer into line */
	char        charName[100];
	int         ignore;

	if (sscanf((char *) line, "STARTCHAR %s", charName) != 1) {
	    bdfError("bad character name in BDF file\n");
	    goto BAILOUT;	/* bottom of function, free and return error */
	}
	if (bitmapExtra)
	    bitmapExtra->glyphNames[ndx] = bdfForceMakeAtom(charName, NULL);

	line = bdfGetLine(file, lineBuf, BDFLINELEN);
	if (!line || (t = sscanf((char *) line, "ENCODING %d %d", &enc, &enc2)) < 1) {
	    bdfError("bad 'ENCODING' in BDF file\n");
	    goto BAILOUT;
	}
	if (enc < -1 || t == 2 && enc2 < -1) {
	    bdfError("bad ENCODING value");
	    goto BAILOUT;
	}
	if (t == 2 && enc == -1)
	    enc = enc2;
	ignore = 0;
	if (enc == -1) {
	    if (!bitmapExtra) {
		nignored++;
		ignore = 1;
	    }
	} else if (enc > MAXENCODING) {
	    bdfError("char '%s' has encoding too large (%d)\n",
		     charName, enc);
	} else {
	    char_row = (enc >> 8) & 0xFF;
	    char_col = enc & 0xFF;
	    if (char_row < pFont->info.firstRow)
		pFont->info.firstRow = char_row;
	    if (char_row > pFont->info.lastRow)
		pFont->info.lastRow = char_row;
	    if (char_col < pFont->info.firstCol)
		pFont->info.firstCol = char_col;
	    if (char_col > pFont->info.lastCol)
		pFont->info.lastCol = char_col;
	    if (bdfEncoding[char_row] == (CharInfoPtr *) NULL) {
		bdfEncoding[char_row] =
		    (CharInfoPtr *) xalloc(256 * sizeof(CharInfoPtr));
		if (!bdfEncoding[char_row]) {
		    bdfError("Couldn't allocate row %d of encoding (%d*%d)\n",
			     char_row, INDICES, sizeof(CharInfoPtr));
		    goto BAILOUT;
		}
		for (i = 0; i < 256; i++)
		    bdfEncoding[char_row][i] = (CharInfoPtr) NULL;
	    }
	    if (bdfEncoding[char_row] != NULL) {
		bdfEncoding[char_row][char_col] = ci;
		numEncodedGlyphs++;
	    }
	}

	line = bdfGetLine(file, lineBuf, BDFLINELEN);
	if ((!line) || (sscanf((char *) line, "SWIDTH %d %d", &wx, &wy) != 2)) {
	    bdfError("bad 'SWIDTH'\n");
	    goto BAILOUT;
	}
	if (wy != 0) {
	    bdfError("SWIDTH y value must be zero\n");
	    goto BAILOUT;
	}
	if (bitmapExtra)
	    bitmapExtra->sWidths[ndx] = wx;

/* 5/31/89 (ef) -- we should be able to ditch the character and recover */
/*		from all of these.					*/

	line = bdfGetLine(file, lineBuf, BDFLINELEN);
	if ((!line) || (sscanf((char *) line, "DWIDTH %d %d", &wx, &wy) != 2)) {
	    bdfError("bad 'DWIDTH'\n");
	    goto BAILOUT;
	}
	if (wy != 0) {
	    bdfError("DWIDTH y value must be zero\n");
	    goto BAILOUT;
	}
	line = bdfGetLine(file, lineBuf, BDFLINELEN);
	if ((!line) || (sscanf((char *) line, "BBX %d %d %d %d", &bw, &bh, &bl, &bb) != 4)) {
	    bdfError("bad 'BBX'\n");
	    goto BAILOUT;
	}
	if ((bh < 0) || (bw < 0)) {
	    bdfError("character '%s' has a negative sized bitmap, %dx%d\n",
		     charName, bw, bh);
	    goto BAILOUT;
	}
	line = bdfGetLine(file, lineBuf, BDFLINELEN);
	if ((line) && (bdfIsPrefix(line, "ATTRIBUTES"))) {
	    for (p = line + strlen("ATTRIBUTES ");
		    (*p == ' ') || (*p == '\t');
		    p++)
		 /* empty for loop */ ;
	    ci->metrics.attributes = bdfHexByte(p) << 8 + bdfHexByte(p + 2);
	    line = bdfGetLine(file, lineBuf, BDFLINELEN);
	} else
	    ci->metrics.attributes = 0;

	if (!line || !bdfIsPrefix(line, "BITMAP")) {
	    bdfError("missing 'BITMAP'\n");
	    goto BAILOUT;
	}
	/* collect data for generated properties */
	if ((strlen(charName) == 1)) {
	    if ((charName[0] >= '0') && (charName[0] <= '9')) {
		pState->digitWidths += wx;
		pState->digitCount++;
	    } else if (charName[0] == 'x') {
		pState->exHeight = (bh + bb) <= 0 ? bh : bh + bb;
	    }
	}
	if (!ignore) {
	    ci->metrics.leftSideBearing = bl;
	    ci->metrics.rightSideBearing = bl + bw;
	    ci->metrics.ascent = bh + bb;
	    ci->metrics.descent = -bb;
	    ci->metrics.characterWidth = wx;
	    ci->bits = NULL;
	    bdfReadBitmap(ci, file, bit, byte, glyph, scan, bitmapsSizes);
	    ci++;
	    ndx++;
	} else
	    bdfSkipBitmap(file, bh);

	line = bdfGetLine(file, lineBuf, BDFLINELEN);	/* get STARTCHAR or
							 * ENDFONT */
    }

    if (ndx + nignored != nchars) {
	bdfError("%d too few characters\n", nchars - (ndx + nignored));
	goto BAILOUT;
    }
    nchars = ndx;
    bitmapFont->num_chars = nchars;
    if ((line) && (bdfIsPrefix(line, "STARTCHAR"))) {
	bdfError("more characters than specified\n");
	goto BAILOUT;
    }
    if ((!line) || (!bdfIsPrefix(line, "ENDFONT"))) {
	bdfError("missing 'ENDFONT'\n");
	goto BAILOUT;
    }
    if (numEncodedGlyphs == 0)
	bdfWarning("No characters with valid encodings\n");

    nencoding = (pFont->info.lastRow - pFont->info.firstRow + 1) *
	(pFont->info.lastCol - pFont->info.firstCol + 1);
    bitmapFont->encoding = (CharInfoPtr *) xalloc(nencoding * sizeof(CharInfoPtr));
    if (!bitmapFont->encoding) {
	bdfError("Couldn't allocate ppCI (%d,%d)\n",
		 nencoding,
		 sizeof(CharInfoPtr));
	goto BAILOUT;
    }
    pFont->info.allExist = TRUE;
    i = 0;
    for (char_row = pFont->info.firstRow;
	    char_row <= pFont->info.lastRow;
	    char_row++) {
	if (bdfEncoding[char_row] == (CharInfoPtr *) NULL) {
	    pFont->info.allExist = FALSE;
	    for (char_col = pFont->info.firstCol;
		    char_col <= pFont->info.lastCol;
		    char_col++) {
		bitmapFont->encoding[i++] = NullCharInfo;
	    }
	} else {
	    for (char_col = pFont->info.firstCol;
		    char_col <= pFont->info.lastCol;
		    char_col++) {
		if (!bdfEncoding[char_row][char_col])
		    pFont->info.allExist = FALSE;
		bitmapFont->encoding[i++] = bdfEncoding[char_row][char_col];
	    }
	}
    }
    for (i = 0; i < 256; i++)
	if (bdfEncoding[i])
	    xfree(bdfEncoding[i]);
    return (TRUE);
BAILOUT:
    for (i = 0; i < 256; i++)
	if (bdfEncoding[i])
	    xfree(bdfEncoding[i]);
    /* bdfFreeFontBits will clean up the rest */
    return (FALSE);
}

/***====================================================================***/

static Bool
bdfReadHeader(file, pState)
    FontFilePtr file;
    bdfFileState *pState;
{
    unsigned char *line;
    char        namebuf[BDFLINELEN];
    char        lineBuf[BDFLINELEN];

    line = bdfGetLine(file, lineBuf, BDFLINELEN);
    if (!line || sscanf((char *) line, "STARTFONT %s", namebuf) != 1 ||
	    !bdfStrEqual(namebuf, "2.1")) {
	bdfError("bad 'STARTFONT'\n");
	return (FALSE);
    }
    line = bdfGetLine(file, lineBuf, BDFLINELEN);
    if (!line || sscanf((char *) line, "FONT %[^\n]", pState->fontName) != 1) {
	bdfError("bad 'FONT'\n");
	return (FALSE);
    }
    line = bdfGetLine(file, lineBuf, BDFLINELEN);
    if (!line || !bdfIsPrefix(line, "SIZE")) {
	bdfError("missing 'SIZE'\n");
	return (FALSE);
    }
    if (sscanf((char *) line, "SIZE %f%d%d", &pState->pointSize,
	       &pState->resolution_x, &pState->resolution_y) != 3) {
	bdfError("bad 'SIZE'\n");
	return (FALSE);
    }
    if (pState->pointSize < 1 ||
	pState->resolution_x < 1 || pState->resolution_y < 1) {
	bdfError("SIZE values must be > 0\n");
	return (FALSE);
    }
    line = bdfGetLine(file, lineBuf, BDFLINELEN);
    if (!line || !bdfIsPrefix(line, "FONTBOUNDINGBOX")) {
	bdfError("missing 'FONTBOUNDINGBOX'\n");
	return (FALSE);
    }
    return (TRUE);
}

/***====================================================================***/

static Bool
bdfReadProperties(file, pFont, pState)
    FontFilePtr file;
    FontPtr     pFont;
    bdfFileState *pState;
{
    int         nProps, props_left,
                nextProp;
    char       *stringProps;
    FontPropPtr props;
    char        namebuf[BDFLINELEN],
                secondbuf[BDFLINELEN],
                thirdbuf[BDFLINELEN];
    unsigned char *line;
    char        lineBuf[BDFLINELEN];
    BitmapFontPtr  bitmapFont = (BitmapFontPtr) pFont->fontPrivate;

    line = bdfGetLine(file, lineBuf, BDFLINELEN);
    if (!line || !bdfIsPrefix(line, "STARTPROPERTIES")) {
	bdfError(file, "missing 'STARTPROPERTIES'\n");
	return (FALSE);
    }
    if (sscanf((char *) line, "STARTPROPERTIES %d", &nProps) != 1) {
	bdfError("bad 'STARTPROPERTIES'\n");
	return (FALSE);
    }
    pFont->info.isStringProp = NULL;
    pFont->info.props = NULL;

    stringProps = (char *) xalloc((nProps + BDF_GENPROPS) * sizeof(char));
    pFont->info.isStringProp = stringProps;
    if (stringProps == NULL) {
	bdfError("Couldn't allocate stringProps (%d*%d)\n",
		 (nProps + BDF_GENPROPS), sizeof(Bool));
	goto BAILOUT;
    }
    pFont->info.props = props = (FontPropPtr) xalloc((nProps + BDF_GENPROPS) *
						     sizeof(FontPropRec));
    if (props == NULL) {
	bdfError("Couldn't allocate props (%d*%d)\n", nProps + BDF_GENPROPS,
						      sizeof(FontPropRec));
	goto BAILOUT;
    }
    nextProp = 0;
    props_left = nProps;
    while (props_left-- > 0) {
	line = bdfGetLine(file, lineBuf, BDFLINELEN);
	if (line == NULL || bdfIsPrefix(line, "ENDPROPERTIES")) {
	    bdfError("\"STARTPROPERTIES %d\" followed by only %d properties\n",
		     nProps, nProps - props_left - 1);
	    goto BAILOUT;
	}
	while (*line && isspace(*line))
	    line++;

	switch (sscanf((char *) line, "%s%s%s", namebuf, secondbuf, thirdbuf)) {
	default:
	    bdfError("missing '%s' parameter value\n", namebuf);
	    goto BAILOUT;

	case 2:
	    /*
	     * Possibilites include: valid quoted string with no white space
	     * valid integer value invalid value
	     */
	    if (secondbuf[0] == '"') {
		stringProps[nextProp] = TRUE;
		props[nextProp].value =
		    bdfGetPropertyValue(line + strlen(namebuf) + 1);
		if (!props[nextProp].value)
		    goto BAILOUT;
		break;
	    } else if (bdfIsInteger(secondbuf)) {
		stringProps[nextProp] = FALSE;
		props[nextProp].value = atoi(secondbuf);
		break;
	    } else {
		bdfError("invalid '%s' parameter value\n", namebuf);
		goto BAILOUT;
	    }

	case 3:
	    /*
	     * Possibilites include: valid quoted string with some white space
	     * invalid value (reject even if second string is integer)
	     */
	    if (secondbuf[0] == '"') {
		stringProps[nextProp] = TRUE;
		props[nextProp].value =
		    bdfGetPropertyValue(line + strlen(namebuf) + 1);
		if (!props[nextProp].value)
		    goto BAILOUT;
		break;
	    } else {
		bdfError("invalid '%s' parameter value\n", namebuf);
		goto BAILOUT;
	    }
	}
	props[nextProp].name = bdfForceMakeAtom(namebuf, NULL);
	if (props[nextProp].name == None) {
	    bdfError("Empty property name.\n");
	    goto BAILOUT;
	}
	if (!bdfSpecialProperty(pFont, &props[nextProp],
				stringProps[nextProp], pState))
	    nextProp++;
    }

    line = bdfGetLine(file, lineBuf, BDFLINELEN);
    if (!line || !bdfIsPrefix(line, "ENDPROPERTIES")) {
	bdfError("missing 'ENDPROPERTIES'\n");
	goto BAILOUT;
    }
    if (!pState->haveFontAscent || !pState->haveFontDescent) {
	bdfError("missing 'FONT_ASCENT' or 'FONT_DESCENT' properties\n");
	goto BAILOUT;
    }
    if (bitmapFont->bitmapExtra) {
	bitmapFont->bitmapExtra->info.fontAscent = pFont->info.fontAscent;
	bitmapFont->bitmapExtra->info.fontDescent = pFont->info.fontDescent;
    }
    if (!pState->pointSizeProp) {
	props[nextProp].name = bdfForceMakeAtom("POINT_SIZE", NULL);
	props[nextProp].value = (INT32) (pState->pointSize * 10.0);
	stringProps[nextProp] = FALSE;
	pState->pointSizeProp = &props[nextProp];
	nextProp++;
    }
    if (!pState->fontProp) {
	props[nextProp].name = bdfForceMakeAtom("FONT", NULL);
	props[nextProp].value = (INT32) bdfForceMakeAtom(pState->fontName, NULL);
	stringProps[nextProp] = TRUE;
	pState->fontProp = &props[nextProp];
	nextProp++;
    }
    if (!pState->weightProp) {
	props[nextProp].name = bdfForceMakeAtom("WEIGHT", NULL);
	props[nextProp].value = -1;	/* computed later */
	stringProps[nextProp] = FALSE;
	pState->weightProp = &props[nextProp];
	nextProp++;
    }
    if (!pState->resolutionProp &&
	pState->resolution_x == pState->resolution_y) {
	props[nextProp].name = bdfForceMakeAtom("RESOLUTION", NULL);
	props[nextProp].value = (INT32) ((pState->resolution_x * 100.0) / 72.27);
	stringProps[nextProp] = FALSE;
	pState->resolutionProp = &props[nextProp];
	nextProp++;
    }
    if (!pState->resolutionXProp) {
	props[nextProp].name = bdfForceMakeAtom("RESOLUTION_X", NULL);
	props[nextProp].value = (INT32) pState->resolution_x;
	stringProps[nextProp] = FALSE;
	pState->resolutionProp = &props[nextProp];
	nextProp++;
    }
    if (!pState->resolutionYProp) {
	props[nextProp].name = bdfForceMakeAtom("RESOLUTION_Y", NULL);
	props[nextProp].value = (INT32) pState->resolution_y;
	stringProps[nextProp] = FALSE;
	pState->resolutionProp = &props[nextProp];
	nextProp++;
    }
    if (!pState->xHeightProp) {
	props[nextProp].name = bdfForceMakeAtom("X_HEIGHT", NULL);
	props[nextProp].value = -1;	/* computed later */
	stringProps[nextProp] = FALSE;
	pState->xHeightProp = &props[nextProp];
	nextProp++;
    }
    if (!pState->quadWidthProp) {
	props[nextProp].name = bdfForceMakeAtom("QUAD_WIDTH", NULL);
	props[nextProp].value = -1;	/* computed later */
	stringProps[nextProp] = FALSE;
	pState->quadWidthProp = &props[nextProp];
	nextProp++;
    }
    pFont->info.nprops = nextProp;
    return (TRUE);
BAILOUT:
    if (pFont->info.isStringProp) {
	xfree(pFont->info.isStringProp);
	pFont->info.isStringProp = NULL;
    }
    if (pFont->info.props) {
	xfree(pFont->info.props);
	pFont->info.props = NULL;
    }
    while (line && bdfIsPrefix(line, "ENDPROPERTIES"))
	line = bdfGetLine(file, lineBuf, BDFLINELEN);
    return (FALSE);
}

/***====================================================================***/

int
bdfReadFont(pFont, file, bit, byte, glyph, scan)
    FontPtr     pFont;
    FontFilePtr file;
    int         bit,
                byte,
                glyph,
                scan;
{
    bdfFileState state;
    xCharInfo  *min,
               *max;
    BitmapFontPtr  bitmapFont;

    pFont->fontPrivate = 0;

    bzero(&state, sizeof(bdfFileState));
    bdfFileLineNum = 0;

    if (!bdfReadHeader(file, &state))
	goto BAILOUT;

    bitmapFont = (BitmapFontPtr) xalloc(sizeof(BitmapFontRec));
    if (!bitmapFont)
	goto BAILOUT;

    pFont->fontPrivate = (pointer) bitmapFont;
    bitmapFont->metrics = 0;
    bitmapFont->ink_metrics = 0;
    bitmapFont->bitmaps = 0;
    bitmapFont->encoding = 0;
    bitmapFont->pDefault = NULL;
    bitmapFont->bitmapExtra = (BitmapExtraPtr) xalloc(sizeof(BitmapExtraRec));
    bitmapFont->bitmapExtra->glyphNames = 0;
    bitmapFont->bitmapExtra->sWidths = 0;

    if (!bdfReadProperties(file, pFont, &state))
	goto BAILOUT;

    if (!bdfReadCharacters(file, pFont, &state, bit, byte, glyph, scan))
	goto BAILOUT;

    if (state.haveDefaultCh) {
	unsigned int r, c, cols;

	r = pFont->info.defaultCh >> 8;
	c = pFont->info.defaultCh & 0xFF;
	if (pFont->info.firstRow <= r && r <= pFont->info.lastRow &&
		pFont->info.firstCol <= c && c <= pFont->info.lastCol) {
	    cols = pFont->info.lastCol - pFont->info.firstCol + 1;
	    r = r - pFont->info.firstRow;
	    c = c - pFont->info.firstCol;
	    bitmapFont->pDefault = bitmapFont->encoding[r * cols + c];
	}
    }
    pFont->bit = bit;
    pFont->byte = byte;
    pFont->glyph = glyph;
    pFont->scan = scan;
    pFont->info.anamorphic = FALSE;
    pFont->info.cachable = TRUE;
    bitmapComputeFontBounds(pFont);
    if (FontCouldBeTerminal(&pFont->info)) {
	bdfPadToTerminal(pFont);
	bitmapComputeFontBounds(pFont);
    }
    FontComputeInfoAccelerators(&pFont->info);
    if (bitmapFont->bitmapExtra)
	FontComputeInfoAccelerators(&bitmapFont->bitmapExtra->info);
    if (pFont->info.constantMetrics)
	bitmapAddInkMetrics(pFont);
    if (bitmapFont->bitmapExtra)
	bitmapFont->bitmapExtra->info.inkMetrics = pFont->info.inkMetrics;

    bitmapComputeFontInkBounds(pFont);
/*    ComputeFontAccelerators (pFont); */

    /* generate properties */
    min = &pFont->info.ink_minbounds;
    max = &pFont->info.ink_maxbounds;
    if (state.xHeightProp && (state.xHeightProp->value == -1))
	state.xHeightProp->value = state.exHeight ?
	    state.exHeight : min->ascent;

    if (state.quadWidthProp && (state.quadWidthProp->value == -1))
	state.quadWidthProp->value = state.digitCount ?
	    (INT32) (state.digitWidths / state.digitCount) :
	    (min->characterWidth + max->characterWidth) / 2;

    if (state.weightProp && (state.weightProp->value == -1))
	state.weightProp->value = bitmapComputeWeight(pFont);

    pFont->get_glyphs = bitmapGetGlyphs;
    pFont->get_metrics = bitmapGetMetrics;
    pFont->unload_font = bdfUnloadFont;
    pFont->unload_glyphs = NULL;
    return Successful;
BAILOUT:
    if (pFont->fontPrivate)
	bdfFreeFontBits (pFont);
    return AllocError;
}

bdfFreeFontBits(pFont)
    FontPtr pFont;
{
    BitmapFontPtr  bitmapFont;
    BitmapExtraPtr bitmapExtra;
    int         i;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    bitmapExtra = (BitmapExtraPtr) bitmapFont->bitmapExtra;
    xfree(bitmapFont->ink_metrics);
    xfree(bitmapFont->encoding);
    for (i = 0; i < bitmapFont->num_chars; i++)
	xfree(bitmapFont->metrics[i].bits);
    xfree(bitmapFont->metrics);
    if (bitmapExtra)
    {
	xfree (bitmapExtra->glyphNames);
	xfree (bitmapExtra->sWidths);
	xfree (bitmapExtra);
    }
    xfree(pFont->info.props);
    xfree(bitmapFont);
}

int
bdfReadFontInfo(pFontInfo, file)
    FontInfoPtr pFontInfo;
    FontFilePtr file;
{
    FontRec     font;
    int         ret;

    ret = bdfReadFont(&font, file, MSBFirst, LSBFirst, 1, 1);
    if (ret == Successful) {
	*pFontInfo = font.info;
	font.info.props = 0;
	font.info.isStringProp = 0;
	font.info.nprops = 0;
	bdfFreeFontBits (&font);
    }
    return ret;
}

void
bdfUnloadFont(pFont)
    FontPtr     pFont;
{
    bdfFreeFontBits (pFont);
    xfree (pFont->devPrivates);
    xfree(pFont);
}

static Bool
bdfPadToTerminal(pFont)
    FontPtr     pFont;
{
    BitmapFontPtr  bitmapFont;
    BitmapExtraPtr bitmapExtra;
    int         i;
    int         new_size;
    CharInfoRec new;
    int         w,
                h;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    new.metrics.ascent = pFont->info.fontAscent;
    new.metrics.descent = pFont->info.fontDescent;
    new.metrics.leftSideBearing = 0;
    new.metrics.rightSideBearing = pFont->info.minbounds.characterWidth;
    new.metrics.characterWidth = new.metrics.rightSideBearing;
    new_size = BYTES_FOR_GLYPH(&new, pFont->glyph);
    for (i = 0; i < bitmapFont->num_chars; i++) {
	new.bits = (char *) xalloc(new_size);
	if (!new.bits)
	    return FALSE;
	FontCharReshape(pFont, &bitmapFont->metrics[i], &new);
        new.metrics.attributes = bitmapFont->metrics[i].metrics.attributes;
	xfree(bitmapFont->metrics[i].bits);
	bitmapFont->metrics[i] = new;
    }
    bitmapExtra = bitmapFont->bitmapExtra;
    if (bitmapExtra) {
	w = GLYPHWIDTHPIXELS(&new);
	h = GLYPHHEIGHTPIXELS(&new);
	for (i = 0; i < GLYPHPADOPTIONS; i++)
	    bitmapExtra->bitmapsSizes[i] = bitmapFont->num_chars *
		(BYTES_PER_ROW(w, 1 << i) * h);
    }
    return TRUE;
}
