/* $XConsortium: iceauth.c,v 1.5 93/11/18 11:45:34 mor Exp $ */
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


IcePoAuthStatus
_IcePoMagicCookie1Proc (authStatePtr, connectionString, cleanUp, swap,
    authDataLen, authData, replyDataLenRet, replyDataRet, errorStringRet)

IcePointer	*authStatePtr;
char		*connectionString;
Bool 		cleanUp;
Bool		swap;
int     	authDataLen;
IcePointer	authData;
int 		*replyDataLenRet;
IcePointer	*replyDataRet;
char    	**errorStringRet;

{
    if (cleanUp && *authStatePtr != NULL)
    {
	/*
	 * Free state.  We're done.
	 */

	IceDisposeAuthFileEntry ((IceAuthFileEntry *) *authStatePtr);
	return (IcePoAuthDoneCleanup);
    }

    *errorStringRet = NULL;

    if (*authStatePtr == NULL)
    {
	/*
	 * This is the first time we're being called.  Check the
	 * .ICEauthority file for the first occurence of
	 * ICE-MAGIC-COOKIE-1 that matches connectionString.
	 */

	IceAuthFileEntry *entry = IceGetAuthFileEntry (3, "ICE",
            strlen (connectionString), connectionString,
	    18, "ICE-MAGIC-COOKIE-1");

	if (!entry)
	{
	    char *tempstr = "Could not find correct ICE-MAGIC-COOKIE-1 entry in .ICEauthority file";

	    *errorStringRet = (char *) malloc (strlen (tempstr) + 1);
	    strcpy (*errorStringRet, tempstr);

	    return (IcePoAuthFailed);
	}
	else
	{
	    *authStatePtr = (IcePointer) entry;

	    *replyDataLenRet = entry->auth_data_length;
	    *replyDataRet = entry->auth_data;

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
	strcpy (*errorStringRet, tempstr);
	IceDisposeAuthFileEntry ((IceAuthFileEntry *) *authStatePtr);

	return (IcePoAuthFailed);
    }
}



IcePaAuthStatus
_IcePaMagicCookie1Proc (authStatePtr, connectionString, swap,
    replyDataLen, replyData, authDataLenRet, authDataRet, errorStringRet)

IcePointer	*authStatePtr;
char		*connectionString;
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
	 * This is the first time we're being called.  Check the
	 * authentication data set by IceSetAuthenticationData()
	 * for the first occurence of ICE-MAGIC-COOKIE-1 that
	 * matches connectionString.  The reason we don't check the
	 * .ICEauthority file is because this would be a security
	 * hole.  The client could just write an entry into the
	 * .ICEauthority file knowing the accepting side would use
	 * it in authentication.
	 */

	entry = _IceGetAuthDataEntry (3, "ICE",
            strlen (connectionString), connectionString,
	    18, "ICE-MAGIC-COOKIE-1");

	if (!entry)
	{
	    /*
	     * We should never get here because in the ConnectionReply
	     * we should have passed all the valid methods.  So we should
	     * always find a valid entry.
	     */

	    char *tempstr =
		"ICE-MAGIC-COOKIE-1 authentication internal error";

	    *errorStringRet = (char *) malloc (strlen (tempstr) + 1);
	    strcpy (*errorStringRet, tempstr);

	    return (IcePaAuthFailed);
	}
	else
	{
	    *authStatePtr = (IcePointer) entry;

	    return (IcePaAuthContinue);
	}
    }
    else
    {
	entry = (IceAuthFileEntry *) *authStatePtr;

	if (replyDataLen == entry->auth_data_length &&
	    binaryEqual (replyData, entry->auth_data, replyDataLen))
	{
	    return (IcePaAuthAccepted);
	}
	else
	{
	    char *tempstr = "ICE-MAGIC-COOKIE-1 authentication rejected";

	    *errorStringRet = (char *) malloc (strlen (tempstr) + 1);
	    strcpy (*errorStringRet, tempstr);

	    return (IcePaAuthRejected);
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
