/*
 * $XConsortium$
 *
 * Copyright 1994 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * greet.h - interface to xdm's dynamically-loadable modular greeter
 */

#include <X11/Xlib.h>

/*
 * Return values for GreetUser();
 * Values must be explictly defined because the greet library
 * may come from a different vendor.
 * Negative values indicate an error.
 */
typedef enum {
    Greet_Session_Over = 0,	/* session managed and over */
    Greet_Success = 1,		/* greet succeeded, session not managed */
    Greet_Failure = -1		/* greet failed */
} greet_user_rtn;

/*
 * GreetUser can either handle the user's session or allow xdm to do it.
 * The return or exit status of GreetUser indicates to xdm whether it
 * should start a session.
 * 
 * GreetUser is passed the xdm struct display pointer, a pointer to a
 * Display, and pointers to greet and verify structs.  If it expectes xdm
 * to run the session, it fills in the Display pointer and the fields
 * of the greet and verify structs.
 * 
 * The verify struct includes the uid, gid, arguments to run the session,
 * environment for the session, and environment for startup/reset.
 * 
 * The greet struct includes the user's name and password but these are
 * really only needed if xdm is compiled with a user-based authorization
 * option such as SECURE_RPC or K5AUTH.
 */

extern greet_user_rtn GreetUser(
#if NeedFunctionPrototypes
    struct display *d,
    Display **dpy,
    struct verify_info *verify,
    struct greet_info *greet
#endif
);

typedef greet_user_rtn (*GreetUserProc)(
#if NeedFunctionPrototypes
    struct display *,
    Display **,
    struct verify_info *,
    struct greet_info *
#endif
);

/* The greeter uses these xdm symbols.
   Should they be renamed to have a consistent prefix?
   Should we try to reduce their number?

PingServer
SessionPingFailed
Debug
RegisterCloseOnFork
SecureDisplay
UnsecureDisplay
ClearCloseOnFork
SetupDisplay
LogError
SessionExit
DeleteXloginResources
source
defaultEnv
setEnv
parseArgs
printEnv
systemEnv
LogOutOfMem
*/
