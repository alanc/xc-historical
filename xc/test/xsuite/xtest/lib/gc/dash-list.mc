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
>>EXTERN
static char	dashlist[] = {
	1, 5, 3, 9, 15, 1, 1, 2, 2, 3, 3,
	2, 3, 1, 5, 1, 7, 1, 2, 1};
>>ASSERTION Good A
The
initial and alternate elements of the
.M dash-list
within the GC
specify the lengths of the even dashes and 
the second and alternate elements specify the lengths of the odd dashes.
>>STRATEGY
Set graphics coordinates for dashed lines 
	(includes horizontal and vertical cases,
	and includes joins and caps where relevant).
Set the line_style of the GC to LineOnOffDash using XChangeGC.
Set the dash_list of the GC to even length list using XSetDashes.
Clear drawable.
Draw lines.
Verify that dashes drawn correspond to dash_list (use pixmap checking).
Repeat with odd length dash_list.
Repeat with single dash of length 255 in dash_list.
>>CODE
XVisualInfo	*vp;
static unsigned char	longdash[] = {
	255};

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();
		setlinestyle(A_DISPLAY, A_GC, LineOnOffDash);

		trace("various length dashes, %d in list", NELEM(dashlist));
		XSetDashes(A_DISPLAY, A_GC, 0, dashlist, NELEM(dashlist));
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		trace("various length dashes, %d in list", NELEM(dashlist)-1);
		XSetDashes(A_DISPLAY, A_GC, 0, dashlist, NELEM(dashlist)-1);
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);

		trace("dash length of 255");
		XSetDashes(A_DISPLAY, A_GC, 0, (char *)longdash, NELEM(longdash));
		dclear(A_DISPLAY, A_DRAWABLE);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
	}

	CHECKPASS(3*nvinf());

>>ASSERTION Good A
The dashes component of the GC specifies the length of both even and
odd dashes.
>>STRATEGY
Set the dashes component of the GC using XChangeGC.
Draw lines.
Verify that even and odd dashes are same length (use pixmap checking).
>>CODE
XVisualInfo	*vp;
XGCValues	gcv;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();
		setlinestyle(A_DISPLAY, A_GC, LineOnOffDash);

		gcv.dashes = 8;
		XChangeGC(A_DISPLAY, A_GC, GCDashList, &gcv);

		XCALL;

		PIXCHECK(A_DISPLAY, A_DRAWABLE);
	}

	CHECKPASS(nvinf());

>>ASSERTION def
>>#There is nothing new to test as the first test purpose implicitly tests this.
When the line is horizontal, then the length of a dash is measured along the
x axis.
>>ASSERTION def
>>#There is nothing new to test as the first test purpose implicitly tests this.
When the line is vertical, then the length of a dash is measured along the
y axis.
