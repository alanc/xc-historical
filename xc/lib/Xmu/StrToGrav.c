/* $XConsortium: StrToGrav.c,v 1.5 93/08/16 20:40:41 kaleb Exp $ */

/* 

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#include <X11/Intrinsic.h>
#include <X11/Xmu/Converters.h>

#define done(address, type) \
        { (*toVal).size = sizeof(type); (*toVal).addr = (XPointer) address; }

static struct _namepair {
    XrmQuark quark;
    char *name;
    XtGravity gravity;
} names[] = {
    { NULLQUARK, XtEForget, ForgetGravity },
    { NULLQUARK, XtENorthWest, NorthWestGravity },
    { NULLQUARK, XtENorth, NorthGravity },
    { NULLQUARK, XtENorthEast, NorthEastGravity },
    { NULLQUARK, XtEWest, WestGravity },
    { NULLQUARK, XtECenter, CenterGravity },
    { NULLQUARK, XtEEast, EastGravity },
    { NULLQUARK, XtESouthWest, SouthWestGravity },
    { NULLQUARK, XtESouth, SouthGravity },
    { NULLQUARK, XtESouthEast, SouthEastGravity },
    { NULLQUARK, XtEStatic, StaticGravity },
    { NULLQUARK, XtEUnmap, UnmapGravity },
    { NULLQUARK, XtEleft, WestGravity },
    { NULLQUARK, XtEtop, NorthGravity },
    { NULLQUARK, XtEright, EastGravity },
    { NULLQUARK, XtEbottom, SouthGravity },
    { NULLQUARK, NULL, ForgetGravity }
};

/*
 * This function is deprecated as of the addition of 
 * XtCvtStringToGravity in R6.
 */

void XmuCvtStringToGravity (args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    static Boolean haveQuarks = FALSE;
    char lowerName[40];
    XrmQuark q;
    char *s;
    struct _namepair *np;

    if (*num_args != 0)
        XtWarningMsg("wrongParameters","cvtStringToGravity","XtToolkitError",
                  "String to Gravity conversion needs no extra arguments",
                  (String *) NULL, (Cardinal *)NULL);

    if (!haveQuarks) {
	for (np = names; np->name; np++) {
	    np->quark = XrmPermStringToQuark (np->name);
	}
	haveQuarks = TRUE;
    }

    s = (char *) fromVal->addr;
    if (strlen(s) < sizeof lowerName) {
	XmuCopyISOLatin1Lowered (lowerName, s);
	q = XrmStringToQuark (lowerName);

	for (np = names; np->name; np++) {
	    if (np->quark == q) {
		done (&np->gravity, XtGravity);
		return;
	    }
	}
    }
    XtStringConversionWarning((char *) fromVal->addr, XtRGravity);
}
