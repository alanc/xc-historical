/* $XConsortium: misc.c,v 1.1 93/08/17 18:58:58 mor Exp $ */
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
 */

void
_IceRead (iceConn, nbytes, ptr)

register IceConn iceConn;
register int	 nbytes;
register char	 *ptr;

{
    int	nleft, nread;

    nleft = nbytes;
    while (nleft > 0)
    {
	nread = read (iceConn->fd, ptr, nleft);

	if (nread <= 0)
	{
	    (*_IceIOErrorHandler) (iceConn);
	    exit (1);
	}

	nleft -= nread;
	ptr   += nread;
    }
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
	nwritten = write (iceConn->fd, ptr, nleft);

	if (nwritten <= 0)
	{
	    (*_IceIOErrorHandler) (iceConn);
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



void
_IceFreeConnection (iceConn)

IceConn iceConn;

{
    if (iceConn)
    {
	if (iceConn->fd >= 0)
	    close (iceConn->fd);

	if (iceConn->connection_string)
	    free (iceConn->connection_string);

	if (iceConn->vendor)
	    free (iceConn->vendor);

	if (iceConn->release)
	    free (iceConn->release);

	if (iceConn->inbuf)
	    free (iceConn->inbuf);

	if (iceConn->outbuf)
	    free (iceConn->outbuf);

	if (iceConn->scratch)
	    free (iceConn->scratch);

	if (iceConn->process_msg_info)
	    free ((char *) iceConn->process_msg_info);

	if (iceConn->connect_to_you)
	    free ((char *) iceConn->connect_to_you);

	if (iceConn->protosetup_to_you)
	    free ((char *) iceConn->protosetup_to_you);

	if (iceConn->connect_to_me)
	    free ((char *) iceConn->connect_to_me);

	if (iceConn->protosetup_to_me)
	    free ((char *) iceConn->protosetup_to_me);

	free ((char *) iceConn);
    }
}
