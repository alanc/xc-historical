/* $XConsortium: XImText.c,v 11.14 90/12/12 09:18:13 rws Exp $ */
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
XDrawImageString(
    register Display *dpy,
    Drawable d,
    GC gc,
    int x,
    int y,
    _Xconst char *string,
    int length)
#else
XDrawImageString(dpy, d, gc, x, y, string, length)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x, y;
    char *string;
    int length;
#endif
{   
    register xImageText8Req *req;
    char *CharacterOffset = (char *)string;
    int FirstTimeThrough = True;
    int lastX = 0;

    LockDisplay(dpy);
    FlushGC(dpy, gc);

    while (length > 0) 
    {
	int Unit;

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
	
	    FontStruct = XQueryFont(dpy, gc->gid);

  	    XTextExtents(FontStruct, CharacterOffset - 255, 255,
		&direction, &ascent, &descent, &overall);

	    Xfree(FontStruct);

	    LockDisplay(dpy);

	    x = lastX + overall.width;
	}


        GetReq (ImageText8, req);
        req->length += (Unit + 3) >> 2;
        req->nChars = Unit;
        req->drawable = d;
        req->gc = gc->gid;
        req->y = y;

	lastX = req->x = x;
        Data (dpy, CharacterOffset, (long)Unit);
        CharacterOffset += Unit;
	length -= Unit;
    }
    UnlockDisplay(dpy);
    SyncHandle();
}

