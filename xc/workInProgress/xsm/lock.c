/* $XConsortium$ */
/******************************************************************************

Copyright (c) 1994  X Consortium

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
#include <sys/types.h>
#include <fcntl.h>


Status
LockSession (session_name)

char *session_name;

{
    char *path;
    char lock_file[PATH_MAX];
    struct flock lock;
    Status status = 1;
    int fd;

    path = (char *) getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = (char *) getenv ("HOME");
	if (!path)
	    path = ".";
    }

    sprintf (lock_file, "%s/.XSMlock-%s", path, session_name);

    if ((fd = open (lock_file, O_RDWR|O_CREAT, 0666)) == -1)
	return (0);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = lock.l_len = 0;

    if (fcntl (fd, F_SETLK, &lock) == -1)
	status = 0;
    else if (ftruncate (fd, 0) < 0)
	status = 0;
    else if (write (fd, networkIds,
	strlen (networkIds)) != strlen (networkIds))
	status = 0;

    if (!status)
    {
	close (fd);
	return (0);
    }
    else
	return (1);
}


void
UnlockSession (session_name)

char *session_name;

{
    ;
}


Bool
CheckSessionLocked (session_name, get_id, id_ret)

char *session_name;
Bool get_id;
char **id_ret;

{
    char *path;
    char lock_file[PATH_MAX];
    struct flock lock;
    int fd, stat;

    path = (char *) getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = (char *) getenv ("HOME");
	if (!path)
	    path = ".";
    }

    sprintf (lock_file, "%s/.XSMlock-%s", path, session_name);

    if ((fd = open (lock_file, O_RDWR|O_CREAT, 0666)) == -1)
	return (1);

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = lock.l_len = 0;

    stat = fcntl (fd, F_GETLK, &lock);

    if (get_id)
    {
	FILE *fp = fdopen (fd, "r+");
	char buf[256];

	buf[0] = '\0';
	fscanf (fp, "%s\n", buf);
	*id_ret = XtNewString (buf);
    }

    close (fd);

    return (stat == -1 || lock.l_type != F_UNLCK);
}


void
UnableToLockSession (session_name)

char *session_name;

{
    /*
     * We should popup a dialog here giving error.
     */

    XBell (XtDisplay (topLevel), 0);
    sleep (2);

    ChooseSession ();
}
