/* $XConsortium: iceauth.c,v 1.16 94/03/18 15:59:16 mor Exp $ */
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
#include "ICElibint.h"
#include <X11/ICE/ICEutil.h>

#ifdef X_NOT_STDC_ENV
#define Time_t long
extern Time_t time ();
#else
#include <time.h>
#define Time_t time_t
#endif

static int binaryEqual ();

static int was_called_state;

/*
 * MIT-MAGIC-COOKIE-1 is a sample authentication method implemented by
 * the SI.  It is not part of standard ICElib.
 */


char *
IceGenerateMagicCookie (len)

int len;

{
    char    *auth;
    long    ldata[2];
    int	    seed;
    int	    value;
    int	    i;
    
    if ((auth = (char *) malloc (len + 1)) == NULL)
	return (NULL);

#ifdef ITIMER_REAL
    {
	struct timeval  now;
	GETTIMEOFDAY (&now);
	ldata[0] = now.tv_sec;
	ldata[1] = now.tv_usec;
    }
#else
    {
	long    time ();

	ldata[0] = time ((long *) 0);
	ldata[1] = getpid ();
    }
#endif
    seed = (ldata[0]) + (ldata[1] << 16);
    srand (seed);
    for (i = 0; i < len; i++)
    {
	value = rand ();
	auth[i] = value & 0xff;
    }
    auth[len] = '\0';

    return (auth);
}



IcePoAuthStatus
_IcePoMagicCookie1Proc (iceConn, authStatePtr, cleanUp, swap,
    authDataLen, authData, replyDataLenRet, replyDataRet, errorStringRet)

IceConn		iceConn;
IcePointer	*authStatePtr;
Bool 		cleanUp;
Bool		swap;
int     	authDataLen;
IcePointer	authData;
int 		*replyDataLenRet;
IcePointer	*replyDataRet;
char    	**errorStringRet;

{
    if (cleanUp)
    {
	/*
	 * We didn't allocate any state.  We're done.
	 */

	return (IcePoAuthDoneCleanup);
    }

    *errorStringRet = NULL;

    if (*authStatePtr == NULL)
    {
	/*
	 * This is the first time we're being called.  Search the
	 * authentication data for the first occurence of
	 * MIT-MAGIC-COOKIE-1 that matches iceConn->connection_string.
	 */

	unsigned short  length;
	char		*data;

	_IceGetPoAuthData ("ICE", iceConn->connection_string,
	    "MIT-MAGIC-COOKIE-1", &length, &data);

	if (!data)
	{
	    char *tempstr =
		"Could not find correct MIT-MAGIC-COOKIE-1 authentication";

	    *errorStringRet = (char *) malloc (strlen (tempstr) + 1);
	    if (*errorStringRet)
		strcpy (*errorStringRet, tempstr);

	    return (IcePoAuthFailed);
	}
	else
	{
	    *authStatePtr = (IcePointer) &was_called_state;

	    *replyDataLenRet = length;
	    *replyDataRet = data;

	    return (IcePoAuthHaveReply);
	}
    }
    else
    {
	/*
	 * We should never get here for MIT-MAGIC-COOKIE-1 since it is
	 * a single pass authentication method.
	 */

	char *tempstr = "MIT-MAGIC-COOKIE-1 authentication internal error";

	*errorStringRet = (char *) malloc (strlen (tempstr) + 1);
	if (*errorStringRet)
	    strcpy (*errorStringRet, tempstr);

	return (IcePoAuthFailed);
    }
}



IcePaAuthStatus
_IcePaMagicCookie1Proc (iceConn, authStatePtr, swap,
    authDataLen, authData, replyDataLenRet, replyDataRet, errorStringRet)

IceConn		iceConn;
IcePointer	*authStatePtr;
Bool		swap;
int     	authDataLen;
IcePointer	authData;
int 		*replyDataLenRet;
IcePointer	*replyDataRet;
char    	**errorStringRet;

{
    IceAuthDataEntry	*entry;

    *errorStringRet = NULL;
    *replyDataLenRet = 0;
    *replyDataRet = NULL;

    if (*authStatePtr == NULL)
    {
	/*
	 * This is the first time we're being called.  We don't have
	 * any data to pass to the other client.
	 */

	*authStatePtr = (IcePointer) &was_called_state;

	return (IcePaAuthContinue);
    }
    else
    {
	/*
	 * Search the authentication data for the first occurence of
	 * MIT-MAGIC-COOKIE-1 that matches iceConn->connection_string.
	 */

	unsigned short  length;
	char		*data;

	_IceGetPaAuthData ("ICE", iceConn->connection_string,
	    "MIT-MAGIC-COOKIE-1", &length, &data);

	if (data)
	{
	    IcePaAuthStatus stat;

	    if (authDataLen == length &&
	        binaryEqual (authData, data, authDataLen))
	    {
		stat = IcePaAuthAccepted;
	    }
	    else
	    {
		char *tempstr = "MIT-MAGIC-COOKIE-1 authentication rejected";

		*errorStringRet = (char *) malloc (strlen (tempstr) + 1);
		if (*errorStringRet)
		    strcpy (*errorStringRet, tempstr);

		stat = IcePaAuthRejected;
	    }

	    free (data);
	    return (stat);
	}
	else
	{
	    /*
	     * We should never get here because in the ConnectionReply
	     * we should have passed all the valid methods.  So we should
	     * always find a valid entry.
	     */

	    char *tempstr =
		"MIT-MAGIC-COOKIE-1 authentication internal error";

	    *errorStringRet = (char *) malloc (strlen (tempstr) + 1);
	    if (*errorStringRet)
		strcpy (*errorStringRet, tempstr);

	    return (IcePaAuthFailed);
	}
    }
}



/*
 * local routines
 */

static int
binaryEqual (a, b, len)

register char		*a, *b;
register unsigned	len;

{
    while (len--)
	if (*a++ != *b++)
	    return 0;
    return 1;
}
