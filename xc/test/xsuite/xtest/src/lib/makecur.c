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

#include	<stdlib.h>
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"

/*
 * Create a cursor that can be used within the test suite.
 * The cursor is created using XCreateFontCursor.  The shape
 * chosen can be controlled through the configuration variable
 * XT_FONTCURSOR_GOOD and arg. "inc".  If the config. variable is not set or is
 * undefined, the shape defaults to inc.
 */
Cursor
makecurnum(disp, inc)
Display	*disp;
int inc;
{
Cursor cursor;
char *varptr;
unsigned int shape;

	varptr = tet_getvar("XT_FONTCURSOR_GOOD");
	if (varptr == NULL || *varptr == 'U')
		shape = 0;
	else
		shape = atov(varptr);

	shape += (inc >= 0) ? (unsigned int) inc : (unsigned int) 0;

	cursor = XCreateFontCursor(disp, shape);

	/* register cursor */
	regid(disp, (union regtypes *)&cursor, REG_CURSOR);

	return(cursor);
}

/*
 * Create a cursor that can be used within the test suite.
 * The cursor is created using XCreateFontCursor.  The shape
 * chosen can be controlled through the configuration variable
 * XT_FONTCURSOR_GOOD.  If this variable is not set or is
 * undefined, the shape defaults to 0.
 */
Cursor
makecur(disp)
Display *disp;
{
	return makecurnum(disp, 0);
}

/*
 * Create a cursor that can be used within the test suite.
 * The cursor is created using XCreateFontCursor.  The shape
 * chosen can be controlled through the configuration variable
 * XT_FONTCURSOR_GOOD; it's XT_FONTCURSOR_GOOD+2.  If this
 * variable is not set or is undefined, the shape defaults to 2.
 */
Cursor
makecur2(disp)
Display *disp;
{
	return makecurnum(disp, 2);
}
