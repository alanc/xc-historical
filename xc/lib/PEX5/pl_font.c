/* $XConsortium: pl_font.c,v 1.14 92/05/07 23:28:30 mor Exp $ */

/************************************************************************
Copyright 1987,1991,1992 by Digital Equipment Corporation, Maynard,
Massachusetts, and the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

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

*************************************************************************/

#include "PEXlib.h"
#include "PEXlibint.h"


PEXFont
PEXLoadFont (display, fontname)

INPUT Display	*display;
INPUT char	*fontname;

{
    pexOpenFontReq	*req;
    pexFont		id;


    /*
     * Lock around critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    PEXGetReq (OpenFont, req);
    req->numBytes = strlen (fontname);
    req->font = id = XAllocID (display);
    req->length += (req->numBytes + 3) >> 2;

    Data (display, (char *) fontname, req->numBytes);


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (id);
}


void
PEXUnloadFont (display, font)

INPUT Display	*display;
INPUT PEXFont	font;

{
    pexResourceReq *req;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    PEXGetReq (CloseFont, req);
    req->id = font;


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);
}


PEXFontInfo *
PEXQueryFont (display, font)

INPUT Display		*display;
INPUT PEXFont		font;

{
    pexQueryFontReply 	rep;
    pexQueryFontReq	*req;
    char		*buf;
    int 		prop_size;
    PEXFontInfo		*fontInfoReturn;


    /*
     * Lock around critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer and get a reply.
     */

    PEXGetReq (QueryFont, req);
    req->font = font;

    if (_XReply (display, &rep, 0, xFalse) == 0)
    {
	UnlockDisplay (display);
	PEXSyncHandle (display);
	return (NULL);            /* return an error */
    }


    /*
     * Allocate a scratch buffer and copy the reply data to the buffer.
     */

    buf = (char *) _XAllocScratch (display, (unsigned long) (rep.length << 2));

    _XRead (display, (char *) buf, (long) (rep.length << 2));


    /*
     * Allocate a buffer for the replies to pass back to the client.
     */

    fontInfoReturn = (PEXFontInfo *)
	PEXAllocBuf ((unsigned) sizeof (PEXFontInfo));

    prop_size = ((pexFontInfo *) (buf))->numProps * sizeof (PEXFontProp);

    COPY_AREA ((char *) buf, (char *) fontInfoReturn, sizeof (pexFontInfo));
    fontInfoReturn->prop = (PEXFontProp *) PEXAllocBuf ((unsigned ) prop_size);
    buf += sizeof (pexFontInfo);
    COPY_AREA ((char *) buf, (char *) fontInfoReturn->prop, prop_size);


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (fontInfoReturn);
}


PEXStringData *
PEXListFonts (display, pattern, maxNames, countReturn)

INPUT Display		*display;
INPUT char		*pattern;
INPUT unsigned int	maxNames;
OUTPUT unsigned long	*countReturn;

{
    pexListFontsReply 	rep;
    pexListFontsReq   	*req;
    long		numChars;
    pexString 		*repStrings;
    PEXStringData	*nextName, *namesReturn;
    int			i;


    /*
     * Lock around critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer and get a reply.
     */

    PEXGetReq (ListFonts, req);
    req->maxNames = maxNames;
    numChars = req->numChars = strlen (pattern);
    req->length += (numChars + 3) >> 2;

    Data (display, (char *) pattern, numChars);

    if (_XReply (display, &rep, 0, xFalse) == 0)
    {
	UnlockDisplay (display);
	PEXSyncHandle (display);
	*countReturn = 0;
	return (NULL);            /* return an error */
    }

    *countReturn = rep.numStrings;


    /*
     * Allocate a scratch buffer and copy the reply data to the buffer.
     */

    repStrings = (pexString *)
	_XAllocScratch (display, (unsigned long) (rep.length << 2));

    _XRead (display, (char *) repStrings, (long) (rep.length << 2));


    /*
     * Allocate a buffer for the replies to pass back to the client.
     */

    namesReturn = (PEXStringData *)
	PEXAllocBuf (rep.numStrings * sizeof (PEXStringData));

    for (i = 0, nextName = namesReturn; i < rep.numStrings; i++, nextName++)
    {
	nextName->length = (int) repStrings->length;

	nextName->ch = PEXAllocBuf ((unsigned) nextName->length);
	COPY_AREA ((char *) &repStrings[1], (char *) nextName->ch,
	    (unsigned) nextName->length);

	repStrings = (pexString *) ((char *) repStrings +
	    PADDED_BYTES (sizeof (pexString) + nextName->length));
    }


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (namesReturn);
}


PEXStringData *
PEXListFontsWithInfo (display, pattern, maxNames, numStringsReturn, 
    numFontInfoReturn, fontInfoReturn)

INPUT Display		*display;
INPUT char		*pattern;
INPUT unsigned int	maxNames;
OUTPUT unsigned long	*numStringsReturn;
OUTPUT unsigned long	*numFontInfoReturn;
OUTPUT PEXFontInfo	**fontInfoReturn;

