/* $XConsortium: pl_oc_util.c,v 1.2 92/05/20 21:06:55 mor Exp $ */

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
#include "pl_oc_util.h"


Status
PEXStartOCs (display, resource_id, req_type, float_format, numOCs, numWords)

INPUT Display		*display;
INPUT XID		resource_id;
INPUT PEXOCRequestType	req_type;
INPUT int		float_format;
INPUT int		numOCs;
INPUT int		numWords;

{
    PEXDisplayInfo 	*pexDisplayInfo;
    pexOCRequestHeader 	*currentReq;


    /*
     * Is the oc larger than the protocol max request size?
     * If so, return an error.
     */

    if (numWords + LENOF (pexOCRequestHeader) > MAX_REQUEST_SIZE)
        return (0);


    /*
     * If possible add the OC to a ROC or StoreElements request,
     * otherwise start a new request.
     */
    
    PEXGetDisplayInfo (display, pexDisplayInfo);
    LockDisplay (display);

    currentReq = (XBufferFlushed (display)) ? 
	(NULL) : ((pexOCRequestHeader *) (display->last_req));
    
    if (currentReq == NULL ||
	pexDisplayInfo->lastReqNum != display->request ||
	pexDisplayInfo->lastResID != resource_id ||
	pexDisplayInfo->lastReqType != req_type ||
	req_type == PEXOCRenderSingle || req_type == PEXOCStoreSingle ||
	(LENOF (pexOCRequestHeader) + numWords > WordsLeftInXBuffer (display)))
    {
	PEXGetOCReq (display,
	    sizeof (pexOCRequestHeader) + NUMBYTES (numWords));
	
	pexDisplayInfo->lastResID = resource_id;
	pexDisplayInfo->lastReqType = req_type;
	pexDisplayInfo->lastReqNum = display->request;
	
	currentReq = (pexOCRequestHeader *) (display->bufptr);

	currentReq->extOpcode = pexDisplayInfo->extOpcode;
	currentReq->pexOpcode =
	    (req_type == PEXOCStore || req_type == PEXOCStoreSingle) ?
	    PEXRCStoreElements : PEXRCRenderOutputCommands;
	currentReq->reqLength = LENOF (pexOCRequestHeader) + numWords;
	currentReq->fpFormat = float_format;
	currentReq->target = resource_id;
	currentReq->numCommands = numOCs;

        display->bufptr += sizeof (pexOCRequestHeader);
    }
    else
    {
	currentReq->reqLength += numWords;
	currentReq->numCommands += numOCs;
    }
    
    return (1);
}


#ifdef PEXFinishOC
#undef PEXFinishOC
#endif

void
PEXFinishOC (display)

INPUT Display	*display;

{
    UnlockDisplay (display);
}


void
PEXCopyBytesToOC (display, numBytes, data) 

INPUT Display		*display;
INPUT int		numBytes;
INPUT char		*data;

{
    PEXDisplayInfo 	*pexDisplayInfo;


    if (numBytes <= BytesLeftInXBuffer (display))
    {
	/*
	 * There is room in the X buffer to do the copy.
	 */

	COPY_AREA (data, display->bufptr, numBytes);
	display->bufptr += numBytes;
    }
    else
    {
	/*
	 * Copying this OC will overflow the transport buffer.  Using
	 * _XSend will take care of splitting the buffer into chunks
	 * small enough to fit in the transport buffer.
	 */
	
	_XSend (display, data, numBytes);


	/*
	 * Make sure that the next oc starts a new request.
	 */
	
	PEXGetDisplayInfo (display, pexDisplayInfo);
	pexDisplayInfo->lastReqNum = -1;
    }
}


/* INTERNAL */

void
_PEXCopyPaddedBytesToOC (display, numBytes, data) 

INPUT Display		*display;
INPUT int		numBytes;
INPUT char		*data;

{
    PEXDisplayInfo 	*pexDisplayInfo;
    int			paddedBytes = PADDED_BYTES (numBytes);


    if (paddedBytes <= BytesLeftInXBuffer (display))
    {
	/*
	 * There is room in the X buffer to do the copy.
	 */

	COPY_AREA (data, display->bufptr, numBytes);
	display->bufptr += paddedBytes;
    }
    else
    {
	/*
	 * Copying this OC will overflow the transport buffer.  Using
	 * _XSend will take care of splitting the buffer into chunks
	 * small enough to fit in the transport buffer.
	 */
	
	_XSend (display, data, numBytes);


	/*
	 * Make sure that the next oc starts a new request.
	 */
	
	PEXGetDisplayInfo (display, pexDisplayInfo);
	pexDisplayInfo->lastReqNum = -1;
    }
}


char *
PEXGetOCAddr (display, numBytes) 

INPUT Display		*display;
INPUT int		numBytes;

{
    PEXDisplayInfo 	*pexDisplayInfo;
    char		*retPtr;


    /*
     * If numBytes is larger than the max allowed size, return error.
     */

    if (numBytes >  PEXGetOCAddrMaxSize (display))
	return (NULL);


    /*
     * If there isn't enough space in the X buffer, flush it
     * and make sure that the next OC starts a new request.
     */

    if (numBytes > BytesLeftInXBuffer (display))
    {
	_XFlush (display);
	PEXGetDisplayInfo (display, pexDisplayInfo);
	pexDisplayInfo->lastReqNum = -1;
    }


    /*
     * Return a pointer to the bytes, and update the display's bufptr.
     */

    retPtr = display->bufptr;
    display->bufptr += numBytes;

    return (retPtr);
}


void
PEXSendOCs (display, resource_id, req_type, float_format,
    oc_count, length, encoded_ocs)

INPUT Display		*display;
INPUT XID		resource_id;
INPUT PEXOCRequestType	req_type;
INPUT int		float_format;
INPUT unsigned long	oc_count;
INPUT unsigned int	length;
INPUT char		*encoded_ocs;

{
    PEXStartOCs (display, resource_id, req_type, float_format,
	oc_count, length);

    PEXCopyBytesToOC (display, NUMBYTES (length), encoded_ocs);

    PEXFinishOC (display);
}



void _PEXGenOCBadLengthError (display, resource_id, req_type)

INPUT Display		*display;
INPUT XID		resource_id;
INPUT PEXOCRequestType	req_type;

{
    PEXDisplayInfo 	*pexDisplayInfo;
    pexOCRequestHeader 	*req;


    /*
     * Generate an OC request with a zero request length.
     */

    LockDisplay (display);

    PEXGetDisplayInfo (display, pexDisplayInfo);
    PEXGetOCReq (display, 0);
    req = (pexOCRequestHeader *) (display->bufptr);

    req->extOpcode = pexDisplayInfo->extOpcode;
    req->pexOpcode =
	(req_type == PEXOCStore || req_type == PEXOCStoreSingle) ?
	PEXRCStoreElements : PEXRCRenderOutputCommands;
    req->reqLength = 0;
    req->fpFormat = pexDisplayInfo->fpFormat;
    req->target = resource_id;
    req->numCommands = 1;


    /*
     * Make sure that the next oc starts a new request.
     */

    pexDisplayInfo->lastReqNum = -1;
    pexDisplayInfo->lastResID = resource_id;
    pexDisplayInfo->lastReqType = req_type;
	
    UnlockDisplay (display);
}
