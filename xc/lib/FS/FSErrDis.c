#include	"copyright.h"
/* @(#)FSErrDis.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include <stdio.h>
#include <X11/Xos.h>
#include "FSlibint.h"

char       *FSErrorList[] = {
     /* FSBadRequest	 */ "BadRequest, invalid request code or no such operation",
     /* FSBadFormat	 */ "BadFormat, bad font format mask",
     /* FSBadFont	 */ "BadFont, invalid Font parameter",
     /* FSBadRange	 */ "BadRange, invalid character range attributes",
     /* FSBadEventMask	 */ "BadEventMask, illegal event mask",
     /* FSBadAccessContext */ "BadAccessContext, insufficient permissions for operation",
     /* FSBadIDChoice  */ "BadIDChoice, invalid resource ID chosen for this connection",
     /* FSBadName	 */ "BadName, named font does not exist",
     /* FSBadResolution	 */ "BadResolution, improperly formatted resolution",
     /* FSBadAlloc	 */ "BadAlloc, insufficient resources for operation",
     /* FSBadLength	 */ "BadLength, request too large or internal FSlib length error",
     /* FSBadImplementation */ "BadImplementation, request unsupported",
};
int         FSErrorListSize = sizeof(FSErrorList);


FSGetErrorText(svr, code, buffer, nbytes)
    register int code;
    register FSServer *svr;
    char       *buffer;
    int         nbytes;
{

    char       *defaultp = NULL;
    char        buf[32];
    register _FSExtension *ext;

    if (nbytes == 0)
	return;
    sprintf(buf, "%d", code);
    if (code <= (FSErrorListSize / sizeof(char *)) && code > 0) {
	defaultp = FSErrorList[code];
	FSGetErrorDatabaseText(svr, "FSProtoError", buf, defaultp, buffer, nbytes);
    }
    ext = svr->ext_procs;
    while (ext) {		/* call out to any extensions interested */
	if (ext->error_string != NULL)
	    (*ext->error_string) (svr, code, &ext->codes, buffer, nbytes);
	ext = ext->next;
    }
    return;
}

/* ARGSUSED */
FSGetErrorDatabaseText(svr, name, type, defaultp, buffer, nbytes)
    register char *name,
               *type;
    char       *defaultp;
    FSServer     *svr;
    char       *buffer;
    int         nbytes;
{
    if (nbytes == 0)
	return;
    (void) strncpy(buffer, (char *) defaultp, nbytes);
    if ((strlen(defaultp) + 1) > nbytes)
	buffer[nbytes - 1] = '\0';
}
