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

#include <X11/SM/SMlib.h>
#include <X11/SM/SMlibint.h>
#include <stdio.h>


/*
 * Free properties
 */

void
SmFreeProperties (numProps, props)

int	numProps;
SmProp	*props;

{
    if (props)
    {
	int i, j;

	for (i = 0; i < numProps; i++)
	{
	    if (props[i].name)
		free (props[i].name);
	    if (props[i].type)
		free (props[i].type);
	    if (props[i].vals)
	    {
		for (j = 0; j < props[i].num_vals; j++)
		    if (props[i].vals[j].value)
			free ((char *) props[i].vals[j].value);
		free ((char *) props[i].vals);
	    }
	}

	free ((char *) props);
    }
}


/*
 * Free reason messages
 */

void
SmFreeReasons (count, reasonMsgs)

int 	count;
char 	**reasonMsgs;

{
    if (reasonMsgs)
    {
	int i;

	for (i = 0; i < count; i++)
	    free (reasonMsgs[i]);

	free ((char *) reasonMsgs);
    }
}



/*
 * Smc informational functions
 */

int
SmcProtocolVersion (smcConn)

SmcConn smcConn;

{
    return (smcConn->proto_major_version);
}


int
SmcProtocolRevision (smcConn)

SmcConn smcConn;

{
    return (smcConn->proto_minor_version);
}


char *
SmcVendor (smcConn)

SmcConn smcConn;

{
    return (IceVendor (smcConn->iceConn));
}


char *
SmcRelease (smcConn)

SmcConn smcConn;

{
    return (IceRelease (smcConn->iceConn));
}


char *
SmcConnectionString (smcConn)

SmcConn smcConn;

{
    return (IceConnectionString (smcConn->iceConn));
}


unsigned long
SmcLastSequenceNumber (smcConn)

SmcConn smcConn;

{
    return (IceLastSequenceNumber (smcConn->iceConn));
}


void
SmcClientID (smcConn, clientIdLenRet, clientIdRet)

SmcConn smcConn;
int	*clientIdLenRet;
char	**clientIdRet;

{
    *clientIdLenRet = smcConn->client_id_len;
    *clientIdRet = (char *) malloc (smcConn->client_id_len + 1);
    bcopy (smcConn->client_id, *clientIdRet, smcConn->client_id_len);
    (*clientIdRet)[smcConn->client_id_len] = '\0';
}



/*
 * Sms informational functions
 */

int
SmsProtocolVersion (smsConn)

SmsConn smsConn;

{
    return (smsConn->proto_major_version);
}


int
SmsProtocolRevision (smsConn)

SmsConn smsConn;

{
    return (smsConn->proto_minor_version);
}


char *
SmsVendor (smsConn)

SmsConn smsConn;

{
    return (IceVendor (smsConn->iceConn));
}


char *
SmsRelease (smsConn)

SmsConn smsConn;

{
    return (IceRelease (smsConn->iceConn));
}


char *
SmsConnectionString (smsConn)

SmsConn smsConn;

{
    return (IceConnectionString (smsConn->iceConn));
}


unsigned long
SmsLastSequenceNumber (smsConn)

SmsConn smsConn;

{
    return (IceLastSequenceNumber (smsConn->iceConn));
}


void
SmsClientID (smsConn, clientIdLenRet, clientIdRet)

SmsConn smsConn;
int	*clientIdLenRet;
char	**clientIdRet;

{
    *clientIdLenRet = smsConn->client_id_len;
    *clientIdRet = (char *) malloc (smsConn->client_id_len + 1);
    bcopy (smsConn->client_id, *clientIdRet, smsConn->client_id_len);
    (*clientIdRet)[smsConn->client_id_len] = '\0';
}