{
    pexListFontsWithInfoReq	*req;
    pexListFontsWithInfoReply	rep;
    long			numChars;
    int				font_info_size, prop_size, i;
    PEXStringData		*nextName;
    char			*buf;
    pexString			*repStrings;
    PEXStringData		*namesReturn;
    PEXFontInfo			*pFontInfo;


    /*
     * Lock around critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer and get a reply.
     */

    PEXGetReq (ListFontsWithInfo, req);
    req->maxNames = maxNames;
    numChars = req->numChars = strlen (pattern);
    req->length += (numChars + 3) >> 2;

    Data (display, (char *) pattern, numChars);

    if (_XReply (display, &rep, 0, xFalse) == 0)
    {
	UnlockDisplay (display);
        PEXSyncHandle (display);
	*numStringsReturn = *numFontInfoReturn = 0;
        return (NULL);                /* return an error */
    }

    *numStringsReturn = rep.numStrings;


    /*
     * Allocate a scratch buffer and copy the reply data to the buffer.
     */

    buf = (char *) _XAllocScratch (display, (unsigned long) (rep.length << 2));

    _XRead (display, (char *) buf, (long) (rep.length << 2));


    /*
     * Allocate a buffer for the font names to pass back to the client.
     */

    namesReturn = (PEXStringData *)
	PEXAllocBuf (rep.numStrings * sizeof (PEXStringData));

    repStrings = (pexString *) buf;
    for (i = 0, nextName = namesReturn; i < rep.numStrings; i++, nextName++)
    {
        nextName->length = (int) repStrings->length;

        nextName->ch = PEXAllocBuf ((unsigned) nextName->length);
        COPY_AREA ((char *) &repStrings[1], (char *) nextName->ch,
            (unsigned) nextName->length);

        repStrings = (pexString *) ((char *) repStrings +
            PADDED_BYTES (sizeof (pexString) + nextName->length));
    }


    /*
     * Allocate a buffer for the font info to pass back to the client.
     */

    buf = (char *) repStrings;
    *numFontInfoReturn = (int) *((CARD32 *) buf); 
    buf += sizeof (CARD32);

    font_info_size = *numFontInfoReturn * sizeof (PEXFontInfo);
    *fontInfoReturn = pFontInfo = (PEXFontInfo *) PEXAllocBuf (font_info_size);

    for (i = 0; i < *numFontInfoReturn; i++, pFontInfo++)
    {
    	COPY_AREA ((char *) buf, (char *) pFontInfo, sizeof (pexFontInfo));
	prop_size = ((pexFontInfo *) (buf))->numProps * sizeof (PEXFontProp);
	(pFontInfo)->prop = (PEXFontProp *) PEXAllocBuf (prop_size);
	buf += sizeof (pexFontInfo);
    	COPY_AREA ((char *) buf, (char *) pFontInfo->prop, prop_size);
	buf += prop_size;
    }


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (namesReturn);
}


PEXTextExtent *
PEXQueryTextExtents (display, id, fontGroup, path, expansion, spacing, height, 
    halign, valign, count, text)

INPUT Display			*display;
INPUT XID			id;
INPUT unsigned int		fontGroup;
INPUT int			path;
INPUT double			expansion;
INPUT double			spacing;
INPUT double			height;
INPUT int			halign;
INPUT int			valign;
INPUT unsigned long		count;
INPUT PEXStringData		*text;
{
    pexQueryTextExtentsReq	*req;
    pexQueryTextExtentsReply 	rep;
    char			*ch;
    pexMonoEncoding 		pMonoEncoding;
    int				convertFP, numEncodings, i;
    PEXTextExtent		*textExtent, *ptextExtents;


    /*
     * Lock around critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer and get a reply.
     */

    PEXGetFPReq (QueryTextExtents, req, convertFP);
    req->textPath = path;
    req->id = id;
    req->fontGroupIndex = (pexTableIndex) fontGroup;
    req->charExpansion = expansion;
    req->charSpacing = spacing;
    req->charHeight = height;
    req->textAlignment.vertical = valign;
    req->textAlignment.horizontal = halign;
    req->numStrings = count;

    req->length += count * (LENOF (CARD32) + LENOF (pexMonoEncoding));
    for (i = 0; i < count; i++)
	req->length += (((int) text[i].length + 3) >> 2);

    pMonoEncoding.characterSet = (INT16) 1;
    pMonoEncoding.characterSetWidth = (CARD8) PEXCSByte;
    pMonoEncoding.encodingState = 0;  

    numEncodings = 1;

    for (i = 0; i < count; i++)
    {
	Data (display, (char *) &numEncodings, sizeof (CARD32));
	pMonoEncoding.numChars = (CARD16) (text[i].length);
	Data (display, (char *) &pMonoEncoding, sizeof (pexMonoEncoding));
	Data (display, (char *) text[i].ch, text[i].length);
    }

    if (_XReply (display, &rep, 0, xFalse) == 0)
    {
        UnlockDisplay (display);
        PEXSyncHandle (display);
        return (NULL);            /* return an error */
    }


    /*
     * Allocate a scratch buffer and copy the reply data to the buffer.
     */

    ch = (char *) _XAllocScratch (display, (unsigned long) (rep.length << 2));

    _XRead (display, ch, (long) (rep.length << 2));


    /*
     * Allocate a buffer for the replies to pass back to the client.
     */

    textExtent = ptextExtents =
	(PEXTextExtent *) PEXAllocBuf (count * sizeof (PEXTextExtent));

    for (i = 0; i < count; i++, textExtent++)
    {
	textExtent->lower_left = *(PEXCoord2D *) ch;
	ch += sizeof (PEXCoord2D);
	textExtent->upper_right = *(PEXCoord2D *) ch;
	ch += sizeof (PEXCoord2D);
	textExtent->concat_point = *(PEXCoord2D *) ch;
    }


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (ptextExtents);
}


