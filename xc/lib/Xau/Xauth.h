/*
 * Xau - X Authorization Database Library
 *
 * $XConsortium: Xauth.h,v 1.1 88/11/22 15:27:19 jim Exp $
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

# include   <stdio.h>

# define FamilyLocal (256)	/* not part of X standard (i.e. X.h) yet */
# define FamilyWild  (65535)

typedef struct xauth {
    unsigned short   family;
    unsigned short   address_length;
    char    *address;
    unsigned short   number_length;
    char    *number;
    unsigned short   name_length;
    char    *name;
    unsigned short   data_length;
    char    *data;
} Xauth;

#ifdef __STDC__
extern char *XauFileName	();
Xauth	    *XauReadAuth	(FILE   *auth_file);
int	    XauLockAuth		(char   *file_name,	int	retries, 
				 int    timeout,	long	dead);
int	    XauUnlockAuth	(char   *file_name);
int	    XauWriteAuth	(FILE   *auth_file,	Xauth	*auth);
Xauth	    *XauGetAuthByName	(char	*display_name);
Xauth	    *XauGetAuthByAddr	(unsigned short	family, 
			         unsigned short	address_length,	char	*address, 
			         unsigned short	number_length,	char	*number,
				 unsigned short name_length,	char	*name);
void	    XauDisposeAuth	(Xauth	*auth);
#else
extern char *XauFileName	();
Xauth	    *XauReadAuth	();
int	    XauLockAuth		();
int	    XauUnlockAuth	();
int	    XauWriteAuth	();
Xauth	    *XauGetAuthByName	();
Xauth	    *XauGetAuthByAddr	();
void	    XauDisposeAuth	();
#endif

/* Return values from XauLockAuth */

# define LOCK_SUCCESS	0	/* lock succeeded */
# define LOCK_ERROR	1	/* lock unexpectely failed, check errno */
# define LOCK_TIMEOUT	2	/* lock failed, timeouts expired */
