#include	"copyright.h"
/* @(#)FSErrHndlr.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

extern int  _FSDefaultError();
extern int  _FSDefaultIOError();

int         (*
	     FSSetErrorHandler(handler)) ()
    int         (*handler) ();
{
    int         (*oldhandler) () = _FSErrorFunction;

    if (handler != NULL) {
	_FSErrorFunction = handler;
    } else {
	_FSErrorFunction = _FSDefaultError;
    }
    return oldhandler;
}

extern int  _FSIOError();

int         (*
	     FSSetIOErrorHandler(handler)) ()
    int         (*handler) ();
{
    int         (*oldhandler) () = _FSIOErrorFunction;

    if (handler != NULL) {
	_FSIOErrorFunction = handler;
    } else {
	_FSIOErrorFunction = _FSDefaultIOError;
    }
    return oldhandler;
}
