#ifndef lint
static char rcsid[] = "$Header: XGetDflt.c,v 1.5 88/02/03 23:53:56 swick Exp $";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include <stdio.h>
#include <ctype.h>
#include <X11/Xos.h>
#include "Xlibint.h"
#include <Xresource.h>

#define XOPTIONFILE "/.Xdefaults"  /* name in home directory of options file */

static XrmDatabase InitDefaults (dpy)
    Display *dpy;			/* display for defaults.... */
{
    XrmDatabase userdb = NULL;
    XrmDatabase xdb = NULL;
    char fname[BUFSIZ];                 /* longer than any conceivable size */
    char *getenv();
    char *home = getenv("HOME");

    /*
     * attempt to generate user's file name
     */
    fname[0] = '\0';
    if (home != NULL) {
	strcpy (fname, home);
	strcat (fname, XOPTIONFILE);
	}

    XrmInitialize();

    if (fname[0] != '\0') userdb =  XrmGetFileDatabase(fname);
    xdb = XrmGetStringDatabase(dpy->xdefaults);
    XrmMergeDatabases(userdb, &xdb);
    return xdb;
}

char *XGetDefault(dpy, prog, name)
	Display *dpy;			/* display for defaults.... */
	register char *name;		/* name of option program wants */
	char *prog;			/* name of program for option	*/

{					/* to get, for example, "font"  */
	char temp[BUFSIZ];
	XrmAtom type;
	XrmValue result;


	/*
	 * see if database has ever been initialized.  Lookups can be done
	 * without locks held.
	 */
	LockDisplay(dpy);
	if (dpy->db == NULL) {
		dpy->db = InitDefaults(dpy);
		}
	UnlockDisplay(dpy);

	sprintf(temp, "%s.%s", prog, name);
	XrmGetResource(dpy->db, temp, "Program.Name", &type, &result);

	return (result.addr);
}

