/* $XConsortium: pl_escape.c,v 1.2 92/05/07 23:28:10 mor Exp $ */

/************************************************************************
Copyright 1992 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

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
    pexEscapeReq		*req;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    PEXGetReqExtra (Escape, escapeDataSize, req);
    req->escapeID = escapeID;

    COPY_AREA ((char *) escapeData, ((char *) &req[1]), escapeDataSize);


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);
}


char *
PEXEscapeWithReply (display, escapeID, escapeDataSize, escapeData,
    escapeOutID, escapeOutDataSize)

INPUT Display		*display;
INPUT unsigned long  	escapeID;
INPUT int		escapeDataSize;
INPUT char		*escapeData;
OUTPUT unsigned long  	*escapeOutID;
OUTPUT int		*escapeOutDataSize;

{
    pexEscapeWithReplyReq		*req;
    pexEscapeWithReplyReply		rep;
    char				*escRepData, *pescRepData;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer and get a reply.
     */

    PEXGetReqExtra (EscapeWithReply, escapeDataSize, req);
    req->escapeID = escapeID;

    COPY_AREA ((char *) escapeData, ((char *) &req[1]), escapeDataSize);

    if (_XReply (display, &rep, 0, xFalse) == 0)
    {
        UnlockDisplay (display);
        PEXSyncHandle (display);
	*escapeOutDataSize = 0;
        return (NULL);               /* return an error */
    }

    *escapeOutID = rep.escapeID;
    *escapeOutDataSize = 20 + (rep.length << 2);


    /*
     * Allocate a buffer for the reply escape data
     */

    escRepData = pescRepData = PEXAllocBuf ((unsigned) *escapeOutDataSize);

    COPY_AREA ((char *) rep.escape_specific, escRepData, 20);
    escRepData += 20;

    if (rep.length)
	_XRead (display, escRepData, (long) (rep.length << 2));


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (pescRepData);
}


void PEXSetEchoColor (display, renderer, color_type, color)

INPUT Display		*display;
INPUT PEXRenderer	renderer;
INPUT int		color_type;
INPUT PEXColor		*color;

{
    PEXEchoColorData	escapeData;
    int			escapeSize;


    /*
     * Fill in the escape record.
     */

    escapeData.fp_format = PEXGetProtocolFloatFormat (display);
    escapeData.renderer = renderer;
    escapeData.echo_color.color_type = color_type;

    COPY_AREA ((char *) color, (char *) &(escapeData.echo_color.color),
	GetColorSize (color_type));


    /*
     * Generate the escape.
     */

    escapeSize = sizeof (PEXEchoColorData) - AdjustSizeFromType (color_type);

    PEXEscape (display, PEXEscapeSetEchoColor, escapeSize,
        (char *) &escapeData);
}
