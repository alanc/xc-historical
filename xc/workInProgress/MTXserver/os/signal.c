/***********************************************************
Copyright 1992 by Data General Corporation, Westborough, Massachusetts,
and OMRON Corporation.
Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Data General, OMRON,
Digital or MIT not be used in advertising or publicity pertaining to 
distribution of the software without specific, written prior permission.  

DATA GENERAL, OMRON, AND DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO 
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS, IN NO EVENT SHALL DATA GENERAL, OMRON, OR
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

Copyright 1992, 1993 Data General Corporation;
Copyright 1992, 1993 OMRON Corporation  

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
neither the name OMRON or DATA GENERAL be used in advertising or publicity
pertaining to distribution of the software without specific, written prior
permission of the party whose name is to be used.  Neither OMRON or 
DATA GENERAL make any representation about the suitability of this software
for any purpose.  It is provided "as is" without express or implied warranty.  

OMRON AND DATA GENERAL EACH DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OMRON OR DATA GENERAL BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
OF THIS SOFTWARE.

******************************************************************/

#ifdef MTX /* XXX:SM - I don't think any of this should be used by STX. */

#include <signal.h>
#include "mtxlock.h"

extern void SignalServerReset();
extern void SignalServerTerminate();
extern void DITSetScreenSaver();


#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif


SIGVAL
SigPipeHandler()
{
#if defined(SYSV) || defined(SVR4)
    signal (SIGPIPE, SigPipeHandler);
#endif
}

SIGVAL
HandleSaverTime()
{
#if defined(SYSV) || defined(SVR4)
    signal (SIGALRM, HandleSaverTime);
#endif
    DITSetScreenSaver();
}

/***********************************************************************/
#ifndef CTHREADS

static sigset_t mtxSignalMask;
static sigset_t ditSignalMask;

void *
SignalHandlingThread()
{
    for (;;)
    {
        switch(sigwait(&mtxSignalMask))
        {
            case SIGINT:
            case SIGTERM:
                SignalServerTerminate();
                break;

            case SIGHUP:
                SignalServerReset();
                break;

            case SIGALRM:
		DITSetScreenSaver();
                break;

            case SIGPIPE:
		break;

            default:
                break;
        }
    }
}

Bool
InitializeSignalHandlers()
{
    X_THREAD_TYPE signal_thread;

    sigemptyset(&mtxSignalMask);
    sigaddset(&mtxSignalMask, SIGINT);
    sigaddset(&mtxSignalMask, SIGTERM);
    sigaddset(&mtxSignalMask, SIGHUP);
    sigaddset(&mtxSignalMask, SIGALRM);
    sigaddset(&mtxSignalMask, SIGPIPE);

    /* Start the signal handling thread */
    if (X_THREAD_CREATE(&signal_thread, X_CIT_ATTR_DEFAULT,
                         SignalHandlingThread, NULL))
    {
        ErrorF("SignalHandlingThread: thread create failed\n");
        return (FALSE);
    }
    X_THREAD_DETACH(&signal_thread);

    /* Block signals for all threads created after this one */

    sigprocmask(SIG_BLOCK, &mtxSignalMask, NULL);

    return (TRUE);
}

void
UnblockSigalrmSignal()
{
    sigemptyset(&ditSignalMask);
    sigaddset(&ditSignalMask, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &ditSignalMask, NULL);
    return;
}

#else

Bool
InitializeSignalHandlers()
{
    return (TRUE);
}

void
UnblockSigalrmSignal()
{
    return;
}

#endif /* CTHREADS */
#endif /* MTX */
