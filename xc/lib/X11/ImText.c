#include "copyright.h"

/* $Header: XImText.c,v 11.10 87/08/26 16:21:52 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XDrawImageString(dpy, d, gc, x, y, string, length)
    register Display *dpy;
    Drawable d;
    GC gc;
    int x, y;
    char *string;
    int length;
{   
    register xImageText8Req *req;
    char *CharacterOffset = string;
    int FirstTimeThrough = True;
    int lastX;

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

