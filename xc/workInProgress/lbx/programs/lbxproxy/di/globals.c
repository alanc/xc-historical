/************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/* $XConsortium: globals.c,v 1.51 92/03/13 15:40:57 rws Exp $ */

#include "X.h"
#include "Xmd.h"
#include "misc.h"
#include "input.h"
#include "site.h"
#include "dixstruct.h"
#include "os.h"

ClientPtr *clients;
ClientPtr  serverClient;
int  currentMaxClients;   /* current size of clients array */

unsigned long globalSerialNumber = 0;
unsigned long serverGeneration = 0;

/* these next four are initialized in main.c */
long ScreenSaverTime;
long ScreenSaverInterval;
int  ScreenSaverBlanking;
int  ScreenSaverAllowExposures;

ClientPtr requestingClient;	/* XXX this should be obsolete now, remove? */

TimeStamp currentTime;
TimeStamp lastDeviceEventTime;

long TimeOutValue = DEFAULT_TIMEOUT * MILLI_PER_SECOND;

char *defaultDisplayClass = "foo";

Bool	terminateAtReset;

long	*checkForInput[2];

char *display;
