/*
 * $XConsortium$
 *
 * Copyright 1992 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/* Trace up the stack and build a call history -- SPARC specific code */

/* hack -- flush out the register windows by recursing */

static void
flushWindows (depth)
{
    if (depth == 0)
	return;
    flushWindows (depth-1);
}

getStackTrace (results, max)
    unsigned long   *results;
    int		    max;
{
    unsigned long   *sp, *getStackPointer (), *getFramePointer();
    unsigned long   *ra, mainCall;
    extern int	    main ();

    flushWindows (32);
    sp = getFramePointer ();
    while (max) 
    {
	/* sparc stack traces are easy -- chain up the saved FP/SP values */
	ra = (unsigned long *) sp[15];
	sp = (unsigned long *) sp[14];
	/* stop when we get the call to main */
	mainCall = ((((unsigned long) main) - ((unsigned long) ra)) >> 2) | 0x40000000;
	if (ra[0] == mainCall)
	{
	    *results++ = 0;
	    break;
	}
	*results++ = (unsigned long) ra;
	max--;
    }
}
