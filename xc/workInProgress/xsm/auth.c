/* $XConsortium: auth.c,v 1.9 94/11/30 18:11:29 mor Exp mor $ */
/******************************************************************************

Copyright (c) 1993  X Consortium

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
******************************************************************************/

#include "xsm.h"

static char *remAuthFile;



/*
 * Host Based Authentication Callback.  This callback is invoked if
 * the connecting client can't offer any authentication methods that
 * we can accept.  We can accept/reject based on the hostname.
 */

Bool
HostBasedAuthProc (hostname)

char *hostname;

{
    return (0);	      /* For now, we don't support host based authentication */
}



/*
 * We use temporary files which contain commands to add/remove entries from
 * the .ICEauthority file.
 */

static void
write_iceauth (addfp, removefp, entry)

FILE		 *addfp;
FILE 		 *removefp;
IceAuthDataEntry *entry;

{
    fprintf (addfp,
	"add %s \"\" %s %s ",
	entry->protocol_name,
        entry->network_id,
        entry->auth_name);
    fprintfhex (addfp, entry->auth_data_length, entry->auth_data);
    fprintf (addfp, "\n");

    fprintf (removefp,
	"remove protoname=%s protodata=\"\" netid=%s authname=%s\n",
	entry->protocol_name,
        entry->network_id,
        entry->auth_name);
}



/*
 * Provide authentication data to clients that wish to connect
 */

#define MAGIC_COOKIE_LEN 16

Status
SetAuthentication (count, listenObjs, authDataEntries)

int			count;
IceListenObj		*listenObjs;
IceAuthDataEntry	**authDataEntries;

{
    FILE	*addfp;
    FILE	*removefp;
    char	*path;
    int		original_umask;
    char	*addAuthFile;
    char	command[256];
    int		i;

    original_umask = umask (0077);	/* disallow non-owner access */

    path = getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = getenv ("HOME");
	if (!path)
	    path = ".";
    }

    if ((addAuthFile = (char *) XtNewString (
	(char *) tempnam (path, ".xsm"))) == NULL)
	return (0);

    if ((remAuthFile = (char *) XtNewString (
	(char *) tempnam (path, ".xsm"))) == NULL)
	return (0);

    if (!(addfp = fopen (addAuthFile, "w")))
	return (0);

    if (!(removefp = fopen (remAuthFile, "w")))
	return (0);

    *authDataEntries = (IceAuthDataEntry *) XtMalloc (
	count * 2 * sizeof (IceAuthDataEntry));

    for (i = 0; i < count * 2; i += 2)
    {
	(*authDataEntries)[i].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	(*authDataEntries)[i].protocol_name = "ICE";
	(*authDataEntries)[i].auth_name = "MIT-MAGIC-COOKIE-1";

	(*authDataEntries)[i].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*authDataEntries)[i].auth_data_length = MAGIC_COOKIE_LEN;

	(*authDataEntries)[i+1].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	(*authDataEntries)[i+1].protocol_name = "XSMP";
	(*authDataEntries)[i+1].auth_name = "MIT-MAGIC-COOKIE-1";

	(*authDataEntries)[i+1].auth_data = 
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*authDataEntries)[i+1].auth_data_length = MAGIC_COOKIE_LEN;

	write_iceauth (addfp, removefp, &(*authDataEntries)[i]);
	write_iceauth (addfp, removefp, &(*authDataEntries)[i+1]);

	IceSetPaAuthData (2, &(*authDataEntries)[i]);

	IceSetHostBasedAuthProc (listenObjs[i/2], HostBasedAuthProc);
    }

    fclose (addfp);
    fclose (removefp);

    umask (original_umask);

    sprintf (command, "iceauth source %s", addAuthFile);
    system (command);

    sprintf (command, "rm %s", addAuthFile);
    system (command);

    XtFree (addAuthFile);

    return (1);
}



/*
 * Free up authentication data.
 */

void
FreeAuthenticationData (count, authDataEntries)

int			count;
IceAuthDataEntry 	*authDataEntries;

{
    /* Each transport has entries for ICE and XSMP */

    char command[256];
    int i;

    for (i = 0; i < count * 2; i++)
    {
	free (authDataEntries[i].network_id);
	free (authDataEntries[i].auth_data);
    }

    XtFree ((char *) authDataEntries);

    sprintf (command, "iceauth source %s", remAuthFile);
    system (command);

    sprintf (command, "rm %s", remAuthFile);
    system (command);

    XtFree (remAuthFile);
}
