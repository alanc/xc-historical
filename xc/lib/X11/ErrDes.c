/*
 * $XConsortium: XErrDes.c,v 11.35 89/05/08 17:19:01 jim Exp $
 */

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

#include <stdio.h>
#include <X11/Xos.h>
#include "Xlibint.h"
#include "Xresource.h"

#ifndef ERRORDB
#define ERRORDB "/usr/lib/X11/XErrorDB"
#endif

/*
 * descriptions of errors in Section 4 of Protocol doc (pp. 350-351); more
 * verbose descriptions are given in the error database
 */
static char *_XErrorList[] = {
    /* No error	*/		"no error",
    /* BadRequest */		"BadRequest",
    /* BadValue	*/		"BadValue",
    /* BadWindow */		"BadWindow",
    /* BadPixmap */		"BadPixmap",
    /* BadAtom */		"BadAtom",
    /* BadCursor */		"BadCursor",
    /* BadFont */		"BadFont",
    /* BadMatch	*/		"BadMatch",
    /* BadDrawable */		"BadDrawable",
    /* BadAccess */		"BadAccess",
    /* BadAlloc	*/		"BadAlloc",
    /* BadColor */  		"BadColor",
    /* BadGC */  		"BadGC",
    /* BadIDChoice */		"BadIDChoice",
    /* BadName */		"BadName",
    /* BadLength */		"BadLength",
    /* BadImplementation */	"BadImplementation",
};
static int _XErrorListSize = sizeof(_XErrorList);


XGetErrorText(dpy, code, buffer, nbytes)
    register int code;
    register Display *dpy;
    char *buffer;
    int nbytes;
{
    char buf[32];
    register _XExtension *ext;

    if (nbytes == 0) return;
    sprintf(buf, "%d", code);
    if (code <= (_XErrorListSize/ sizeof (char *)) && code > 0) {
	XGetErrorDatabaseText(dpy, "XProtoError", buf, _XErrorList[code],
			      buffer, nbytes);
	}
    ext = dpy->ext_procs;
    while (ext) {		/* call out to any extensions interested */
 	if (ext->error_string != NULL) 
 	    (*ext->error_string)(dpy, code, &ext->codes, buffer, nbytes);
 	ext = ext->next;
    }    
    return;
}

XGetErrorDatabaseText(dpy, name, type, defaultp, buffer, nbytes)
    register char *name, *type;
    char *defaultp;
    Display *dpy;
    char *buffer;
    int nbytes;
{

    static XrmDatabase db;
    XrmString type_str;
    XrmValue result;
    static int initialized = False;
    char temp[BUFSIZ];

    if (nbytes == 0) return;
    if (initialized == False) {
	_XInitErrorHandling (&db);
	initialized = True;
    }
    sprintf(temp, "%s.%s", name, type);
    XrmGetResource(db, temp, "ErrorType.ErrorNumber", &type_str, &result);
    if (!result.addr) {
	result.addr = (caddr_t) defaultp;
	result.size = strlen(defaultp);
    }
    (void) strncpy (buffer, (char *) result.addr, nbytes);
    if (result.size >= nbytes) buffer[nbytes-1] = '\0';
}

_XInitErrorHandling (db)
    XrmDatabase *db;
    {
    XrmDatabase errordb;
    XrmInitialize();
    errordb = XrmGetFileDatabase(ERRORDB);
    XrmMergeDatabases(errordb, db);
     }

