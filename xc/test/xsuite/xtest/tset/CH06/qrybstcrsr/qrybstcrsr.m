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
>>TITLE XQueryBestCursor CH06
Status
XQueryBestCursor(display, d, width, height, rwidth, rheight)
Display *display = Dsp;
Drawable d;
unsigned int width;
unsigned int height;
unsigned int *rwidth = &cwidth;
unsigned int *rheight = &cheight;
>>EXTERN
static unsigned int cheight, cwidth;
>>ASSERTION Good B 1
A call to xname returns in the
.A width_return
and
.A height_return
arguments the largest cursor height and width to those specified in
.A height
and
.A width
that can be fully displayed on the specified screen.
>>STRATEGY
Call XQueryBestCursor with width twice that of DisplayWidth and
height twice that of DisplayHeight.
Verify that XQueryBestCursor returns non-zero.
Call XQueryBestCursor with returned values of width and height.
Verify that XQueryBestCursor returns non-zero.
Verify that XQueryBestCursor returns the previously returned
values for width and height.
Verify that returned values for width and height are small enough
to be fully displayed on the specified screen.
Repeat with initial width and height both set to zero.
>>CODE
unsigned int swidth;
unsigned int sheight;
Status qstat;

	d = DRW(display);

/* Dynamically determine width and height values */
	swidth = DisplayWidth(display, DefaultScreen(display));
	sheight = DisplayHeight(display, DefaultScreen(display));
	width = 2 * swidth;
	height = 2 * sheight;

/* Call XQueryBestCursor with width twice that of DisplayWidth and */
/* twice that of DisplayHeight. */
	qstat = XCALL;

/* Verify that XQueryBestCursor returns non-zero. */
	if (qstat == 0) {
		report("%s returned wrong value %ld", TestName, (long) qstat);
		FAIL;
	} else
		CHECK;

	trace("Status returned was %d", qstat);
	trace("Best height (for %d ) = %d", height, cheight);
	trace("Best width (for %d)  = %d", width, cwidth);

	width = cwidth;
	height = cheight;

	cwidth = cheight = 0;

/* Call XQueryBestCursor with returned values of width and height. */
	qstat = XCALL;

/* Verify that XQueryBestCursor returns non-zero. */
	if(qstat == 0) {
		report("%s returned wrong value %ld", TestName, (long) qstat);
		FAIL;
	} else
		CHECK;

	trace("Status returned was %d", qstat);
	trace("Best height (for %d ) = %d", height, cheight);
	trace("Best width (for %d)  = %d", width, cwidth);

/* Verify that XQueryBestCursor returns the previously returned */
	if(width != cwidth) {
		report("%s returned best width %d", TestName, cwidth);
		report("after previously returning best width %d", width);
		FAIL;
	} else
		CHECK;

	if(height != cheight) {
		report("%s returned best height %d", TestName, cheight);
		report("after previously returning best height %d", height);
		FAIL;
	} else
		CHECK;
/* Verify that returned values for width and height are small enough */
/* to be fully displayed on the specified screen. */
	if (cwidth > swidth) {
		report("%s returned non-fully displayable width %d",
			TestName, cwidth);
		FAIL;
	} else
		CHECK;
	if (cheight > sheight) {
		report("%s returned non-fully displayable height %d",
			TestName, cheight);
		FAIL;
	} else
		CHECK;

/* Repeat with initial width and height both set to zero. */
	width = 0;
	height = 0;

	qstat = XCALL;

	if(qstat == 0) {
		report("%s returned wrong value %ld", TestName, (long) qstat);
		FAIL;
	} else
		CHECK;

	trace("Status returned was %d", qstat);
	trace("Best height (for %d ) = %d", height, cheight);
	trace("Best width (for %d)  = %d", width, cwidth);

	width = cwidth;
	height = cheight;

	cwidth = cheight = 0;

	qstat = XCALL;

	if(qstat == 0) {
		report("%s returned wrong value %ld", TestName, (long) qstat);
		FAIL;
	} else
		CHECK;

	trace("Status returned was %d", qstat);
	trace("Best height (for %d ) = %d", height, cheight);
	trace("Best width (for %d)  = %d", width, cwidth);

	if(width != cwidth) {
		report("%s returned best width %d", TestName, cwidth);
		report("after previously returning best width %d", width);
		FAIL;
	} else
		CHECK;

	if(height != cheight) {
		report("%s returned best height %d", TestName, cheight);
		report("after previously returning best height %d", height);
		FAIL;
	} else
		CHECK;
	if (cwidth > swidth) {
		report("%s returned non-fully displayable width %d",
			TestName, cwidth);
		FAIL;
	} else
		CHECK;
	if (cheight > sheight) {
		report("%s returned non-fully displayable height %d",
			TestName, cheight);
		FAIL;
	} else
		CHECK;

	CHECKUNTESTED(12);
>>ASSERTION Bad A
.ER BadDrawable 
>>#HISTORY peterc Completed Updated as per RTCB#3
>>#HISTORY peterc Completed engineering
