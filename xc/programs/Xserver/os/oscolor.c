/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/* $Header: oscolor.c,v 1.9 87/10/22 11:54:26 rws Locked $ */
#include <dbm.h>
#include "rgb.h"

/* Looks up the color in the database.  Note that we are assuming there
 * is only one database for all the screens.  If you have multiple databases,
 * remove the dbminit() in OsInit(), and open the appropriate database each
 * time. Or implement a database package that allows you to have more than
 * one database open at a time.
 */
extern int havergb;

/*ARGSUSED*/
int
OsLookupColor(screen, name, len, pred, pgreen, pblue)
    int		screen;
    char	*name;
    unsigned	len;
    unsigned short	*pred, *pgreen, *pblue;

{
    datum	dbent;
    RGB		rgb;

    if(!havergb)
	return(0);

    dbent.dptr = name;
    dbent.dsize = len;
    dbent = fetch (dbent);

    if(dbent.dptr)
    {
	bcopy(dbent.dptr, (char *) &rgb, sizeof (RGB));
	*pred = rgb.red;
	*pgreen = rgb.green;
	*pblue = rgb.blue;
	return (1);
    }
    return(0);
}