PEXTextExtent *
PEXQueryEncodedTextExtents (display, id, fontGroup, path, expansion,
    spacing, height, halign, valign, count, encoded_text)

INPUT Display			*display;
INPUT XID			id;
INPUT unsigned int		fontGroup;
INPUT int			path;
INPUT double			expansion;
INPUT double			spacing;
INPUT double			height;
INPUT int			halign;
INPUT int			valign;
INPUT unsigned long		count;
INPUT PEXListOfEncodedText    	*encoded_text;

{
    pexQueryTextExtentsReq	*req;
    pexQueryTextExtentsReply 	rep;
    PEXEncodedTextData      	*string;
    char			*ch;
    int				i, j;
    int				convertFP;
    PEXTextExtent		*textExtent, *ptextExtents;


    /*
     * Lock around critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    PEXGetFPReq (QueryTextExtents, req, convertFP);
    req->textPath = path;
    req->id = id;
    req->fontGroupIndex = (pexTableIndex) fontGroup;
    req->charExpansion = expansion;
    req->charSpacing = spacing;
    req->charHeight = height;
    req->textAlignment.vertical = valign;
    req->textAlignment.horizontal = halign;
    req->numStrings = count;


    /*
     * Update the request length header.
     */

    req->length += count * (LENOF (CARD32) + LENOF (pexMonoEncoding));
    for (i = 0; i < count; i++)
    {
	string = encoded_text[i].encoded_text;
	for (j = 0; j < (int) encoded_text[i].count; j++, string++)
	{
	    if (string->character_set_width == PEXCSLong) 
		req->length += string->string.length;
	    else if (string->character_set_width == PEXCSShort) 
		req->length += ((int) string->string.length + 1) >> 1;
	    else /* string->character_set_width == PEXCSByte) */ 
		req->length += ((int) string->string.length + 3) >> 2;
	} 
    }


    /*
     * Put the encoded text in the request.
     */

    for (i = 0; i < count; i++)
    {
	Data (display, (char *) &encoded_text[i].count, sizeof (CARD32));
	Data (display, (char *) (pexMonoEncoding *) &encoded_text[i],
	    sizeof (pexMonoEncoding));

	string = encoded_text[i].encoded_text;
	for (j = 0; j < (int) encoded_text[i].count; j++, string++)
	{
	    if (string->character_set_width == PEXCSLong) 
	    {
		Data (display, string->string.ch,
		    string->string.length * sizeof (long));
	    }
	    else if (string->character_set_width == PEXCSShort) 
	    {
		Data (display, string->string.ch,
		    string->string.length * sizeof (short));
	    }
	    else /* string->character_set_width == PEXCSByte) */ 
	    {
		Data (display, string->string.ch, string->string.length);
	    }
	}
    }


    /*
     * Get a reply.
     */

    if (_XReply (display, &rep, 0, xFalse) == 0)
    {
        UnlockDisplay (display);
        PEXSyncHandle (display);
        return (NULL);            /* return an error */
    }


    /*
     * Allocate a scratch buffer and copy the reply data to the buffer.
     */

    ch = (char *) _XAllocScratch (display, (unsigned long) (rep.length << 2));

    _XRead (display, ch, (long) (rep.length << 2));


    /*
     * Allocate a buffer for the replies to pass back to the client.
     */

    textExtent = ptextExtents =
	(PEXTextExtent *) PEXAllocBuf (count * sizeof (PEXTextExtent));

    for (i = 0; i < count; i++, textExtent++)
    {
	textExtent->lower_left = *(PEXCoord2D *) ch;
	ch += sizeof (PEXCoord2D);
	textExtent->upper_right = *(PEXCoord2D *) ch;
	ch += sizeof (PEXCoord2D);
	textExtent->concat_point = *(PEXCoord2D *) ch;
    }


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (ptextExtents);
}
