/* $XConsortium: XImText16.c,v 11.17 91/01/06 11:46:32 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

#include "Xlibint.h"

#if NeedFunctionPrototypes
XDrawImageString16(
    register Display *dpy,
    Drawable d,
    GC gc,
    int x,
    int y,
    _Xconst XChar2b *string,
    int length)
#else
XDrawImageString16(dpy, d, gc, x, y, string, length)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x, y;
    XChar2b *string;
    int length;
#endif
{   
    register xImageText16Req *req;
    XChar2b *CharacterOffset = (XChar2b *)string;
    int FirstTimeThrough = True;
    int lastX = 0;

    LockDisplay(dpy);
    FlushGC(dpy, gc);

    while (length > 0) 
    {
	int Unit, Datalength;

	if (length > 255) Unit = 255;
	else Unit = length;

   	if (FirstTimeThrough)
	{
	    FirstTimeThrough = False;
        }
	else
	{
	    int direction, ascent, descent;
	    XCharStruct overall;
	    XFontStruct *FontStruct;
            
	    UnlockDisplay(dpy);
	
	    overall.width = 0;
	    FontStruct = XQueryFont(dpy, gc->gid);

	    if (FontStruct) {
		XTextExtents16(FontStruct, CharacterOffset - 255, 255,
		    &direction, &ascent, &descent, &overall);

		if (FontStruct->per_char)
		    Xfree ((char *) FontStruct->per_char);
		if (FontStruct->properties)
		    Xfree ((char *) FontStruct->properties);
		Xfree((char *)FontStruct);
	    }

	    LockDisplay(dpy);

	    x = lastX + overall.width;
	}


        GetReq (ImageText16, req);
        req->length += ((Unit << 1) + 3) >> 2;
        req->nChars = Unit;
        req->drawable = d;
        req->gc = gc->gid;
        req->y = y;

	lastX = req->x = x;
	Datalength = Unit << 1;
        Data (dpy, (char *)CharacterOffset, (long)Datalength);
        CharacterOffset += Unit;
	length -= Unit;
    }
    UnlockDisplay(dpy);
    SyncHandle();
}

