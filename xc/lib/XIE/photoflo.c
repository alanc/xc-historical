/* $XConsortium: photoflo.c,v 1.1 93/07/19 11:39:28 mor Exp $ */

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


XiePhotoElement *
XieAllocatePhotofloGraph (count)

unsigned int count;

{
    XiePhotoElement	*ptr;
    unsigned		size;

    size = count * sizeof (XiePhotoElement);
    ptr = (XiePhotoElement *) Xmalloc (size);
    bzero ((char *) ptr, size);

    return (ptr);
}


XiePhotoflo
XieCreatePhotoflo (display, elem_list, elem_count)

Display		*display;
XiePhotoElement	*elem_list;
int		elem_count;

{
    xieCreatePhotofloReq	*req;
    char			*pBuf, *pStart;
    unsigned			size;
    XiePhotoflo			id;
    int				i;

    LockDisplay (display);

    id = XAllocID (display);

    size = _XiePhotofloSize (elem_list, elem_count);

    GET_REQUEST (CreatePhotoflo, pBuf);

    BEGIN_REQUEST_HEADER (CreatePhotoflo, pBuf, req);

    STORE_REQUEST_EXTRA_HEADER (CreatePhotoflo, size, req);
    req->floID = id;
    req->numElements = elem_count;

    END_REQUEST_HEADER (CreatePhotoflo, pBuf, req);

    pBuf = pStart = _XAllocScratch (display, size);

    for (i = 0; i < elem_count; i++)
	(*_XieElemFuncs[elem_list[i].elemType - 1]) (&pBuf, &elem_list[i]);

    Data (display, pStart, size);

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (id);
}


void
XieDestroyPhotoflo (display, photoflo)

Display		*display;
XiePhotoflo	photoflo;

{
    xieDestroyPhotofloReq	*req;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (DestroyPhotoflo, pBuf);

    BEGIN_REQUEST_HEADER (DestroyPhotoflo, pBuf, req);

    STORE_REQUEST_HEADER (DestroyPhotoflo, req);
    req->floID = photoflo;

    END_REQUEST_HEADER (DestroyPhotoflo, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


void
XieExecutePhotoflo (display, photoflo, notify)

Display		*display;
XiePhotoflo	photoflo;
Bool		notify;

{
    xieExecutePhotofloReq	*req;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (ExecutePhotoflo, pBuf);

    BEGIN_REQUEST_HEADER (ExecutePhotoflo, pBuf, req);

    STORE_REQUEST_HEADER (ExecutePhotoflo, req);
    req->floID = photoflo;
    req->notify = notify;

    END_REQUEST_HEADER (ExecutePhotoflo, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


void
XieModifyPhotoflo (display, photoflo, start, elem_list, elem_count)

Display		*display;
XiePhotoflo	photoflo;
int		start;
XiePhotoElement	*elem_list;
int		elem_count;

{
    xieModifyPhotofloReq	*req;
    char			*pBuf, *pStart;
    unsigned			size;
    int				i;

    LockDisplay (display);

    size = _XiePhotofloSize (elem_list, elem_count);

    GET_REQUEST (ModifyPhotoflo, pBuf);

    BEGIN_REQUEST_HEADER (ModifyPhotoflo, pBuf, req);

    STORE_REQUEST_EXTRA_HEADER (ModifyPhotoflo, size, req);
    req->floID = photoflo;
    req->start = start;
    req->numElements = elem_count;

    END_REQUEST_HEADER (ModifyPhotoflo, pBuf, req);

    pBuf = pStart = _XAllocScratch (display, size);

    for (i = 0; i < elem_count; i++)
	(*_XieElemFuncs[elem_list[i].elemType - 1]) (&pBuf, &elem_list[i]);

    Data (display, pStart, size);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


void
XieRedefinePhotoflo (display, photoflo, elem_list, elem_count)

Display		*display;
XiePhotoflo	photoflo;
XiePhotoElement	*elem_list;
int		elem_count;

{
    xieRedefinePhotofloReq	*req;
    char			*pBuf, *pStart;
    unsigned			size;
    int				i;

    LockDisplay (display);

    size = _XiePhotofloSize (elem_list, elem_count);

    GET_REQUEST (RedefinePhotoflo, pBuf);

    BEGIN_REQUEST_HEADER (RedefinePhotoflo, pBuf, req);

    STORE_REQUEST_EXTRA_HEADER (RedefinePhotoflo, size, req);
    req->floID = photoflo;
    req->numElements = elem_count;

    END_REQUEST_HEADER (RedefinePhotoflo, pBuf, req);

    pBuf = pStart = _XAllocScratch (display, size);

    for (i = 0; i < elem_count; i++)
	(*_XieElemFuncs[elem_list[i].elemType - 1]) (&pBuf, &elem_list[i]);

    Data (display, pStart, size);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}


Status
XieQueryPhotoflo (display, name_space, flo_id,
    state_ret, data_expected_ret, nexpected_ret,
    data_available_ret, navailable_ret)

Display      		*display;
unsigned long		name_space;
unsigned long		flo_id;
XiePhotofloState	*state_ret;
XiePhototag		**data_expected_ret;
unsigned int    	*nexpected_ret;
XiePhototag		**data_available_ret;
unsigned int    	*navailable_ret;

{
    xieQueryPhotofloReq		*req;
    xieQueryPhotofloReply	rep;
    char			*pBuf;

    LockDisplay (display);

    GET_REQUEST (QueryPhotoflo, pBuf);

    BEGIN_REQUEST_HEADER (QueryPhotoflo, pBuf, req);

    STORE_REQUEST_HEADER (QueryPhotoflo, req);
    req->nameSpace = name_space;
    req->floID = flo_id;

    END_REQUEST_HEADER (QueryPhotoflo, pBuf, req);

    if (_XReply (display, &rep, 0, xFalse) == 0)
    {
        UnlockDisplay (display);
	SYNC_HANDLE (display);

	*state_ret = 0;
	*nexpected_ret = 0;
	*data_expected_ret = NULL;
	*navailable_ret = 0;
	*data_available_ret = NULL;
	
	return (0);
    }

    *state_ret = rep.state;
    *nexpected_ret = rep.expectedCount;
    *navailable_ret = rep.availableCount;

    *data_expected_ret = (XiePhototag *) Xmalloc (
	sizeof (XiePhototag) * rep.expectedCount);

    *data_available_ret = (XiePhototag *) Xmalloc (
	sizeof (XiePhototag) * rep.availableCount);

    _XRead16Pad (display, data_expected_ret,
	rep.expectedCount * SIZEOF (xieTypPhototag));

    _XRead16Pad (display, data_available_ret,
	rep.availableCount * SIZEOF (xieTypPhototag));

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (1);
}
