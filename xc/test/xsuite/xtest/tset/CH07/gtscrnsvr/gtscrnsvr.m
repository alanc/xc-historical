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
>>TITLE XGetScreenSaver CH07
void

Display	*display = Dsp;
int 	*timeout_return = &tr;
int 	*interval_return = &ir;
int 	*prefer_blanking_return = &pbr;
int 	*allow_exposures_return = &aer;
>>EXTERN

static	int 	tr;
static	int 	ir;
static	int 	pbr;
static	int 	aer;

static	int 	origt;
static	int 	origi;
static	int 	origpb;
static	int 	origae;

>>SET startup savesaver
static void
savesaver()
{
	startup();
	if(Dsp)
		XGetScreenSaver(Dsp, &origt, &origi, &origpb, &origae);
}

>>SET cleanup resetsaver
static void
resetsaver()
{
	if(Dsp)
		XSetScreenSaver(Dsp, origt, origi, origpb, origae);
	cleanup();
}

>>ASSERTION Good A
A call to xname returns the current screen saver values.
>>STRATEGY
Set screen saver values.
Get screen saver values.
Verify that returned values are as set.
>>EXTERN
#define	TOUT	71
#define	INTERVAL	57
#define	BLANKING	PreferBlanking
#define	EXPOSURES	AllowExposures
>>CODE

	XSetScreenSaver(display, TOUT, INTERVAL, BLANKING, EXPOSURES);

	XCALL;

	if (*timeout_return == TOUT)
		CHECK;
	else {
		report("timeout_return was %d, expecting %d", *timeout_return, TOUT);
		FAIL;
	}
	if (*interval_return == INTERVAL)
		CHECK;
	else {
		report("interval_return was %d, expecting %d",
			*interval_return, INTERVAL);
		FAIL;
	}
	if (*prefer_blanking_return == BLANKING)
		CHECK;
	else {
		report("prefer_blanking_return was %d, expecting %d",
			*prefer_blanking_return, BLANKING);
		FAIL;
	}
	if (*allow_exposures_return == EXPOSURES)
		CHECK;
	else {
		report("allow_exposures_return was %d, expecting %d",
			*allow_exposures_return, EXPOSURES);
		FAIL;
	}

	CHECKPASS(4);
