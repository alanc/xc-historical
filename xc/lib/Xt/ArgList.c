#ifndef lint
static char rcsid[] = "$Header: ArgList.c,v 1.11 88/02/14 14:52:47 rws Exp $";
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

#include	<X11/Xlib.h>
#include	<stdio.h>
#include	"IntrinsicI.h"

#ifndef lint
static void PrintArgList(args, num_args)
    ArgList	args;
    int num_args;
{
    for (; --num_args >= 0; args++) {
	(void) printf("name: %s, value: 0x%x\n", args->name, args->value);
    }
}
#endif

/*
 * This routine merges two arglists. It does NOT check for duplicate entries.
 */

ArgList XtMergeArgLists(args1, num_args1, args2, num_args2)
    ArgList args1;
    Cardinal num_args1;
    ArgList args2;
    Cardinal num_args2;
{
    ArgList result, args;

    result = (ArgList) XtCalloc((unsigned) num_args1 + num_args2,
				(unsigned) sizeof(Arg));

    for (args = result; num_args1 != 0; num_args1--)
    	*args++ = *args1++;
    for (             ; num_args2 != 0; num_args2--)
    	*args++ = *args2++;

    return result;
}

 
