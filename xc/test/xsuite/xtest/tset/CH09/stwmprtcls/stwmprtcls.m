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
>>TITLE XSetWMProtocols CH09
Status
XSetWMProtocols(display, w, protocols, count)
Display	*display = Dsp;
Window	w = DRW(Dsp);
Atom	*protocols = &prots;
int 	count = 1;
>>EXTERN
#include	"Xatom.h"
Atom	prots;
>>ASSERTION Good A
A call to xname sets the
.S WM_PROTOCOLS
property for the window
.A w ,
to be of type
.S ATOM ,
format 32 and to have value set
to the list of
.A count
atoms specified by the
.A protocols
argument
and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow.
Set the WM_PROTOCOLS property using XSetWMProtocols.
Verify that the call returned non-zero.
Obtain the WM_PROTOCOLS atom using XInternAtom.
Obtain the WM_PROTOCOLS property using XGetWindowProperty.
Verify that the property type is ATOM
Verify that the property format is 32.
Verify that the returned number of elements is correct.
Verify that the property value is correct.
>>CODE
Status		status;
XVisualInfo	*vp;
unsigned long	leftover, nitems, len;
int		actual_format;
Atom		actual_type;
int		nats = 5;
Atom		xa_wm_protocols;
Atom		prots[5], *rprots = (Atom *) NULL;
Atom		at, *atp;
int		i;


	for(i=0, at = XA_LAST_PREDEFINED; i<nats; i++)
		prots[i] = (int) --at;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

	protocols = prots;
	count = nats;
	status = XCALL;

	if(status == 0) {
		report("%s() returned zero.", TestName);
		FAIL;
	} else
		CHECK;

	if((xa_wm_protocols = XInternAtom(display, "WM_PROTOCOLS", True)) == None) {
		delete("The \"WM_PROTOCOLS\" string was not interned.");
		return;
	} else
		CHECK;


	if (XGetWindowProperty(display, w, xa_wm_protocols,
                            0L, (long) nats, False,
                            AnyPropertyType, &actual_type, &actual_format,
                            &nitems, &leftover, (unsigned char **) &rprots) != Success) {
		delete("XGetWindowProperty() did not return Success.");
		return;
	} else
		CHECK;

	if(leftover != 0) {
		report("The leftover elements numbered %lu instead of 0", leftover);
		FAIL;
	} else
		CHECK;

	if(actual_format != 32) {
		report("The format of the WM_PROTOCOLS property was %lu instead of 32", actual_format);
		FAIL;
	} else
		CHECK;

	if(actual_type != XA_ATOM) {
		report("The type of the WM_PROTOCOLS property was %lu instead of ATOM (%lu)", actual_type, (long) XA_ATOM);
		FAIL;
	} else
		CHECK;

	if( rprots == (Atom *) NULL) {
		report("No value was set for the WM_PROTOCOLS property.");
		FAIL;
	} else {

		CHECK;		
		if( nitems != nats) {
			report("The WM_PROTOCOLS property comprised %ul elements instead of %d", nitems, nats);
			FAIL;
		} else {

			if(actual_format == 32) {

				CHECK;
				for(i = 0, atp = rprots; i<nats; i++, atp++)
					if( *atp != prots[i]) {
						report("Element %d of the WM_PROTOCOLS value was %lu instead of %lu", i+1, (long) *atp, (long) prots[i]);
						FAIL;
					} else
						CHECK;
			}
		}
		XFree((char*)rprots);
	}

	CHECKPASS(8 + nats);

>>ASSERTION Bad B 1
When the atom name \(lqWM_PROTOCOLS\(rq cannot be interned,
then a call to xname returns zero.
>>ASSERTION Bad B 1
.ER BadAlloc 
>>ASSERTION Good A
.ER BadWindow 
>># Kieron	Action	Review
