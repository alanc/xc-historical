/* $XConsortium: xsm.h,v 1.1 94/02/22 14:35:01 mor Exp $ */
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
******************************************************************************/

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xfuncs.h>
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#include <stdio.h>
#undef _POSIX_SOURCE
#else
#include <stdio.h>
#endif
#include <X11/Shell.h>
#include <X11/SM/SMlib.h>
#include <ctype.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif
#include <setjmp.h>
#include <limits.h>
#include <sys/param.h>
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#include <X11/Xos.h>

/* Fix ISC brain damage.  When using gcc fdopen isn't declared in <stdio.h>. */
#if defined(SYSV) && defined(SYSV386) && defined(__STDC__) && defined(ISC)
extern FILE *fdopen(int, char const *);
#endif

#include "list.h"

#define MAX_PROPS 50

typedef struct _ClientRec {
    SmsConn	 	smsConn;
    IceConn		ice_conn;
    char 		*clientId;
    char		*clientHostname;
    Bool		interactPending;
    int			numProps;
    SmProp *		props[MAX_PROPS];
    struct _ClientRec	*next;
} ClientRec;

typedef struct _PendingClient {
    char		*clientId;
    char		*clientHostname;
    List		*props;
} PendingClient;

typedef struct _PendingProp {
    char		*name;
    char		*type;
    List		*values;
} PendingProp;

typedef struct _PendingValue {
    void		*value;
    int			length;
} PendingValue;


typedef struct _AppResources {
    Boolean	verbose;
    Boolean	debug;
} AppResources;

extern AppResources app_resources;

extern void fprintfhex ();

#ifndef X_NOT_STDC_ENV
#define Strstr strstr
#else
extern char *Strstr();
#endif

extern strbw ();
