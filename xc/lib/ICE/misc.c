/* $XConsortium: misc.c,v 1.12 93/12/07 11:04:11 mor Exp $ */
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

#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICElibint.h>
#include <stdio.h>
#include <sys/types.h>


#ifndef WIN32

#include <sys/socket.h>

#ifdef UNIXCONN
#include <sys/un.h>
#endif

#ifdef TCPCONN
# include <sys/param.h>
# include <netinet/in.h>
# ifndef hpux
#  ifdef apollo
#   ifndef NO_TCP_H
#    include <netinet/tcp.h>
#   endif
#  else
#   include <netinet/tcp.h>
#  endif
# endif
#endif

#ifdef DNETCONN
#include <netdnet/dn.h>
#endif

#else /* WIN32 */

#include <errno.h>
#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
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
    return (iceConn->fd);
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
#ifdef WIN32
	int nread = recv (iceConn->fd, ptr, (int) nleft, 0);
#else
	int nread = read (iceConn->fd, ptr, (int) nleft);
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
		 * Fatal IO error.  First notify each protocol's IceIOErrorProc
		 * callback, then invoke the application IO error handler.
		 */

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
 * Write "n" bytes to a descriptor.
 * Use in place of write() when fd is a stream socket.
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
#ifdef WIN32
	int nwritten = send (iceConn->fd, ptr, (int) nleft, 0);
#else
	int nwritten = write (iceConn->fd, ptr, (int) nleft);
#endif

	if (nwritten <= 0)
	{
#ifdef WIN32
	    errno = WSAGetLastError();
#endif
	    /*
	     * Fatal IO error.  First notify each protocol's IceIOErrorProc
	     * callback, then invoke the application IO error handler.
	     */

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


	    /*
	     * All IO error handlers must either exit() or do
	     * a long jump.  If we reached this point, we have
	     * no choice but to exit().
	     */

	    exit (1);
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
    char *name = NULL;
    char *temp;

    union {
	struct sockaddr sa;
#ifdef TCPCONN
	struct sockaddr_in in;
#endif /* TCPCONN */
#ifdef DNETCONN
	struct sockaddr_dn dn;
#endif /* DNETCONN */
    } from;
    int	fromlen = sizeof (from);

#ifdef UNIXCONN
    if (iceConn->listen_obj->local_conn)
    {
	char hostnamebuf[256];

	if (gethostname (hostnamebuf, sizeof (hostnamebuf)) == 0)
	{
	    name = (char *) malloc (7 + strlen (hostnamebuf));
	    if (name)
		sprintf (name, "local/%s", hostnamebuf);
	}
    }
    else
#endif
    {
	if (getpeername (iceConn->fd, &from.sa, &fromlen) == -1)
	    return (NULL);

	switch (from.sa.sa_family)
	{
#ifdef TCPCONN
	case AF_INET:
	    temp = (char *) inet_ntoa (from.in.sin_addr);
	    name = (char *) malloc (strlen (temp) + 5);
	    if (name)
		sprintf (name, "tcp/%s", temp);
	    break;
#endif

#ifdef DNETCONN
	case AF_DECnet:
	    temp = (char *) dnet_ntoa (&from.dn.sdn_add);
	    name = (char *) malloc (strlen (temp) + 8);
	    if (name)
		sprintf (name, "decnet/%s", temp);
	    break;
#endif

	default:
	    break;
	}
    }

    return (name);
}
