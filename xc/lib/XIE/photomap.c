/* $XConsortium: photomap.c,v 1.4 94/01/12 19:44:38 rws Exp $ */

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


XiePhotomap
XieCreatePhotomap (display)

Display	*display;

{
    xieCreatePhotomapReq	*req;
    char			*pBuf;
    XiePhotomap			id;

    LockDisplay (display);

    id = XAllocID (display);

    GET_REQUEST (CreatePhotomap, pBuf);

    BEGIN_REQUEST_HEADER (CreatePhotomap, pBuf, req);

    STORE_REQUEST_HEADER (CreatePhotomap, req);
    req->photomap = id;

    END_REQUEST_HEADER (CreatePhotomap, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (id);
}


void
XieDestroyPhotomap (display, photomap)

Display		*display;
XiePhotomap	photomap;

{
    xieDestroyPhotomapReq	*req;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (DestroyPhotomap, pBuf);

    BEGIN_REQUEST_HEADER (DestroyPhotomap, pBuf, req);

    STORE_REQUEST_HEADER (DestroyPhotomap, req);
    req->photomap = photomap;

    END_REQUEST_HEADER (DestroyPhotomap, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


Status
XieQueryPhotomap (display, photomap, populated_ret, datatype_ret,
    dataclass_ret, decode_technique_ret, width_ret, height_ret, levels_ret)

Display      		*display;
XiePhotomap    		photomap;
Bool         		*populated_ret;
XieDataType   		*datatype_ret;
XieDataClass		*dataclass_ret;
XieDecodeTechnique	*decode_technique_ret;
XieLTriplet     	width_ret;
XieLTriplet     	height_ret;
XieLTriplet     	levels_ret;

{
    xieQueryPhotomapReq		*req;
    xieQueryPhotomapReply	rep;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (QueryPhotomap, pBuf);

    BEGIN_REQUEST_HEADER (QueryPhotomap, pBuf, req);

    STORE_REQUEST_HEADER (QueryPhotomap, req);
    req->photomap = photomap;

    END_REQUEST_HEADER (QueryPhotomap, pBuf, req);

    if (_XReply (display, (xReply *)&rep,
	(SIZEOF (xieQueryPhotomapReply) - 32) >> 2, xTrue) == 0)
    {
        UnlockDisplay (display);
	SYNC_HANDLE (display);

	return (0);
    }

    *populated_ret 	  = rep.populated;
    *datatype_ret         = rep.dataType;
    *dataclass_ret	  = rep.dataClass;
    *decode_technique_ret = rep.decodeTechnique;
    width_ret[0]      	  = rep.width0;
    width_ret[1]      	  = rep.width1;
    width_ret[2]          = rep.width2;
    height_ret[0]         = rep.height0;
    height_ret[1]         = rep.height1;
    height_ret[2]         = rep.height2;
    levels_ret[0]         = rep.levels0;
    levels_ret[1]         = rep.levels1;
    levels_ret[2]         = rep.levels2;

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (1);
}
