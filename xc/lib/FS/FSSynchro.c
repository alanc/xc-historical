#include	"copyright.h"
/* @(#)FSSynchro.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

int
_FSSyncFunction(svr)
    FSServer     *svr;
{
    FSSync(svr, 0);
}

int
(* FSSynchronize(svr, onoff)) ()
    FSServer     *svr;
    int         onoff;
{
    int         (*temp) ();

    temp = svr->synchandler;
    if (onoff)
	svr->synchandler = _FSSyncFunction;
    else
	svr->synchandler = NULL;
    return temp;
}

int
(* FSSetAfterFunction(svr, func)) ()
    FSServer     *svr;
    int         (*func) ();
{
    int         (*temp) ();

    temp = svr->synchandler;
    svr->synchandler = func;
    return temp;
}
