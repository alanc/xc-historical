/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */

static	int 	passcount;
static	int 	failcount;

/*
 * This routine exists just so that the pass and fail counters in each
 * test purpose can be 'used' and thus save a lot of output from lint
 * and fussy compilers.  Note that these counts are not of any other
 * use at all because there is no guarantee that pfcount is actually
 * called.
 */
pfcount(pass, fail)
int 	pass;
int 	fail;
{
	passcount += pass;
	failcount += failcount;
}

rptcounts()
{
	trace("passcount=%d, failcount=%d", passcount, failcount);
}
