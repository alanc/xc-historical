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
>>TITLE XSaveContext CH10
int

Display *display = Dsp;
Window w = defwin(display);
XContext context = XUniqueContext();
caddr_t data;
>>EXTERN

static char *xsc_ctxt ="set context";
static char *xsc_ctxt2 ="set context two";

>>ASSERTION Good A
A call to xname sets the table entry for context data
for display 
.A display ,
window
.A w
and context type
.A context 
to the specified value
.A data ,
and returns zero.
>>STRATEGY
Call xname to enter the context data.
Verify that zero was returned.
Call XFindContext to verify that the context data was added correctly.
>>CODE
int a;
int ret;
caddr_t b;

/* Call xname to enter the context data. */
	data = (caddr_t) xsc_ctxt;
	ret = XCALL;

/* Verify that zero was returned. */
	if (ret != 0) {
		FAIL;
		report("%s returned non-zero when expected zero.",
			TestName);
		report("Returned value: %s", contexterrorname(ret));
	} else
		CHECK;

/* Call XFindContext to verify that the context data was added correctly. */
	a = XFindContext(display, w, context, &b);
	if (a != 0) {
		FAIL;
		report("XFindContext failed to find the context saved by %s", TestName);
		report("XFindContext returned %s", contexterrorname(a));
	} else {
		CHECK;

		if (b != (caddr_t)xsc_ctxt) {
			FAIL;
			report("XFindContext returned an unexpected context.");
			report("Expected context: %0x", (unsigned int)xsc_ctxt);
			report("Returned context: %0x", (unsigned int)b);
		} else
			CHECK;
	}

	CHECKPASS(3);

>>ASSERTION Good A
When there is previously saved context data
for display
.A display ,
window
.A w
and context type
.A context ,
then a call to xname replaces the previously saved context data
with the specified value
.A data ,
and returns zero.
>>STRATEGY
Call xname to set the context data.
Verify that zero was returned.
Call xname to reset the context data.
Verify that zero was returned.
Call XFindContext to verify that the context data was added correctly.
>>CODE
int a;
int ret;
caddr_t b;

/* Call xname to set the context data. */
	data = (caddr_t) xsc_ctxt;
	ret = XCALL;

/* Verify that zero was returned. */
	if (ret != 0) {
		FAIL;
		report("%s returned non-zero when expected zero when setting",
			TestName);
		report("context information.");
		report("Returned value: %s", contexterrorname(ret));
	} else
		CHECK;

/* Call xname to reset the context data. */
	data = (caddr_t) xsc_ctxt2;
	ret = XCALL;

/* Verify that zero was returned. */
	if (ret != 0) {
		FAIL;
		report("%s returned non-zero when expected zero when resetting",
			TestName);
		report("context information.");
		report("Returned value: %s", contexterrorname(ret));
	} else
		CHECK;

/* Call XFindContext to verify that the context data was added correctly. */
	a = XFindContext(display, w, context, &b);
	if (a != 0) {
		FAIL;
		report("XFindContext failed to find the context saved by %s", TestName);
		report("XFindContext returned %s", contexterrorname(a));
	} else {
		CHECK;

		if (b != (caddr_t)xsc_ctxt2) {
			FAIL;
			report("XFindContext returned an unexpected context.");
			report("Original context: %0x", (unsigned int)xsc_ctxt);
			report("Expected context: %0x", (unsigned int)xsc_ctxt2);
			report("Returned context: %0x", (unsigned int)b);
		} else
			CHECK;
	}

	CHECKPASS(4);
>>ASSERTION Bad B 1
When there is insufficient memory, then a call to xname returns
.S XCNOMEM .
