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
>>TITLE XGetErrorText CH08
void
XGetErrorText(display, code, buffer_return, length)
Display *display = Dsp;
int code = 1;
char *buffer_return = buffer;
int length = sizeof(buffer);
>>EXTERN
static char buffer[512];
>>ASSERTION Good A
A call to xname
returns in
.A buffer_return
a string terminated with ASCII NUL.
>>STRATEGY
Set each character in buffer_return to non-ASCII NUL.
Call XGetErrorText.
Verify that there exists at least one ASCII NUL
character in buffer_return.
>>CODE
int	i;

/* Set each character in buffer_return to non-ASCII NUL. */
	for (i=0; i < sizeof(buffer); i++) {
		if (!i)
			CHECK;
		buffer[i] = 'A';
	}
	buffer_return = buffer;
/* Call XGetErrorText. */
	XCALL;
/* Verify that there exists at least one ASCII NUL */
/* character in buffer_return. */
	for (i=0; i < sizeof(buffer); i++) {
		if (buffer[i] == '\0') {
			CHECK;
			break;
		}
	}
	if (i == sizeof(buffer)) {
		report("Returned string not terminated with ASCII NUL.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(3);
>>ASSERTION Good B 1
>>#NOTE How to prove this??? Answer: tet result code FIP
After a call to xname
.A buffer_return
contains a textual description of
.A code .
