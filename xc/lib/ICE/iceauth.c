/* $XConsortium: iceauth.c,v 1.8 93/11/30 15:29:00 mor Exp $ */
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

static int binaryEqual ();

static int was_called_state;



IcePoAuthStatus
_IcePoMagicCookie1Proc (authStatePtr, address, cleanUp, swap,
    authDataLen, authData, replyDataLenRet, replyDataRet, errorStringRet)

IcePointer	*authStatePtr;
char		*address;
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
	 * ICE-MAGIC-COOKIE-1 that matches address.
	 */

	unsigned short  length;
	char		*data;

	IceGetPoAuthData ("ICE", address, "ICE-MAGIC-COOKIE-1",
	    &length, &data);

	if (!data)
	{
	    char *tempstr =
		"Could not find correct ICE-MAGIC-COOKIE-1 authentication";

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
	 * We should never get here for ICE-MAGIC-COOKIE-1 since it is
	 * a single pass authentication method.
	 */

	char *tempstr = "ICE-MAGIC-COOKIE-1 authentication internal error";

	*errorStringRet = (char *) malloc (strlen (tempstr) + 1);
	if (*errorStringRet)
	    strcpy (*errorStringRet, tempstr);

	return (IcePoAuthFailed);
    }
}



IcePaAuthStatus
_IcePaMagicCookie1Proc (authStatePtr, address, swap,
    replyDataLen, replyData, authDataLenRet, authDataRet, errorStringRet)

IcePointer	*authStatePtr;
char		*address;
Bool		swap;
int     	replyDataLen;
IcePointer	replyData;
int 		*authDataLenRet;
IcePointer	*authDataRet;
char    	**errorStringRet;

{
    IceAuthDataEntry	*entry;

    *errorStringRet = NULL;
    *authDataLenRet = 0;
    *authDataRet = NULL;

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
	 * ICE-MAGIC-COOKIE-1 that matches address.
	 */

	unsigned short  length;
	char		*data;

	IceGetPaAuthData ("ICE", address, "ICE-MAGIC-COOKIE-1",
	    &length, &data);

	if (data)
	{
	    IcePaAuthStatus stat;

	    if (replyDataLen == length &&
	        binaryEqual (replyData, data, replyDataLen))
	    {
		stat = IcePaAuthAccepted;
	    }
	    else
	    {
		char *tempstr = "ICE-MAGIC-COOKIE-1 authentication rejected";

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
		"ICE-MAGIC-COOKIE-1 authentication internal error";

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
