/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Xlibint.h"
#include "Xresource.h"

#define XOPTIONFILE "/.Xdefaults"  /* name in home directory of options file */

static XrmDatabase InitDefaults (dpy)
    Display *dpy;			/* display for defaults.... */
{
    XrmDatabase userdb = NULL;
    XrmDatabase xdb = NULL;
    int status;
    char fname[BUFSIZ], *f;             /* longer than any conceivable size */
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
	char temp[BUFSIZ], *t;
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

