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
char	*TestName = "XDisplayString";
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
char		*dispstr;
char		*rdispstr;
Display		*display;

	exec_startup();
	tpstartup();
	trace("Exec'd file ./Test1.");

	if((dispstr = getenv("DISPLAY")) == (char *) NULL) {
		delete("Environment variable DISPLAY is not set.");
		return;
	} else
		CHECK;

	if((display = XOpenDisplay((char *) NULL)) == (Display *) NULL) {
		report("%s() returned NULL.", TestName);		
		FAIL;
	} else {
		CHECK;

		rdispstr = XDisplayString(display);

		if(rdispstr == (char *) NULL) {
			delete("%s() returned NULL.", TestName);
			return;
		} else {
			CHECK;
			if(strcmp(rdispstr, dispstr) != 0) {
				report("%s() opened display  \"%s\" instead of \"%s\".", TestName, rdispstr, dispstr);
				FAIL;
			} else
				CHECK;
		}

		XCloseDisplay(display);
		CHECKPASS(4);
	}

	tpcleanup();
	exec_cleanup();
}
