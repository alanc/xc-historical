/*
 * Xau - X Authorization Database Library
 *
 * $XConsortium: $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

# include   "Xauth.h"
# include   <sys/types.h>
# include   <sys/stat.h>
# include   <sys/errno.h>

int
XauLockAuth (file_name, retries, timeout, dead)
char	*file_name;
int	retries;
int	timeout;
long	dead;
{
    char	creat_name[1025], link_name[1025];
    char	*strcpy (), *strcat ();
    long	time ();
    unsigned	sleep ();
    struct stat	statb;
    long	now;
    int		creat_fd = -1;
    extern int	errno;

    if (strlen (file_name) > 1022)
	return LOCK_ERROR;
    (void) strcpy (creat_name, file_name);
    (void) strcat (creat_name, "-c");
    (void) strcpy (link_name, file_name);
    (void) strcat (link_name, "-l");
    if (stat (creat_name, &statb) != -1) {
	now = time ((long *) 0);
	if (now - statb.st_ctime > dead)
	    (void) unlink (creat_name);
    }
    
    while (retries > 0) {
	if (creat_fd == -1) {
	    creat_fd = creat (creat_name, 0666);
	    if (creat_fd == -1) {
		if (errno != EACCES)
		    return LOCK_ERROR;
	    } else
		(void) close (creat_fd);
	} else {
	    if (link (creat_name, link_name) != -1)
		return LOCK_SUCCESS;
	    if (errno != EEXIST)
		return LOCK_ERROR;
	}
	(void) sleep ((unsigned) timeout);
	--retries;
    }
    return LOCK_TIMEOUT;
}
