/*
 * $XConsortium: globals.c,v 1.1 89/06/15 11:46:50 jim Exp $
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

#include <sys/param.h>			/* for definition of NULL */


#ifdef SHARELIB
#include "Xresource.h"			/* for definitions of quarks */


/* from XlibInt.c */

extern int _XIOError();
extern int _XDefaultError();
int (*_XIOErrorFunction)() = _XIOError;
int (*_XErrorFunction)()   = _XDefaultError;


/* from XOpenDis.c */

int _Xdebug = 0;
long _XdebugFlag = 0;
void * _XdebugPtr = NULL;

Display *_XHeadOfDisplayList = NULL;
long _XHeadOfDisplayListFlag = 0;
void * _XHeadOfDisplayListPtr = NULL;


/* from Quarks.c */

XrmQuark  XrmQBoolean	= NULLQUARK;
long   XrmQBooleanFlag = 0;
void * XrmQBooleanPtr = NULL;
XrmQuark  XrmQColor	= NULLQUARK;
long   XrmQColorFlag = 0;
void * XrmQColorPtr = NULL;
XrmQuark  XrmQCursor	= NULLQUARK;
long   XrmQCursorFlag = 0;
void * XrmQCursorPtr = NULL;
XrmQuark  XrmQDims	= NULLQUARK;
long   XrmQDimsFlag = 0;
void * XrmQDimsPtr = NULL;
XrmQuark  XrmQDisplay	= NULLQUARK;
long   XrmQDisplayFlag = 0;
void * XrmQDisplayPtr = NULL;
XrmQuark  XrmQFile	= NULLQUARK;
long   XrmQFileFlag = 0;
void * XrmQFilePtr = NULL;
XrmQuark  XrmQFont	= NULLQUARK;
long   XrmQFontFlag = 0;
void * XrmQFontPtr = NULL;
XrmQuark  XrmQFontStruct= NULLQUARK;
long   XrmQ7sFontStructFlag = 0;
void * XrmQ7sFontStructPtr = NULL;
XrmQuark  XrmQGeometry	= NULLQUARK;
long   XrmQGeometryFlag = 0;
void * XrmQGeometryPtr = NULL;
XrmQuark  XrmQInt	= NULLQUARK;
long   XrmQIntFlag = 0;
void * XrmQIntPtr = NULL;
XrmQuark  XrmQPixel	= NULLQUARK;
long   XrmQPixelFlag = 0;
void * XrmQPixelPtr = NULL;
XrmQuark  XrmQPixmap	= NULLQUARK;
long   XrmQPixmapFlag = 0;
void * XrmQPixmapPtr = NULL;
XrmQuark  XrmQPointer	= NULLQUARK;
long   XrmQPointerFlag = 0;
void * XrmQPointerPtr = NULL;
XrmQuark  XrmQString	= NULLQUARK;
long   XrmQStringFlag = 0;
void * XrmQStringPtr = NULL;
XrmQuark  XrmQWindow	= NULLQUARK;
long   XrmQWindowFlag = 0;
void * XrmQWindowPtr = NULL;

/* "Enumeration" constants */

XrmQuark  XrmQEfalse	= NULLQUARK;
long   XrmQEfalseFlag = 0;
void * XrmQEfalsePtr = NULL;
XrmQuark  XrmQEno	= NULLQUARK;
long   XrmQEnoFlag = 0;
void * XrmQEnoPtr = NULL;
XrmQuark  XrmQEoff	= NULLQUARK;
long   XrmQEoffFlag = 0;
void * XrmQEoffPtr = NULL;
XrmQuark  XrmQEon	= NULLQUARK;
long   XrmQEonFlag = 0;
void * XrmQEonPtr = NULL;
XrmQuark  XrmQEtrue	= NULLQUARK;
long   XrmQEtrueFlag = 0;
void * XrmQEtruePtr = NULL;
XrmQuark  XrmQEyes	= NULLQUARK;
long   XrmQEyesFlag = 0;
void * XrmQEyesPtr = NULL;

#endif /* SHARELIB */


#ifdef STREAMSCONN
#include "Xstreams.h"

char _XsTypeOfStream[100] =
   {
   0
   };

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

