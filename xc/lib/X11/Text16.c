#include "copyright.h"

/* $XConsortium: XText16.c,v 11.18 89/10/09 17:14:43 jim Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

#if NeedFunctionPrototypes
XDrawString16(
    register Display *dpy,
    Drawable d,
    GC gc,
    int x,
    int y,
    const XChar2b *string,
    int length)
#else
XDrawString16(dpy, d, gc, x, y, string, length)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x, y;
    XChar2b *string;
    int length;
#endif
{   
    int Datalength = 0;
    register xPolyText16Req *req;

    if (length <= 0)
       return;

    LockDisplay(dpy);
    FlushGC(dpy, gc);
    GetReq (PolyText16, req);
    req->drawable = d;
    req->gc = gc->gid;
    req->x = x;
    req->y = y;


    Datalength += SIZEOF(xTextElt) * ((length + 253) / 254) + (length << 1);


    req->length += (Datalength + 3)>>2;  /* convert to number of 32-bit words */


    /* 
     * If the entire request does not fit into the remaining space in the
     * buffer, flush the buffer first.   If the request does fit into the
     * empty buffer, then we won't have to flush it at the end to keep
     * the buffer 32-bit aligned. 
     */

    if (dpy->bufptr + Datalength > dpy->bufmax)
    	_XFlush (dpy);

    {
	int nbytes;
	int PartialNChars = length;
        register xTextElt *elt;
 	XChar2b *CharacterOffset = (XChar2b *)string;

	while(PartialNChars > 254)
        {
 	    nbytes = 254 * 2 + SIZEOF(xTextElt);
	    BufAlloc (xTextElt *, elt, nbytes);
	    elt->delta = 0;
	    elt->len = 254;
#ifdef MUSTCOPY
	    {
		register int i;
		register unsigned char *cp;
		for (i = 0, cp = ((unsigned char *)elt) + 2; i < 254; i++) {
		    *cp++ = CharacterOffset[i].byte1;
		    *cp++ = CharacterOffset[i].byte2;
		}
	    }
#else
            bcopy ((char *)CharacterOffset, (char *) (elt + 1), 254 * 2);
#endif
	    PartialNChars = PartialNChars - 254;
	    CharacterOffset += 254;
	}
	    
        if (PartialNChars)
        {
	    nbytes = PartialNChars * 2  + SIZEOF(xTextElt);
	    BufAlloc (xTextElt *, elt, nbytes); 
	    elt->delta = 0;
	    elt->len = PartialNChars;
#ifdef MUSTCOPY
	    {
		register int i;
		register unsigned char *cp;
		for (i = 0, cp = ((unsigned char *)elt) + 2; i < PartialNChars;
		     i++) {
		    *cp++ = CharacterOffset[i].byte1;
		    *cp++ = CharacterOffset[i].byte2;
		}
	    }
#else
            bcopy ((char *)CharacterOffset, (char *) (elt + 1), PartialNChars * 2);
#endif
	 }
    }

    /* Pad request out to a 32-bit boundary */

    if (Datalength &= 3) {
	char *pad;
	/* 
	 * BufAlloc is a macro that uses its last argument more than
	 * once, otherwise I'd write "BufAlloc (char *, pad, 4-length)" 
	 */
	length = 4 - Datalength;
	BufAlloc (char *, pad, length);
	/* 
	 * if there are 3 bytes of padding, the first byte MUST be 0
	 * so the pad bytes aren't mistaken for a final xTextElt 
	 */
	*pad = 0;
        }

    /* 
     * If the buffer pointer is not now pointing to a 32-bit boundary,
     * we must flush the buffer so that it does point to a 32-bit boundary
     * at the end of this routine. 
     */

    if ((dpy->bufptr - dpy->buffer) & 3)
       _XFlush (dpy);
    UnlockDisplay(dpy);
    SyncHandle();
    return;
}

