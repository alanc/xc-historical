/*
 * Xau - X Authorization Database Library
 *
 * $XConsortium: Xauth.h,v 1.8 90/12/12 09:30:27 rws Exp $
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

#ifndef _Xauth_h
#define _Xauth_h

#ifndef NeedFunctionPrototypes
#if defined(FUNCPROTO) || __STDC__ || defined(__cplusplus) || defined(c_plusplus)
#define NeedFunctionPrototypes 1
#else
#define NeedFunctionPrototypes 0
#endif /* __STDC__ */
#endif /* NeedFunctionPrototypes */

#if defined(NeedFunctionPrototypes) && !defined(_Xconst)
#if __STDC__ || defined(__cplusplus) || defined(c_plusplus)
#define _Xconst const
#else
#define _Xconst
#endif
#endif /* NeedFunctionPrototypes */

#ifndef NeedWidePrototypes
#if defined(NARROWPROTO)
#define NeedWidePrototypes 0
#else
#define NeedWidePrototypes 1		/* default to make interropt. easier */
#endif
#endif

# include   <stdio.h>

# define FamilyLocal (256)	/* not part of X standard (i.e. X.h) */
# define FamilyWild  (65535)

#if NeedFunctionPrototypes
typedef struct xauth {
    unsigned short   family;
    unsigned short   address_length;
    _Xconst char    *address;
    unsigned short   number_length;
    _Xconst char    *number;
    unsigned short   name_length;
    _Xconst char    *name;
    unsigned short   data_length;
    _Xconst char    *data;
} Xauth;
#else
typedef struct xauth {
    unsigned short   family;
    unsigned short   address_length;
    char    	    *address;
    unsigned short   number_length;
    char    	    *number;
    unsigned short   name_length;
    char    	    *name;
    unsigned short   data_length;
    char   	    *data;
} Xauth;
#endif

#ifdef __cplusplus			/* do not leave open across includes */
extern "C" {					/* for C++ V2.0 */
#endif

char *XauFileName();

Xauth *XauReadAuth(
#if NeedFunctionPrototypes
FILE*	/* auth_file */
#endif
);

int XauLockAuth(
#if NeedFunctionPrototypes
_Xconst char*	/* file_name */,
int		/* retries */,
int		/* timeout */,
long		/* dead */
#endif
);

int XauUnlockAuth(
#if NeedFunctionPrototypes
_Xconst char*	/* file_name */
#endif
);

int XauWriteAuth(
#if NeedFunctionPrototypes
FILE*		/* auth_file */,
Xauth*		/* auth */
#endif
);

Xauth *XauGetAuthByName(
#if NeedFunctionPrototypes
_Xconst char*	/* display_name */
#endif
);

Xauth *XauGetAuthByAddr(
#if NeedFunctionPrototypes
#if NeedWidePrototypes
unsigned int	/* family */,
unsigned int	/* address_length */,
#else
unsigned short	/* family */,
unsigned short	/* address_length */,
#endif
_Xconst char*	/* address */,
#if NeedWidePrototypes
unsigned int	/* number_length */,
#else
unsigned short	/* number_length */,
#endif
_Xconst char*	/* number */,
#if NeedWidePrototypes
unsigned int	/* name_length */,
#else
unsigned short	/* name_length */,
#endif
_Xconst char*	/* name */
#endif
);

Xauth *XauGetBestAuthByAddr(
#if NeedFunctionPrototypes
#if NeedWidePrototypes
unsigned int	/* family */,
unsigned int	/* address_length */,
#else
unsigned short	/* family */,
unsigned short	/* address_length */,
#endif
_Xconst char*	/* address */,
#if NeedWidePrototypes
unsigned int	/* number_length */,
#else
unsigned short	/* number_length */,
#endif
_Xconst char*	/* number */,
int		/* types_length */,
char**		/* type_names */,
_Xconst int*	/* type_lengths */
#endif
);

void XauDisposeAuth(
#if NeedFunctionPrototypes
Xauth*		/* auth */
#endif
);

#ifdef __cplusplus
}						/* for C++ V2.0 */
#endif

/* Return values from XauLockAuth */

# define LOCK_SUCCESS	0	/* lock succeeded */
# define LOCK_ERROR	1	/* lock unexpectely failed, check errno */
# define LOCK_TIMEOUT	2	/* lock failed, timeouts expired */

#endif /* _Xauth_h */
