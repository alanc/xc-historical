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
>>TITLE XFindContext CH10
int

Display *display = Dsp;
Window w = defwin(display);
XContext context = XUniqueContext();
caddr_t *data_return = &xfc_data;
>>EXTERN
static caddr_t xfc_data;

static char *xfc_ctxt = "another context";
>>ASSERTION Good A
A call to xname returns the context data
for display 
.A display ,
window
.A w
and context type
.A context 
in
.A data_return ,
and returns zero.
>>STRATEGY
Create a test window.
Save some context information using XSaveContext.
Find the context information using xname.
Verify the context was returned as expected.
>>CODE
int a;
int ret;

/* Create a test window. */
/* Save some context information using XSaveContext. */
	a = XSaveContext(display, w, context, (caddr_t)xfc_ctxt);
	if (a != 0) {
		delete("Could not save test context.");
		report("Returned error: %s", contexterrorname(a));
		return;
	} else
		CHECK;

/* Find the context information using xname. */
	xfc_data = (caddr_t)NULL;
	ret = XCALL;

/* Verify the context was returned as expected. */
#ifdef TEST_RPT
	xfc_data++;
#endif
	if (ret != 0) {
		FAIL;
		report("%s returned an error when expected to succeed.",
			TestName);
		report("Returned error: %s", contexterrorname(ret));
	} else {
		CHECK;
		if (xfc_data != (caddr_t)xfc_ctxt) {
			FAIL;
			report("%s did not return the expected context data.",
				TestName);
			report("Expected context: %0x", (unsigned int)xfc_ctxt);
			report("Returned context: %0x", (unsigned int)xfc_data);
		} else
			CHECK;
	}

	CHECKPASS(3);
>>ASSERTION Bad A
When there is no previously saved context data
for display
.A display ,
window
.A w
and context type
.A context ,
then a call to xname returns
.S XCNOENT .
>>STRATEGY
Create a test window with no context.
Call xname to find the non-existent context.
Verify that XCNOENT was returned.
>>CODE
int ret;

/* Create a test window with no context. */

/* Call xname to find the non-existent context. */
	ret = XCALL;

#ifdef TEST_RPT
	ret = 0;
#endif

/* Verify that XCNOENT was returned. */
	if (ret != XCNOENT) {
		FAIL;
		report("%s did not return XCNOENT when",
			TestName);
		report("finding a non-existent context.");
		report("Return code: %s", contexterrorname(ret));
	} else
		CHECK;

	CHECKPASS(1);

