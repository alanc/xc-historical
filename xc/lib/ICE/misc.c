/* $XConsortium: misc.c,v 1.22 94/03/18 15:59:19 mor Exp $ */
/******************************************************************************

Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.

Author: Ralph Mor, X Consortium
******************************************************************************/

#ifdef WIN32
#define _WILLWINSOCK_
#endif
#include <X11/ICE/ICElib.h>
#include "ICElibint.h"
#include <X11/Xtrans.h>
#include <stdio.h>
#ifdef WIN32
#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#include <X11/Xw32defs.h>
#endif


/*
 * scratch buffer
 */

char *
IceAllocScratch (iceConn, size)

IceConn		iceConn;
unsigned long	size;

{
    if (!iceConn->scratch || size > iceConn->scratch_size)
    {
	if (iceConn->scratch)
	    free (iceConn->scratch);

	iceConn->scratch = (char *) malloc ((unsigned) size);
	iceConn->scratch_size = size;
    }

    return (iceConn->scratch);
}



/*
 * Output/Input buffer functions
 */

IceFlush (iceConn)

IceConn iceConn;

{
    _IceWrite (iceConn,
	(unsigned long) (iceConn->outbufptr - iceConn->outbuf),
	iceConn->outbuf);

    iceConn->outbufptr = iceConn->outbuf;
}


int
IceGetOutBufSize (iceConn)

IceConn iceConn;

{
    return (iceConn->outbufmax - iceConn->outbuf);
}


int
IceGetInBufSize (iceConn)

IceConn iceConn;

{
    return (iceConn->inbufmax - iceConn->inbuf);
}



/*
 * informational functions
 */

IceConnectStatus
IceConnectionStatus (iceConn)

IceConn iceConn;

{
    return (iceConn->connection_status);
}


char *
IceVendor (iceConn)

IceConn iceConn;

{
    char *string = (char *) malloc (strlen (iceConn->vendor) + 1);

    if (string)
	strcpy (string, iceConn->vendor);

    return (string);
}


char *
IceRelease (iceConn)

IceConn iceConn;

{
    char *string = (char *) malloc (strlen (iceConn->release) + 1);

    if (string)
	strcpy (string, iceConn->release);

    return (string);
}


int
IceProtocolVersion (iceConn)

IceConn iceConn;

{
    return (_IceVersions[iceConn->my_ice_version_index].major_version);
}


int
IceProtocolRevision (iceConn)

IceConn iceConn;

{
    return (_IceVersions[iceConn->my_ice_version_index].minor_version);
}


int
IceConnectionNumber (iceConn)

IceConn iceConn;

{
    return (_IceTransGetConnectionNumber (iceConn->trans_conn));
}


char *
IceConnectionString (iceConn)

IceConn iceConn;

{
    if (iceConn->connection_string)
    {
	char *string = (char *) malloc (
	    strlen (iceConn->connection_string) + 1);

	if (string)
	    strcpy (string, iceConn->connection_string);

	return (string);
    }
    else
	return (NULL);
}


unsigned long
IceLastSentSequenceNumber (iceConn)

IceConn iceConn;

{
    return (iceConn->send_sequence);
}


unsigned long
IceLastReceivedSequenceNumber (iceConn)

IceConn iceConn;

{
    return (iceConn->receive_sequence);
}


Bool
IceSwapping (iceConn)

IceConn iceConn;

{
    return (iceConn->swap);
}



/*
 * Read "n" bytes from a connection.
 *
 * Return Status 0 if we detected an EXPECTED closed connection.
 *
 */

Status
_IceRead (iceConn, nbytes, ptr)

register IceConn iceConn;
unsigned long	 nbytes;
register char	 *ptr;

