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

#include "Xlib.h"

#ifdef GENERATE_PIXMAPS
/* in this case we never want to do anything like real buffer stuffing or
 * I/O on our "display" as it isn't real enough for that.
 */
#undef XTESTEXTENSION
#endif /* GENERATE_PIXMAPS */

extern      Display *Dsp;

#ifndef XTestExtName
# define XTestExtName "XTEST"
#endif

int XTTestExtn = 0;

Status
IsExtTestAvailable()
{
#ifdef XTESTEXTENSION
	int	mop;
	int	dev;
	int	der;

	switch(XTTestExtn) {
	        case -1 :
		    return(False);
		case 1 :
			return(True);
		default:
		    if(XQueryExtension(Dsp, XTestExtName, &mop, &dev, &der) == False) {
			    XTTestExtn = -1;
			    return(False);
		    } else {
			    XTTestExtn = 1;
			    return(True);
		    }
	    }
#else
	return(False);
#endif
}
