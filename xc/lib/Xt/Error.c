#ifndef lint
static char rcsid[] = "$Header: Error.c,v 1.12 87/11/01 19:34:33 haynes BL5 $";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#ifndef VMS
#include <X11/Xlib.h>
#include <stdio.h>
#else
#include Xlib
#include stdio
#include "VMSutil.h"
#include <descrip.h>
#endif
#include "Intrinsic.h"

static void _XtError (message)
    String message;
{
    extern void exit();
    (void) fprintf(stderr, "X Toolkit Error: %s\n", message);
    exit(1);
}

static void _XtWarning (message)
    String message;
{ (void) fprintf(stderr, "X Toolkit Warning: %s\n", message); }

static void (*errorFunction)() = _XtError;
static vmsbinderror = FALSE;
static void (*warningFunction)() = _XtWarning;
static vmsbindwarning = FALSE;

void XtError(message) 
String message; 
{ 
#ifdef DWTVMS
    $DESCRIPTOR(message$dsc, message); 

    if (vmsbinderror) (*errorFunction)(&message$dsc);
    else (*errorFunction)(message); 
#else
/* how do you construct a descriptor in "C"?
    if (vmsbinderror) (*errorFunction)(&message$dsc);
    else (*errorFunction)(message);
*/
    (*errorFunction)(message); 
#endif

}

void XtWarning(message) 
String message; 
{ 
#ifdef DWTVMS
    $DESCRIPTOR(message$dsc, message); 

    if (vmsbindwarning) (*warningFunction)(&message$dsc);
    else  (*warningFunction)(message);
#else
/*  how do you construct a descriptor in "C"?
    if (vmsbindwarning) (*warningFunction)(&message)
    else (*warningFunction)(message);
*/
    (*warningFunction)(message);
#endif


}


#ifdef DWTVMS
void XT$SETERRORHANDLER(handler) 
    register void (*handler)();
{ 
    if (handler != NULL) {
      errorFunction = handler;
      vmsbinderror = TRUE;
    }
    else {
      errorFunction = _XtError;
      vmsbinderror = FALSE;
    }
}

void XT$SETWARNINGHANDLER(handler) 
    register void (*handler)();
{ 
    if (handler != NULL) {
      warningFunction = handler;
      vmsbindwarning = TRUE;
    }
    else {
      warningFunction = _XtWarning;
      vmsbindwarning = FALSE;
    }
}
#endif

void XtSetErrorHandler(handler)
    register void (*handler)();
{
    if (handler != NULL) errorFunction = handler;
    else errorFunction = _XtError;
    vmsbinderror = FALSE;
}

void XtSetWarningHandler(handler)
    register void (*handler)();
{
    if (handler != NULL) warningFunction = handler;
    else warningFunction = _XtWarning;
    vmsbindwarning = FALSE;
}


