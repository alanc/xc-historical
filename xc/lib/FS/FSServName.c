#include	"copyright.h"
/* @(#)FSServName.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	<stdio.h>

#ifdef X_NOT_STDC_ENV
char       *getenv();
#endif

char       *
FSServerName(server)
    char       *server;
{
    char       *s;

    if (server != NULL && *server != '\0')
	return server;
    if ((s = getenv("FONTSERVER")) != NULL)
	return s;
    return NULL;
}
