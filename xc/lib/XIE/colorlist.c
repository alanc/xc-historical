/* $XConsortium: colorlist.c,v 1.3 94/03/09 15:02:38 mor Exp $ */

/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology

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
******************************************************************************/

#include "XIElibint.h"


XieColorList
XieCreateColorList (display)

Display	*display;

{
    xieCreateColorListReq	*req;
    char			*pBuf;
    XieColorList		id;

    LockDisplay (display);

    id = XAllocID (display);

    GET_REQUEST (CreateColorList, pBuf);

    BEGIN_REQUEST_HEADER (CreateColorList, pBuf, req);

    STORE_REQUEST_HEADER (CreateColorList, req);
    req->colorList = id;

    END_REQUEST_HEADER (CreateColorList, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (id);
}


void
XieDestroyColorList (display, color_list)

Display		*display;
XieColorList	color_list;

{
    xieDestroyColorListReq	*req;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (DestroyColorList, pBuf);

    BEGIN_REQUEST_HEADER (DestroyColorList, pBuf, req);

    STORE_REQUEST_HEADER (DestroyColorList, req);
    req->colorList = color_list;

    END_REQUEST_HEADER (DestroyColorList, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


void
XiePurgeColorList (display, color_list)

Display		*display;
XieColorList	color_list;

{
    xiePurgeColorListReq	*req;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (PurgeColorList, pBuf);

    BEGIN_REQUEST_HEADER (PurgeColorList, pBuf, req);

    STORE_REQUEST_HEADER (PurgeColorList, req);
    req->colorList = color_list;

    END_REQUEST_HEADER (PurgeColorList, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


Status
XieQueryColorList (display, color_list, colormap_ret, ncolors_ret, colors_ret)

Display    	*display;
XieColorList  	color_list;
Colormap   	*colormap_ret;
unsigned   	*ncolors_ret;
unsigned long  	**colors_ret;

{
    xieQueryColorListReq	*req;
    xieQueryColorListReply	rep;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (QueryColorList, pBuf);

    BEGIN_REQUEST_HEADER (QueryColorList, pBuf, req);

    STORE_REQUEST_HEADER (QueryColorList, req);
    req->colorList = color_list;

    END_REQUEST_HEADER (QueryColorList, pBuf, req);

    if (_XReply (display, (xReply *)&rep, 0, xFalse) == 0)
    {
        UnlockDisplay (display);
	SYNC_HANDLE (display);

	*colormap_ret = 0;
	*ncolors_ret = 0;
	*colors_ret = NULL;

	return (0);
    }

    *colormap_ret = rep.colormap;
    *ncolors_ret = rep.length;

    if (*ncolors_ret)
    {
      *colors_ret = (unsigned long *) Xmalloc (
	rep.length * sizeof (unsigned long));

      _XRead32 (display, colors_ret, rep.length << 2);
    }
    else 
	*colors_ret = NULL;

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (1);
}
