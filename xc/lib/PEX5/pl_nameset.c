/* $XConsortium: pl_nameset.c,v 1.11 92/05/07 23:28:41 mor Exp $ */

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


PEXNameSet
PEXCreateNameSet (display)

INPUT Display		*display;

{
    pexCreateNameSetReq	*req;
    PEXNameSet		ns;


    /*
     * Get a nameset resource id from X.
     */

    ns = XAllocID (display);


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    PEXGetReq (CreateNameSet, req);
    req->id = ns;


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (ns);
}


void
PEXFreeNameSet (display, ns)

INPUT Display		*display;
INPUT PEXNameSet	ns;

{
    pexFreeNameSetReq	*req;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    PEXGetReq (FreeNameSet, req);
    req->id = ns;


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);
}


void
PEXCopyNameSet (display, srcNs, destNs)

INPUT Display		*display;
INPUT PEXNameSet	srcNs;
INPUT PEXNameSet	destNs;

{
    pexCopyNameSetReq	*req;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    PEXGetReq (CopyNameSet, req);
    req->src = srcNs;
    req->dst = destNs;


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);
}


PEXName *
PEXGetNameSet (display, ns, numNamesReturn)

INPUT Display		*display;
INPUT PEXNameSet	ns;
OUTPUT unsigned long	*numNamesReturn;

{
    pexGetNameSetReq	*req;
    pexGetNameSetReply	rep;
    PEXName		*pn;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer and get a reply.
     */

    PEXGetReq (GetNameSet, req);
    req->id = ns;

    if (_XReply (display,  &rep, 0, xFalse) == 0)
    {
        UnlockDisplay (display);
        PEXSyncHandle (display);
	*numNamesReturn = 0;
	return (NULL); 		/* return an error */
    }

    *numNamesReturn = rep.numNames;


    /*
     * Allocate a buffer for the replies to pass back to the user.
     */

    pn = (PEXName *) PEXAllocBuf ((unsigned) (rep.length << 2));

    if (rep.numNames)
        _XRead (display, (char *) pn, (long) (rep.length << 2));


   /*
    * Done, so unlock and check for synchronous-ness.
    */

    UnlockDisplay (display);
    PEXSyncHandle (display);

    return (pn);
}


void
PEXChangeNameSet (display, ns, action, numValues, values)

INPUT Display		*display;
INPUT PEXNameSet	ns;
INPUT int		action;
INPUT unsigned long	numValues;
INPUT PEXName		*values;

{
    pexChangeNameSetReq	*req;
    int			size;


    /*
     * Lock around the critical section, for multithreading.
     */

    LockDisplay (display);


    /*
     * Put the request in the X request buffer.
     */

    size = numValues * sizeof (PEXName);

    PEXGetReqExtra (ChangeNameSet, size, req);
    req->ns = ns;
    req->action = action;

    COPY_AREA ((char *) values, ((char *) &req[1]), size);


    /*
     * Done, so unlock and check for synchronous-ness.
     */

    UnlockDisplay (display);
    PEXSyncHandle (display);
}
