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
#include	"Xatom.h"

extern	Display	*Dsp;

/* 
 * Dummy declarations which are normally inserted by mc.
 * Needed to prevent linkstart.c being included.
 */
char	*TestName = "XGetDefault";
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
Display		*display;
char		*prog = "XTest";
char		*opt;
char		*tres;
char		*res;
char		*pval="XTest.testval41:pVAL_1\nXTest.testval42:pVAL_2\nXTest.testval43:pVAL_3\n";
int		i;
static	char	*testval[] = { "testval41", "testval42", "testval46" };
static	char	*result[]  = { "pVAL_1",    "eVAL_5",    "eVAL_6" };

	exec_startup();
	tpstartup();
	trace("Exec'd file ./Test4 with XENVIRONMENT = \"%s\".", getenv("XENVIRONMENT"));

	if(getenv("XENVIRONMENT") == (char *) NULL) {
		delete("XENVIRONMENT environment variable not set.");
		return;
	} else
		CHECK;

	XChangeProperty (Dsp, RootWindow(Dsp, 0), XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *)pval, 1+strlen(pval));
	XSync(Dsp, False);

	display = opendisplay();  /* Should merge $XENVIRONMENT file with existing database. */

	for(i=0; i< NELEM(testval); i++) {

		opt  = testval[i];
		tres = result[i];
	
		startcall(display);
		res = XGetDefault(display, prog, opt);
		endcall(display);
	
		if( res == (char *) NULL) {
			report("%s() returned NULL with program = \"%s\" and option = \"%s\".", TestName, prog, opt);
			FAIL;
		} else {
			CHECK;
			if(strcmp(res, tres) != 0) {
				report("%s() with program = \"%s\" and option = \"%s\" returned \"%s\" instead of \"%s\".", TestName, prog, opt, res, tres);
				FAIL;
			} else
				CHECK;
		}

	}

      	CHECKPASS(1 + 2*NELEM(testval));
	tpcleanup();
	exec_cleanup();
}
