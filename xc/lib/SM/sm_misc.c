/* $XConsortium: sm_misc.c,v 1.4 93/12/07 11:05:32 mor Exp $ */
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

#include <X11/SM/SMlib.h>
#include "SMlibint.h"
#include <stdio.h>

/*
 * Free property
 */

void
SmFreeProperty (prop)

SmProp	*prop;

{
    if (prop)
    {
	int i;

	if (prop->name)
	    free (prop->name);
	if (prop->type)
	    free (prop->type);
	if (prop->vals)
	{
	    for (i = 0; i < prop->num_vals; i++)
		if (prop->vals[i].value)
		    free ((char *) prop->vals[i].value);
	    free ((char *) prop->vals);
	}

	free ((char *) prop);
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
    char *string = (char *) malloc (strlen (smcConn->vendor) + 1);

    strcpy (string, smcConn->vendor);

    return (string);
}


char *
SmcRelease (smcConn)

SmcConn smcConn;

{
    char *string = (char *) malloc (strlen (smcConn->release) + 1);

    strcpy (string, smcConn->release);

    return (string);
}


char *
SmcClientID (smcConn)

SmcConn smcConn;

{
    char *clientId = (char *) malloc (strlen (smcConn->client_id) + 1);

    strcpy (clientId, smcConn->client_id);

    return (clientId);
}


IceConn
SmcGetIceConnection (smcConn)

SmcConn smcConn;

{
    return (smcConn->iceConn);
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
    char *string = (char *) malloc (strlen (smsConn->vendor) + 1);

    strcpy (string, smsConn->vendor);

    return (string);
}


char *
SmsRelease (smsConn)

SmsConn smsConn;

{
    char *string = (char *) malloc (strlen (smsConn->release) + 1);

    strcpy (string, smsConn->release);

    return (string);
}


char *
SmsClientID (smsConn)

SmsConn smsConn;

{
    char *clientId = (char *) malloc (strlen (smsConn->client_id) + 1);

    strcpy (clientId, smsConn->client_id);

    return (clientId);
}


IceConn
SmsGetIceConnection (smsConn)

SmsConn smsConn;

{
    return (smsConn->iceConn);
}
