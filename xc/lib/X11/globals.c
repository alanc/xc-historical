/*
 * $XConsortium: globals.c,v 1.2 89/06/15 15:59:07 jim Exp $
 */

/*
 * Copyright 1988, 1989 AT&T, Inc.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of AT&T not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  AT&T makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * AT&T DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL AT&T
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
*/

/*'copyright	"%c%"'*/


/*
 * Global data
 *
 * This file should contain only those objects which must be predefined.
 * If possible, it is useful to have the global data default to a null value.
 * Some shared library implementations are *much* happier if there isn't any
 * global initialized data.
 */

#ifdef NULL_NOT_ZERO			/* then need to initialize */
/*
 * need to explicitly initialize variables to null values
 */
#ifdef ATTSHAREDLIB			/* then need extra variables */
#if defined(__STDC__) && !defined(UNIXCPP)  /* then ANSI C concatenation */
#define INIT(t,var,val) \
  t var = val; long var##Flag = 0; void * var##Ptr = NULL
#else					/* else pcc concatenation */
#define INIT(t,var,val) \
  t var = val; long var/**/Flag = 0; void * var/**/Ptr = NULL
#endif					/* concat ANSI C vs. pcc */
#else					/* else no extra variables */
/*
 * zeroed variables are NULL
 */
#define INIT(t,var,val) t var = val
#endif					/* NULL_NOT_ZERO */
#include <sys/param.h>			/* for definition of NULL */

#else					/* else zero value same as NULL */
#define INIT(t,var,val) t var	/* let it default to zero */
#endif

#include "Xlib.h"			/* for definition of Display */


/*
 * Error handlers; used to be in XlibInt.c
 */
typedef int (*funcptr)();
INIT (funcptr, _XErrorFunction, NULL);
INIT (funcptr, _XIOErrorFunction, NULL);


/*
 * Debugging information and display list; used to be in XOpenDis.c
 */

INIT (int, _Xdebug, 0);
INIT (Display *, _XHeadOfDisplayList, NULL);




#ifdef STREAMSCONN
/*
 * descriptor block for streams connections
 */
#include "Xstreams.h"

char _XsTypeOfStream[100] = { 0 };

extern int SetupLocalStream();
extern int ConnectLocalClient();
extern int CallLocalServer();
extern int ReadLocalStream();
extern int ErrorCall();
extern int WriteLocalStream();
extern int CloseLocalStream(); 
extern int SetupTliStream();
extern int ConnectTliClient();
extern int CallTliServer(); 
extern int ReadTliStream(); 
extern int WriteTliStream();
extern int CloseTliStream();
extern int ErrorCall(); 

Xstream _XsStream[] = {
    { 
	SetupLocalStream,
	ConnectLocalClient,
	CallLocalServer,
	ReadLocalStream,
	ErrorCall,
	WriteLocalStream,
	CloseLocalStream,
	NULL
    },
    { 
	SetupTliStream,
	ConnectTliClient,
	CallTliServer, 
	ReadTliStream, 
	ErrorCall, 
	WriteTliStream,
	CloseTliStream,
	NULL
    },
    { 
	SetupTliStream,
	ConnectTliClient,
	CallTliServer, 
	ReadTliStream, 
	ErrorCall, 
	WriteTliStream,
	CloseTliStream,
	NULL
    },
    { 
	NULL,
	NULL, 
	NULL, 
	NULL, 
	NULL,
	NULL,
	NULL,
	NULL
    },
    { 
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, 
	NULL, 
	NULL,
	NULL
    },
    { 
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, 
	NULL, 
	NULL
    }
};
#endif /* STREAMSCONN */

#ifdef XTEST1
/*
 * Stuff for input synthesis extension:
 */
/*
 * Holds the two event type codes for this extension.  The event type codes
 * for this extension may vary depending on how many extensions are installed
 * already, so the initial values given below will be added to the base event
 * code that is aquired when this extension is installed.
 *
 * These two variables must be available to programs that use this extension.
 */
int			XTestInputActionType = 0;
int			XTestFakeAckType   = 1;
#endif

/*
 * NOTE: any additional external definition NEED
 * to be inserted BELOW this point!!!
 */

/*
 * NOTE: any additional external definition NEED
 * to be inserted ABOVE this point!!!
 */