{
    register unsigned long nleft;

    nleft = nbytes;
    while (nleft > 0)
    {
	int nread;

	if (iceConn->io_ok)
	    nread = _IceTransRead (iceConn->trans_conn, ptr, (int) nleft);
	else
	    return (1);

	if (nread <= 0)
	{
#ifdef WIN32
	    errno = WSAGetLastError();
#endif
	    if (iceConn->want_to_close)
	    {
		/*
		 * We sent a WantToClose message and now we detected that
		 * the other side closed the connection.
		 */

		_IceConnectionClosed (iceConn);	    /* invoke watch procs */
		_IceFreeConnection (iceConn);

		return (0);
	    }
	    else 
	    {
		/*
		 * Fatal IO error.  First notify each protocol's IceIOErrorProc
		 * callback, then invoke the application IO error handler.
		 */

		iceConn->io_ok = False;

		if (iceConn->process_msg_info)
		{
		    int i;

		    for (i = iceConn->his_min_opcode;
			i <= iceConn->his_max_opcode; i++)
		    {
			_IceProcessMsgInfo *process;

			process = &iceConn->process_msg_info[
			    i - iceConn->his_min_opcode];

			if (process->in_use)
			{
			    IceIOErrorProc IOErrProc = process->accept_flag ?
			      process->protocol->accept_client->io_error_proc :
			      process->protocol->orig_client->io_error_proc;

			    if (IOErrProc)
				(*IOErrProc) (iceConn);
			}
		    }
		}

		(*_IceIOErrorHandler) (iceConn);
		return (1);
	    }
	}

	nleft -= nread;
	ptr   += nread;
    }

    return (1);
}



/*
 * If we read a message header with a bad major or minor opcode,
 * we need to advance to the end of the message.  This way, the next
 * message can be processed correctly.
 */

void
_IceReadSkip (iceConn, nbytes)

register IceConn	iceConn;
register unsigned long	nbytes;

{
    char temp[512];

    while (nbytes > 0)
    {
	unsigned long rbytes = nbytes > 512 ? 512 : nbytes;

	_IceRead (iceConn, rbytes, temp);
	nbytes -= rbytes;
    }
}



/*
 * Write "n" bytes to a connection.
 */

void
_IceWrite (iceConn, nbytes, ptr)

register IceConn iceConn;
unsigned long	 nbytes;
register char	 *ptr;

{
    register unsigned long nleft;

    nleft = nbytes;
    while (nleft > 0)
    {
	int nwritten;

	if (iceConn->io_ok)
	    nwritten = _IceTransWrite (iceConn->trans_conn, ptr, (int) nleft);
	else
	    return;

	if (nwritten <= 0)
	{
#ifdef WIN32
	    errno = WSAGetLastError();
#endif
	    /*
	     * Fatal IO error.  First notify each protocol's IceIOErrorProc
	     * callback, then invoke the application IO error handler.
	     */

	    iceConn->io_ok = False;

	    if (iceConn->process_msg_info)
	    {
		int i;

		for (i = iceConn->his_min_opcode;
		     i <= iceConn->his_max_opcode; i++)
		{
		    _IceProcessMsgInfo *process;

		    process = &iceConn->process_msg_info[
			i - iceConn->his_min_opcode];

		    if (process->in_use)
		    {
			IceIOErrorProc IOErrProc = process->accept_flag ?
			    process->protocol->accept_client->io_error_proc :
			    process->protocol->orig_client->io_error_proc;

			if (IOErrProc)
			    (*IOErrProc) (iceConn);
		    }
		}
	    }

	    (*_IceIOErrorHandler) (iceConn);
	    return;
	}

	nleft -= nwritten;
	ptr   += nwritten;
    }
}

#ifdef WORD64

IceWriteData16 (iceConn, nbytes, data)

IceConn 	iceConn;
unsigned long 	nbytes;
short  		*data;

{
    int numShorts = nbytes / 2;
    int index = 0;

    while (index < numShorts)
    {
	int spaceLeft, count, i;
	int shortsLeft = numShorts - index;

	spaceLeft = iceConn->outbufmax - iceConn->outbufptr - 1;

	if (spaceLeft < 2)
	{
	    IceFlush (iceConn);
	    spaceLeft = iceConn->outbufmax - iceConn->outbufptr - 1;
	}

	count = (shortsLeft < spaceLeft / 2) ? shortsLeft : spaceLeft / 2;

	for (i = 0; i < count; i++)
	    STORE_CARD16 (iceConn->outbufptr, data[index++]);
    }
}


IceWriteData32 (iceConn, nbytes, data)

IceConn 	iceConn;
unsigned long  	nbytes;
int	 	*data;

