/* $XConsortium: misc.c,v 1.2 93/08/20 15:36:59 rws Exp $ */
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
******************************************************************************/

#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICElibint.h>
#include <stdio.h>

#ifdef WIN32
#include <errno.h>
#undef close
#define close closesocket
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

    strcpy (string, iceConn->vendor);

    return (string);
}


char *
IceRelease (iceConn)

IceConn iceConn;

{
    char *string = (char *) malloc (strlen (iceConn->release) + 1);

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
    return (iceConn->fd);
}


char *
IceConnectionString (iceConn)

IceConn iceConn;

{
    char *string = (char *) malloc (strlen (iceConn->connection_string) + 1);

    strcpy (string, iceConn->connection_string);

    return (string);
}


unsigned long
IceLastSequenceNumber (iceConn)

IceConn iceConn;

{
    return (iceConn->sequence);
}



/*
 * Read "n" bytes from a descriptor.
 * Use in place of read() when fd is a stream socket.
 *
 * Return Status 0 if we detected an EXPECTED closed connection.
 */

Status
_IceRead (iceConn, nbytes, ptr)

register IceConn iceConn;
register int	 nbytes;
register char	 *ptr;

{
    int	nleft, nread;

    nleft = nbytes;
    while (nleft > 0)
    {
#ifdef WIN32
	nread = recv (iceConn->fd, ptr, nleft, 0);
#else
	nread = read (iceConn->fd, ptr, nleft);
#endif
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

		_IceFreeConnection (iceConn, False);

		return (0);
	    }
	    else 
	    {
		/*
		 * Fatal IO error, invoke the IO error handler.
		 */

		(*_IceIOErrorHandler) (iceConn);


		/*
		 * All IO error handlers must either exit() or do
		 * a long jump.  If we reached this point, we have
		 * no choice but to exit().
		 */

		exit (1);
	    }
	}

	nleft -= nread;
	ptr   += nread;
    }

    return (1);
}



/*
 * Write "n" bytes to a descriptor.
 * Use in place of write() when fd is a stream socket.
 */

void
_IceWrite (iceConn, nbytes, ptr)

register IceConn iceConn;
register int	 nbytes;
register char	 *ptr;

{
    int	nleft, nwritten;

    nleft = nbytes;
    while (nleft > 0)
    {
#ifdef WIN32
	nwritten = send (iceConn->fd, ptr, nleft, 0);
#else
	nwritten = write (iceConn->fd, ptr, nleft);
#endif

	if (nwritten <= 0)
	{
#ifdef WIN32
	    errno = WSAGetLastError();
#endif
	    if (iceConn->want_to_close)
	    {
		/*
		 * We were about to send a WantToClose message and now we
		 * detected that the other side closed the connection,
		 * so we just close our side.
		 */

		_IceFreeConnection (iceConn, False);
		break;
	    }
	    else
	    {
		/*
		 * Fatal IO error, invoke the IO error handler.
		 */

		(*_IceIOErrorHandler) (iceConn);


		/*
		 * All IO error handlers must either exit() or do
		 * a long jump.  If we reached this point, we have
		 * no choice but to exit().
		 */

		exit (1);
	    }
	}

	nleft -= nwritten;
	ptr   += nwritten;
    }
}



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

	bcopy ((char *) oldVec, (char *) &iceConn->process_msg_info[
	    iceConn->his_min_opcode - hisOpcode],
	    oldsize * sizeof (_IceProcessMsgInfo));

	free ((char *) oldVec);

	for (i = hisOpcode + 1; i < iceConn->his_min_opcode; i++)
	{
	    iceConn->process_msg_info[i -
		iceConn->his_min_opcode].in_use = 0;

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

	bcopy ((char *) oldVec,
	    (char *) iceConn->process_msg_info,
	    oldsize * sizeof (_IceProcessMsgInfo));

	free ((char *) oldVec);

	for (i = iceConn->his_max_opcode + 1; i < hisOpcode; i++)
	{
	    iceConn->process_msg_info[i -
		iceConn->his_min_opcode].in_use = 0;

	    iceConn->process_msg_info[i -
		iceConn->his_min_opcode].protocol = NULL;
	}

	iceConn->his_max_opcode = hisOpcode;
    }

    iceConn->process_msg_info[hisOpcode -
	iceConn->his_min_opcode].in_use = 1;

    iceConn->process_msg_info[hisOpcode -
	iceConn->his_min_opcode].protocol = &_IceProtocols[myOpcode - 1];
}
