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
>>TITLE XMatchVisualInfo CH10
Status
XMatchVisualInfo(display, screen, depth, class, vinfo)
Display	*display = Dsp;
int 	screen = DefaultScreen(Dsp);
int 	depth;
int 	class;
XVisualInfo	*vinfo;
>>ASSERTION Good A
When a visual exists on screen
.A screen
of depth
.A depth
and of class
.A class,
then a call to xname returns the associated
.S XVisualInfo
structure in the
.A vinfo_return
argument, and returns non-zero.
>>STRATEGY
Initialise the list of class/depth pairs from parameter XT_VISUAL_CLASSES.
For each class/depth pair:
  Call xname.
  Verify that xname does not return zero.
  Verify that class and depth of returned structure are correct.
>>CODE
Status	s;
XVisualInfo	vi;

/* Initialise the list of class/depth pairs from parameter XT_VISUAL_CLASSES. */
	if(initvclass() < 0) {
		delete("The list of expected visual classes could not be initialised.");
		return;
	}
	vinfo = &vi;
/* For each class/depth pair: */
	for (resetvclass(); nextvclass(&class, &depth); ) {

/*   Call xname. */
		s = XCALL;

		trace("Matching depth=%d, class=%s", depth, displayclassname(class));

/*   Verify that xname does not return zero. */
		if (s == 0) {
			report("%s returned 0 with depth=%d, class=%s", 
				TestName, depth, displayclassname(class));
			FAIL;
/*   Verify that class and depth of returned structure are correct. */
		} else if (vi.class != class || vi.depth != depth) {
			/*
			 * Since the actual visual returned is not defined then
			 * all we can do is check that class and depth are OK
			 * and output all the other fields for regression test purposes.
			 */
			report(" Incorrect class or depth found");
			report(" Got %s, depth %d", displayclassname(vi.class), vi.depth);
			report(" Expecting %s, depth", displayclassname(class),
				depth);
			FAIL;
		} else {
			trace(" visualid=0x%x, screen=%d, depth=%u, class=%s",
				vi.visualid, vi.screen, vi.depth,
				displayclassname(vi.class));
			trace(" red_mask=0x%x, green_mask=0x%x, blue_mask=0x%x",
				vi.red_mask,
				vi.green_mask,
				vi.blue_mask);
			trace(" colormap_size=%d, bits_per_rgb=%d",
				vi.colormap_size,
				vi.bits_per_rgb);

			CHECK;
		}
	}

	CHECKPASS(nvclass());

>>ASSERTION Good A
When a visual does not exist on screen
.A screen
of depth
.A depth
and of class
.A class,
then a call to xname returns zero.
>>STRATEGY
Call xname with depth zero.
Verify that xname returns zero.
>>CODE
Status	s;

	/*
	 * Try with a depth of 0 which is always invalid.
	 */
	depth = 0;
	class = StaticColor;

	s = XCALL;

	if (s == 0)
		CHECK;
	else {
		report("Return value was %d", s);
		FAIL;
	}

	CHECKPASS(1);
