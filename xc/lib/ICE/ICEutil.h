/* $XConsortium: ICEutil.h,v 1.1 94/02/06 15:22:41 mor Exp $ */
/******************************************************************************

Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.

Author: Ralph Mor, X Consortium
******************************************************************************/

#ifndef ICEUTIL_H
#define ICEUTIL_H

#include <stdio.h>

/*
 * Data structure for entry in ICE authority file
 */

typedef struct {
    char    	    *protocol_name;
    unsigned short  protocol_data_length;
    char   	    *protocol_data;
    char    	    *address;
    char    	    *auth_name;
    unsigned short  auth_data_length;
    char   	    *auth_data;
} IceAuthFileEntry;


/*
 * Authentication data maintained in memory.
 */

typedef struct {
    char    	    *protocol_name;
    char	    *address;
    char    	    *auth_name;
    unsigned short  auth_data_length;
    char   	    *auth_data;
} IceAuthDataEntry;


/*
 * Return values from IceLockAuthFile
 */

#define IceAuthLockSuccess	0   /* lock succeeded */
#define IceAuthLockError	1   /* lock unexpectely failed, check errno */
#define IceAuthLockTimeout	2   /* lock failed, timeouts expired */


/*
 * Function Prototypes
 */

extern char *IceAuthFileName (
#if NeedFunctionPrototypes
    void
#endif
);

extern int IceLockAuthFile (
#if NeedFunctionPrototypes
    char *		/* file_name */,
    int			/* retries */,
    int			/* timeout */,
    long		/* dead */
#endif
);

extern void IceUnlockAuthFile (
#if NeedFunctionPrototypes
    char *		/* file_name */
#endif
);

extern IceAuthFileEntry *IceReadAuthFileEntry (
#if NeedFunctionPrototypes
    FILE *		/* auth_file */
#endif
);

extern void IceFreeAuthFileEntry (
#if NeedFunctionPrototypes
    IceAuthFileEntry *	/* auth */
#endif
);

extern Status IceWriteAuthFileEntry (
#if NeedFunctionPrototypes
    FILE *		/* auth_file */,
    IceAuthFileEntry *	/* auth */
#endif
);

extern IceAuthFileEntry *IceGetAuthFileEntry (
#if NeedFunctionPrototypes
    char *		/* protocol_name */,
    char *		/* address */,
    char *		/* auth_name */
#endif
);

extern char *IceGenerateMagicCookie (
#if NeedFunctionPrototypes
    int			/* len */
#endif
);

extern void IceSetPaAuthData (
#if NeedFunctionPrototypes
    int			/* numEntries */,
    IceAuthDataEntry *	/* entries */
#endif
);

#endif /* ICEUTIL_H */
