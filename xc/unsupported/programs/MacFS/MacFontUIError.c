/* $XConsortium: MacFontUIError.c,v 1.0 94/01/01 00:00:00 rws Exp $ */
/***********************************************************************
Copyright 1991 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

***********************************************************************/
/*
 * Copyright 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation 
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices or Digital
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 * $NCDId: @(#)error.c,v 4.1 1991/07/08 18:22:51 lemke Exp $
 *
 */
/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#include	<stdio.h>
#include	<X11/Xos.h>
#include	<sys/param.h>

#ifdef USE_SYSLOG
#include	<syslog.h>
#endif

#include	"misc.h"

#include "MacFontUI.h"
extern int initDone;

Bool        UseSyslog;
char        ErrorFile[MAXPATHLEN];

static Bool log_open;

static void
abort_server()
{
	if (initDone)
		EventLoop();
	else {
    	fflush(stderr);
    	abort();
	}
}

void
InitErrors()
{
    int         i;

#ifdef USE_SYSLOG
    if (UseSyslog && !log_open) {
	openlog("Font Server", LOG_PID, LOG_LOCAL0);
	log_open = TRUE;
	return;
    }
#endif

    if (ErrorFile[0]) {
	i = creat(ErrorFile, 0666);
	if (i != -1) {
	    dup2(i, 2);
	    close(i);
	} else {
	    ErrorF("Can't open error file \"%s\"\n", ErrorFile);
	}
    }
}

void
CloseErrors()
{

#ifdef USE_SYSLOG
    if (UseSyslog) {
	closelog();
	log_open = FALSE;
	return;
    }
#endif

    fflush(stderr);
    fclose(stderr);
}

void
Error(str)
    char       *str;
{
	MacFontPerror(str);
}

/*
 * used for informational messages
 */
/* VARARGS1 */
void
NoticeF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9)	/* limit of 10 args */
    char       *f;
    char       *s0,
               *s1,
               *s2,
               *s3,
               *s4,
               *s5,
               *s6,
               *s7,
               *s8,
               *s9;
{

#ifdef USE_SYSLOG
    if (UseSyslog) {
	syslog(LOG_NOTICE, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
	return;
    }
#endif

	char buf[256];

    sprintf(buf, "Notice: ");
    sprintf(buf + strlen("Notice: "),f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
	sprintf(buf + strlen(buf),"\r");

	MacFontLogNotice(buf);
}

/*
 * used for non-fatal error messages
 */
/* VARARGS1 */
void
ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9)	/* limit of 10 args */
    char       *f;
    char       *s0,
               *s1,
               *s2,
               *s3,
               *s4,
               *s5,
               *s6,
               *s7,
               *s8,
               *s9;
{

#ifdef USE_SYSLOG
    if (UseSyslog) {
	syslog(LOG_ERR, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
	return;
    }
#endif

	char buf[256];

    sprintf(buf, "Error: ");
    sprintf(buf + strlen("Error: "), f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
	sprintf(buf + strlen(buf),"\r");

	MacFontLogError(buf);
}

/* VARARGS1 */
void
FatalError(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9)	/* limit of 10 args */
    char       *f;
    char       *s0,
               *s1,
               *s2,
               *s3,
               *s4,
               *s5,
               *s6,
               *s7,
               *s8,
               *s9;
{
    ErrorF("Fatal server error!\n");
    ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
    ErrorF("\n");
	AlertUser(eFatalError);
    abort_server();
    /* NOTREACHED */
}