{
    int numLongs = nbytes / 4;
    int index = 0;

    while (index < numLongs)
    {
	int spaceLeft, count, i;
	int longsLeft = numLongs - index;

	spaceLeft = iceConn->outbufmax - iceConn->outbufptr - 1;

	if (spaceLeft < 4)
	{
	    IceFlush (iceConn);
	    spaceLeft = iceConn->outbufmax - iceConn->outbufptr - 1;
	}

	count = (longsLeft < spaceLeft / 4) ? longsLeft : spaceLeft / 4;

	for (i = 0; i < count; i++)
	    STORE_CARD32 (iceConn->outbufptr, data[index++]);
    }
}


IceReadData16 (iceConn, swap, nbytes, data)

IceConn 	iceConn;
Bool		swap;
unsigned long 	nbytes;
short  		*data;

{
    /* NOT IMPLEMENTED YET */
}


IceReadData32 (iceConn, swap, nbytes, data)

IceConn 	iceConn;
Bool		swap;
unsigned long  	nbytes;
int	 	*data;

{
    /* NOT IMPLEMENTED YET */
}

#endif  /* WORD64 */



void
_IceAddOpcodeMapping (iceConn, hisOpcode, myOpcode)

IceConn	iceConn;
int 	hisOpcode;
int 	myOpcode;

{
    if (hisOpcode <= 0 || hisOpcode > 255)
    {
	return;
    }
    else if (iceConn->process_msg_info == NULL)
    {
	iceConn->process_msg_info = (_IceProcessMsgInfo *) malloc (
	    sizeof (_IceProcessMsgInfo));
	iceConn->his_min_opcode = iceConn->his_max_opcode = hisOpcode;
    }
    else if (hisOpcode < iceConn->his_min_opcode)
    {
	_IceProcessMsgInfo *oldVec = iceConn->process_msg_info;
	int oldsize = iceConn->his_max_opcode - iceConn->his_min_opcode + 1;
	int newsize = iceConn->his_max_opcode - hisOpcode + 1;
	int i;

	iceConn->process_msg_info = (_IceProcessMsgInfo *) malloc (
	    newsize * sizeof (_IceProcessMsgInfo));

	memcpy (&iceConn->process_msg_info[
	    iceConn->his_min_opcode - hisOpcode], oldVec,
	    oldsize * sizeof (_IceProcessMsgInfo));

	free ((char *) oldVec);

	for (i = hisOpcode + 1; i < iceConn->his_min_opcode; i++)
	{
	    iceConn->process_msg_info[i -
		iceConn->his_min_opcode].in_use = False;

	    iceConn->process_msg_info[i -
		iceConn->his_min_opcode].protocol = NULL;
	}

	iceConn->his_min_opcode = hisOpcode;
    }
    else if (hisOpcode > iceConn->his_max_opcode)
    {
	_IceProcessMsgInfo *oldVec = iceConn->process_msg_info;
	int oldsize = iceConn->his_max_opcode - iceConn->his_min_opcode + 1;
	int newsize = hisOpcode - iceConn->his_min_opcode + 1;
	int i;

	iceConn->process_msg_info = (_IceProcessMsgInfo *) malloc (
	    newsize * sizeof (_IceProcessMsgInfo));

	memcpy (iceConn->process_msg_info, oldVec,
	    oldsize * sizeof (_IceProcessMsgInfo));

	free ((char *) oldVec);

	for (i = iceConn->his_max_opcode + 1; i < hisOpcode; i++)
	{
	    iceConn->process_msg_info[i -
		iceConn->his_min_opcode].in_use = False;

	    iceConn->process_msg_info[i -
		iceConn->his_min_opcode].protocol = NULL;
	}

	iceConn->his_max_opcode = hisOpcode;
    }

    iceConn->process_msg_info[hisOpcode -
	iceConn->his_min_opcode].in_use = True;

    iceConn->process_msg_info[hisOpcode -
	iceConn->his_min_opcode].my_opcode = myOpcode;

    iceConn->process_msg_info[hisOpcode -
	iceConn->his_min_opcode].protocol = &_IceProtocols[myOpcode - 1];
}



char *
_IceGetPeerName (iceConn)

IceConn iceConn;

{
    int		family, peer_addrlen;
    Xtransaddr	*peer_addr;
    char 	*networkId;


    if (_IceTransGetPeerAddr (iceConn->trans_conn,
	&family, &peer_addrlen, &peer_addr) < 0)
    {
	return (NULL);
    }
    else
    {
	networkId = _IceTransGetPeerNetworkId (
	    family, peer_addrlen, peer_addr);

	free (peer_addr);

	return (networkId);
    }
}
