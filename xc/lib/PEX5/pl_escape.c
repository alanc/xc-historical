/* $XConsortium: pl_escape.c,v 1.5 92/10/22 18:12:26 mor Exp $ */

/************************************************************************
Copyright 1992 by the Massachusetts Institute of Technology

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*************************************************************************/

#include "PEXlib.h"
#include "PEXlibint.h"


void
PEXEscape (display, escapeID, escapeDataSize, escapeData)

INPUT Display		*display;
INPUT unsigned long  	escapeID;
INPUT int		escapeDataSize;
INPUT char		*escapeData;

{
    register pexEscapeReq	*req;
    char			*pBuf;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    PEXGetReqExtra (Escape, escapeDataSize, pBuf);

    BEGIN_REQUEST_HEADER (Escape, pBuf, req);

    PEXStoreReqExtraHead (Escape, escapeDataSize, req);
    req->escapeID = escapeID;

    END_REQUEST_HEADER (Escape, pBuf, req);

    COPY_AREA (escapeData, pBuf, escapeDataSize);


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);
}


char *
PEXEscapeWithReply (display, escapeID, escapeDataSize,
    escapeData, escapeOutDataSize)

INPUT Display		*display;
INPUT unsigned long  	escapeID;
INPUT int		escapeDataSize;
INPUT char		*escapeData;
OUTPUT unsigned long	*escapeOutDataSize;

{
    register pexEscapeWithReplyReq	*req;
    char				*pBuf;
    pexEscapeWithReplyReply		rep;
    char				*escRepData;
    char				*escRepDataRet;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer and get a reply.
     */

    PEXGetReqExtra (EscapeWithReply, escapeDataSize, pBuf);

    BEGIN_REQUEST_HEADER (EscapeWithReply, pBuf, req);

    PEXStoreReqExtraHead (EscapeWithReply, escapeDataSize, req);
    req->escapeID = escapeID;

    END_REQUEST_HEADER (EscapeWithReply, pBuf, req);

    COPY_AREA (escapeData, pBuf, escapeDataSize);

    if (_XReply (display, &rep, 0, xFalse) == 0)
    {
        UnlockDisplay (display);
        PEXSyncHandle (display);
	*escapeOutDataSize = 0;
        return (NULL);               /* return an error */
    }

    *escapeOutDataSize = 20 + (rep.length << 2);


    /*
     * Allocate a buffer for the reply escape data
     */

    escRepData = escRepDataRet = PEXAllocBuf (*escapeOutDataSize);

    COPY_AREA ((char *) rep.escape_specific, escRepData, 20);
    escRepData += 20;

    if (rep.length)
	_XRead (display, (char *) escRepData, (long) (rep.length << 2));


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (escRepDataRet);
}


void PEXSetEchoColor (display, renderer, color_type, color)

INPUT Display		*display;
INPUT PEXRenderer	renderer;
INPUT int		color_type;
INPUT PEXColor		*color;

{
    char			*escapeData;
    int				escapeSize;
    pexEscapeSetEchoColorData	*header;
    char			*ptr;
    int				fpConvert;
    int				fpFormat;


    /*
     * Fill in the escape record.
     */

    escapeSize = SIZEOF (pexEscapeSetEchoColorData) +
	SIZEOF (pexColorSpecifier) + GetColorSize (color_type);

    escapeData = PEXAllocBuf (escapeSize);

    fpFormat = PEXGetProtocolFloatFormat (display);
    fpConvert = (fpFormat != NATIVE_FP_FORMAT);

    header = (pexEscapeSetEchoColorData *) escapeData;
    header->fpFormat = fpFormat;
    header->rdr = renderer;

    ptr = escapeData + SIZEOF (pexEscapeSetEchoColorData);
    STORE_INT16 (color_type, ptr);
    ptr += 2;
    STORE_COLOR_VAL (color_type, (*color), ptr, fpConvert, fpFormat);


    /*
     * Generate the escape.
     */

    PEXEscape (display, PEXEscapeSetEchoColor, escapeSize, escapeData);

    PEXFreeBuf (escapeData);
}
