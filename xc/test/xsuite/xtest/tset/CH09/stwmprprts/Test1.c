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
#include	"tet_api.h"
#include	"xtestlib.h"
#include	"pixval.h"


extern	Display	*Dsp;

/* 
 * Dummy declarations which are normally inserted by mc.
 * Needed to prevent linkstart.c being included.
 */
char	*TestName = "XSetWMProperties";
int     tet_thistest;
struct tet_testlist tet_testlist[] = {
	NULL, 0
};
int 	ntests = sizeof(tet_testlist)/sizeof(struct tet_testlist)-1;

tet_main(argc, argv, envp)
int argc;	
char *argv[];
char *envp[];
{
int		pass = 0, fail = 0;
char		*res_name;
Window		win;
XVisualInfo	*vp;
XClassHint	class_hints;
XClassHint	rclass_hints;

	exec_startup();
	tpstartup();

	trace("Exec'd file ./Test1.");
	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(Dsp, vp);

	class_hints.res_name = NULL;
	class_hints.res_class = "XTest_Undefined.";

	if( (res_name = (char *) getenv("RESOURCE_NAME")) == NULL) {
		delete("RESOURCE_NAME environment variable is not set.");
		return;
	} else
		CHECK;

	startcall(Dsp);
	XSetWMProperties( Dsp, win, (XTextProperty *) NULL, (XTextProperty *) NULL, (char **) NULL, 0, (XSizeHints *) NULL,  (XWMHints *) NULL, &class_hints);
	endcall(Dsp);

	if (geterr() != Success) {
		report("Got %s, Expecting Success", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	if( XGetClassHint(Dsp, win, &rclass_hints) == 0 ) {
		delete("XGetClassHints returned zero.");
		return;
	} else
		CHECK;

	if(rclass_hints.res_name == NULL) {
		report("The res_name component of the returned XClassHint structure was NULL.");
		FAIL;
	} else {
		CHECK;
		if( strcmp(rclass_hints.res_name, res_name) != 0 ) {
			report("The res_name component of the returned XClassHint structure was \"%s\" instead of \"%s\".", rclass_hints.res_name, res_name);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(5);
	tpcleanup();
	exec_cleanup();
}
