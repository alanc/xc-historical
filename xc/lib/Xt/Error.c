#ifndef lint
static char rcsid[] = "$xHeader: Error.c,v 1.4 88/08/26 16:45:22 asente Exp $";
/* $oHeader: Error.c,v 1.4 88/08/26 16:45:22 asente Exp $ */
#endif lint

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <X11/Xlib.h>
#include <stdio.h>
#include "IntrinsicI.h"

XrmDatabase *XtGetErrorDatabase()
{
    return XtAppGetErrorDatabase(_XtDefaultAppContext());
}

XrmDatabase *XtAppGetErrorDatabase(app)
	XtAppContext app;
{
	return &app->errorDB;
}

void XtGetErrorDatabaseText(name,type,class,defaultp, buffer, nbytes)
    register char *name, *type,*class;
    char *defaultp;
    char *buffer;
    int nbytes;
{
    XtAppGetErrorDatabaseText(_XtDefaultAppContext(),
	    name,type,class,defaultp, buffer, nbytes, NULL);
}

void XtAppGetErrorDatabaseText(app, name,type,class,defaultp,
	buffer, nbytes, db)
    XtAppContext app;
    register char *name, *type,*class;
    char *defaultp;
    char *buffer;
    int nbytes;
    XrmDatabase db;
{
    String type_str;
    XrmValue result;
    char temp[BUFSIZ];

    if (app->error_inited == False) {
        _XtInitErrorHandling (&app->errorDB);
        app->error_inited = True;
    }
    (void) sprintf(temp, "%s.%s", name, type);
    if (db == NULL) {
	(void) XrmGetResource(app->errorDB, temp, class, &type_str, &result);
    } else (void) XrmGetResource(db, temp, class, &type_str, &result);
    if (result.addr) {
        (void) strncpy (buffer, result.addr, nbytes);
        if (result.size < nbytes) buffer[result.size] = 0;
    } else (void) strncpy(buffer, defaultp, nbytes);
}

_XtInitErrorHandling (db)
    XrmDatabase *db;
{
    XrmDatabase errordb;

    errordb = XrmGetFileDatabase(ERRORDB);
    XrmMergeDatabases(errordb, db);
}

static void _XtDefaultErrorMsg (name,type,class,defaultp,params,num_params)
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
    char buffer[1000],message[1000];
    XtGetErrorDatabaseText(name,type,class,defaultp, buffer, 1000);
/*need better solution here, perhaps use lower level printf primitives? */
    if (num_params == NULL) XtError(buffer);
    else {
        (void) sprintf(message, buffer, params[0], params[1], params[2],
		params[3], params[4], params[5], params[6], params[7],
		params[8], params[9]);
	XtError(message);
    }
}

static void _XtDefaultWarningMsg (name,type,class,defaultp,params,num_params)
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{

    char buffer[1000],message[1000];
    XtGetErrorDatabaseText(name,type,class,defaultp, buffer, 1000);
/*need better solution here*/
    if (num_params == NULL) XtWarning(buffer);
    else {
        (void) sprintf(message, buffer, params[0], params[1], params[2],
		params[3], params[4], params[5], params[6], params[7],
		params[8], params[9]);
	XtWarning(message); 
   }
}

void XtErrorMsg(name,type,class,defaultp,params,num_params)
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
    XtAppErrorMsg(_XtDefaultAppContext(),name,type,class,
	    defaultp,params,num_params);
}

void XtAppErrorMsg(app, name,type,class,defaultp,params,num_params)
    XtAppContext app;
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
    (*app->errorMsgHandler)(name,type,class,defaultp,params,num_params);
}

void XtWarningMsg(name,type,class,defaultp,params,num_params)
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
    XtAppWarningMsg(_XtDefaultAppContext(),name,type,class,
	    defaultp,params,num_params);
}

void XtAppWarningMsg(app,name,type,class,defaultp,params,num_params)
    XtAppContext app;
    String name,type,class,defaultp;
    String* params;
    Cardinal* num_params;
{
    (*app->warningMsgHandler)(name,type,class,defaultp,params,num_params);
}

void XtSetErrorMsgHandler(handler)
    XtErrorMsgHandler handler;
{
    XtAppSetErrorMsgHandler(_XtDefaultAppContext(), handler);
}

void XtAppSetErrorMsgHandler(app,handler)
    XtAppContext app;
    XtErrorMsgHandler handler;
{
    if (handler != NULL) app->errorMsgHandler = handler;
    else app->errorMsgHandler  = _XtDefaultErrorMsg;
}

void XtSetWarningMsgHandler(handler)
    XtErrorMsgHandler handler;
{
    XtAppSetWarningMsgHandler(_XtDefaultAppContext(),handler);
}

void XtAppSetWarningMsgHandler(app,handler)
    XtAppContext app;
    XtErrorMsgHandler handler;
{
    if (handler != NULL) app->warningMsgHandler  = handler;
    else app->warningMsgHandler = _XtDefaultWarningMsg;
}

static void _XtDefaultError(message)
    String message;
{
    extern void exit();

    (void)fprintf(stderr, "X Toolkit Error: %s\n", message);
    exit(1);

}

static void _XtDefaultWarning(message)
    String message;
{
       (void)fprintf(stderr, "X Toolkit Warning: %s\n", message); 
    return;
}

void XtError(message)
    String message;
{
    XtAppError(_XtDefaultAppContext(),message);
}

void XtAppError(app,message)
    XtAppContext app;
    String message;
{
    (*app->errorHandler)(message);
}

void XtWarning(message)
    String message;
{
    XtAppWarning(_XtDefaultAppContext(),message);
}

void XtAppWarning(app,message)
    XtAppContext app;
    String message;
{
    (*app->warningHandler)(message);
}

void XtSetErrorHandler(handler)
    XtErrorHandler handler;
{
    XtAppSetErrorHandler(_XtDefaultAppContext(),handler);
}

void XtAppSetErrorHandler(app,handler)
    XtAppContext app;
    XtErrorHandler handler;
{
    if (handler != NULL) app->errorHandler = handler;
    else app->errorHandler  = _XtDefaultError;
}

void XtSetWarningHandler(handler)
    XtErrorHandler handler;
{
    XtAppSetWarningHandler(_XtDefaultAppContext(),handler);
}

void XtAppSetWarningHandler(app,handler)
    XtAppContext app;
    XtErrorHandler handler;
{
    if (handler != NULL) app->warningHandler  = handler;
    else app->warningHandler = _XtDefaultWarning;
}

void _SetDefaultErrorHandlers(errMsg, warnMsg, err, warn)
	XtErrorMsgHandler *errMsg, *warnMsg;
	XtErrorHandler *err, *warn;
{
	*errMsg = _XtDefaultErrorMsg;
	*warnMsg = _XtDefaultWarningMsg;
	*err = _XtDefaultError;
	*warn = _XtDefaultWarning;
}
