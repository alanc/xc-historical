/* $XConsortium: iceauth.c,v 1.1 93/08/17 18:58:57 mor Exp $ */
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


/*
 * For now, I haven't implemented any real authentication for ICE.
 * Nevertheless, these routines are here for testing purposes.
 */

typedef struct {
    int phase;
} Auth1State;

typedef struct {
    int phase;
} Auth2State;


IceOCLauthStatus
_IceOCLauth1proc (authStatePtr, cleanUp,
    authDataLen, authData, replyDataLenRet, replyDataRet, errorStringRet)

IcePointer	*authStatePtr;
Bool 		cleanUp;
int     	authDataLen;
IcePointer	authData;
int 		*replyDataLenRet;
IcePointer	*replyDataRet;
char    	**errorStringRet;

{
    Auth1State *state;

    if (cleanUp && *authStatePtr != NULL)
    {
	/*
	 * Free stuff within state, if any.
	 */
	
	state = (Auth1State *) *authStatePtr;


	/*
	 * Now free state pointer.
	 */

	free ((char *) state);
	return (IceOCLauthDoneCleanup);
    }

    *errorStringRet = NULL;

    if (*authStatePtr == NULL)
    {
	/*
	 * This is the first time we're being called.
	 * Allocate state, initialize phase to 1.
	 */

	state = (Auth1State *) malloc (sizeof (Auth1State));
	*authStatePtr = (IcePointer) state;
	state->phase = 1;
    }
    else
    {
	state = (Auth1State *) *authStatePtr;
	state->phase++;
    }

    *replyDataLenRet = 0;
    *replyDataRet = NULL;

    return (IceOCLauthHaveReply);
}



IceACLauthStatus
_IceACLauth1proc (authStatePtr, replyDataLen, replyData,
    authDataLenRet, authDataRet, errorStringRet)

IcePointer	*authStatePtr;
int     	replyDataLen;
IcePointer	replyData;
int 		*authDataLenRet;
IcePointer	*authDataRet;
char    	**errorStringRet;

{
    Auth1State *state;

    *errorStringRet = NULL;

    if (*authStatePtr == NULL)
    {
	/*
	 * This is the first time we're being called.
	 * Allocate state, initialize phase to 1.
	 */

	state = (Auth1State *) malloc (sizeof (Auth1State));
	*authStatePtr = (IcePointer) state;
	state->phase = 1;
    }
    else
    {
	state = (Auth1State *) *authStatePtr;
	state->phase++;
    }

    if (state->phase == 3)
    {
	/*
	 * Free stuff within state, if any.
	 */
	

	/*
	 * Now free state pointer.
	 */

	free ((char *) state);

	return (IceACLauthAccepted);
    }
    else
    {
	*authDataLenRet = 0;
	*authDataRet = NULL;

	return (IceACLauthContinue);
    }
}


IceOCLauthStatus
_IceOCLauth2proc (authStatePtr, cleanUp,
    authDataLen, authData, replyDataLenRet, replyDataRet, errorStringRet)

IcePointer	*authStatePtr;
Bool 		cleanUp;
int     	authDataLen;
IcePointer	authData;
int 		*replyDataLenRet;
IcePointer	*replyDataRet;
char    	**errorStringRet;

{
    Auth2State *state;

    if (cleanUp && *authStatePtr != NULL)
    {
	/*
	 * Free stuff within state, if any.
	 */
	
	state = (Auth2State *) *authStatePtr;


	/*
	 * Now free state pointer.
	 */

	free ((char *) state);
	return (IceOCLauthDoneCleanup);
    }

    *errorStringRet = NULL;

    if (*authStatePtr == NULL)
    {
	/*
	 * This is the first time we're being called.
	 * Allocate state, initialize phase to 1.
	 */

	state = (Auth2State *) malloc (sizeof (Auth2State));
	*authStatePtr = (IcePointer) state;
	state->phase = 1;
    }
    else
    {
	state = (Auth2State *) *authStatePtr;
	state->phase++;
    }

    *replyDataLenRet = 0;
    *replyDataRet = NULL;

    return (IceOCLauthHaveReply);
}



IceACLauthStatus
_IceACLauth2proc (authStatePtr, replyDataLen, replyData,
    authDataLenRet, authDataRet, errorStringRet)

IcePointer 	*authStatePtr;
int     	replyDataLen;
IcePointer	replyData;
int 		*authDataLenRet;
IcePointer	*authDataRet;
char    	**errorStringRet;

{
    Auth2State *state;

    *errorStringRet = NULL;

    if (*authStatePtr == NULL)
    {
	/*
	 * This is the first time we're being called.
	 * Allocate state, initialize phase to 1.
	 */

	state = (Auth2State *) malloc (sizeof (Auth2State));
	*authStatePtr = (IcePointer) state;
	state->phase = 1;
    }
    else
    {
	state = (Auth2State *) *authStatePtr;
	state->phase++;
    }

    if (state->phase == 3)
    {
	/*
	 * Free stuff within state, if any.
	 */
	

	/*
	 * Now free state pointer.
	 */

	free ((char *) state);

	return (IceACLauthAccepted);
    }
    else
    {
	*authDataLenRet = 0;
	*authDataRet = NULL;

	return (IceACLauthContinue);
    }
}