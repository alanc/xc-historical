/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**/

#include <stdio.h>
#include <X/Xos.h>
#include "Xlibint.h"
#include "Xresource.h"

char *XErrorList[] = {
	/* No error	*/	"",
	/* BadRequest	*/	"bad request code",
	/* BadValue	*/	"integer parameter out of range",
	/* BadWindow	*/	"parameter not a Window",
	/* BadPixmap	*/	"parameter not a Pixmap",
	/* BadAtom	*/	"parameter not an Atom",
	/* BadCursor	*/	"parameter not a Cursor",
	/* BadFont	*/	"parameter not a Font",
	/* BadMatch	*/	"parameter mismatch",
	/* BadDrawable	*/	"parameter not a Pixmap or Window",
	/* BadAccess	*/	"attempt to access private resource", 
	/* BadAlloc	*/	"insufficient resources",
    	/* BadColor   	*/  	"no such colormap",
    	/* BadGC   	*/  	"parameter not a GC",
	/* BadIDChoice  */	"invalid resource ID for this connection",
	/* BadName	*/	"font or color name does not exist",
	/* BadLength	*/	"request length incorrect; internal Xlib error",
	/* BadImplementation */	"server does not implement function",
};
int XErrorListSize = sizeof(XErrorList);


XGetErrorText(dpy, code, buffer, nbytes)
    register int code;
    register Display *dpy;
    char *buffer;
    int nbytes;
{

    char *defaultp = NULL;
    char buf[32];
    register _XExtension *ext;

    sprintf(buf, "%d\0", code);
    if (code <= (XErrorListSize/ sizeof (char *)) && code > 0) {
	defaultp =  XErrorList[code];
	XGetErrorDatabaseText(dpy, "XProtoError", buf, defaultp, buffer, nbytes);
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
    XrmAtom type_str;
    XrmValue result;
    static int initialized = False;
    char temp[BUFSIZ];

    if (initialized == False) {
	_XInitErrorHandling (&db);
	initialized = True;
    }
    sprintf(temp, "%s.%s", name, type);
    XrmGetResource(db, temp, "ErrorType.ErrorNumber", &type_str, &result);
    if (result.addr) {
	(void) strncpy (buffer, result.addr, nbytes);
	if (result.size < nbytes) buffer[result.size] = 0;
    } else (void) strncpy(buffer, defaultp, nbytes);
}

_XInitErrorHandling (db)
    XrmDatabase *db;
    {
    XrmDatabase errordb;
    XrmInitialize();
    errordb = XrmGetFileDatabase(ErrorDataBase);
    XrmMergeDatabases(errordb, db);
     }

