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
>>TITLE XGetMotionEvents CH08
XTimeCoord *
XGetMotionEvents(display, w, start, stop, nevents_return)
Display *display = Dsp;
Window w = DRW(display);
Time start = CurrentTime;
Time stop = CurrentTime;
int *nevents_return = &_nevents_return;
>>EXTERN
static	int	_nevents_return;
>>ASSERTION Good D 1
If
the implementation supports a more complete
history of the pointer motion than is reported by event notification:
a call to xname
returns all events in the motion history buffer
that fall between the
.A start
and
.A stop
times, inclusive,
which have coordinates that lie within the specified window
(including its borders)
at its present placement
and sets
.A nevents_return
to the number of events returned.
>>STRATEGY
If a pointer motion buffer is not supported, return.
>>CODE
/* If a pointer motion buffer is not supported, return. */
	if (!config.displaymotionbuffersize)
		unsupported("Pointer motion buffer is not supported.");
	else
		untested("There is no known portable test method for this assertion");
>>ASSERTION Good C
If
the implementation does not support a more complete
history of pointer motion than is reported by event notification:
a call to xname returns no events.
>>STRATEGY
If a pointer motion buffer is supported, return.
Call XGetMotionEvents.
Verify that no events were returned.
>>CODE
XTimeCoord *tc;

/* If a pointer motion buffer is supported, return. */
	if (config.displaymotionbuffersize != 0) {
		report("Pointer motion buffer is supported.");
		UNSUPPORTED;
		return;
	}
	else
		CHECK;
	start = 0;
	stop = CurrentTime;
	*nevents_return = 1;
/* Call XGetMotionEvents. */
	tc = XCALL;
/* Verify that no events were returned. */
	if (tc != (XTimeCoord *) NULL) {
		report("Returned 0x%x, expected NULL", tc);
		FAIL;
		XFree((char*)tc);
	}
	else
		CHECK;
	if (*nevents_return != 0) {
		report("Returned %d, expected 0", *nevents_return);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);
>>ASSERTION Good A
When
.A start
is later than
.A stop ,
then a call to xname returns no events.
>>STRATEGY
Set stop to current time.
Call XGetMotionEvents with start greater than stop.
Verify that no events were returned.
>>CODE
XTimeCoord *tc;

/* Set stop to current time. */
	stop = gettime(display);
/* Call XGetMotionEvents with start greater than stop. */
	start = stop + 1;
	tc = XCALL;
/* Verify that no events were returned. */
	if (tc != (XTimeCoord *) NULL) {
		report("Returned 0x%x, expected NULL", tc);
		FAIL;
		XFree((char*)tc);
	}
	else
		CHECK;
	if (*nevents_return != 0) {
		report("Returned %d, expected 0", *nevents_return);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good A
When
.A start
is in the future,
then a call to xname returns no events.
>>STRATEGY
Set stop to current time.
Set start to a future time.
Call XGetMotionEvents.
Verify that no events were returned.
>>CODE
XTimeCoord *tc;

/* Set stop to current time. */
	stop = CurrentTime;
/* Set start to a future time. */
	start = gettime(display) + 10000;
/* Call XGetMotionEvents. */
	tc = XCALL;
/* Verify that no events were returned. */
	if (tc != (XTimeCoord *) NULL) {
		report("Returned 0x%x, expected NULL", tc);
		FAIL;
		XFree((char*)tc);
	}
	else
		CHECK;
	if (*nevents_return != 0) {
		report("Returned %d, expected 0", *nevents_return);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good B 1
>>#NOTE	I do not believe that this is testable (we can not generate events).
A call to xname with
.A stop
in the future,
is equivalent to specifying
a value of
.S CurrentTime
for
.A stop .
>>ASSERTION Good D 1
On a call to xname
the
.M x
and
.M y
members of the events returned
are set to the coordinates of the pointer
relative to the origin
of the specified window and the
.M time
member is set to the time the pointer reached this coordinate.
>>ASSERTION Bad A
.ER BadWindow
