/* $XConsortium: extnsionst.h,v 1.12 93/09/24 12:17:34 rws Exp $ */
/***********************************************************
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

******************************************************************/
#ifndef EXTENSIONSTRUCT_H
#define EXTENSIONSTRUCT_H 

#include "misc.h"
#include "screenint.h"
#include "extension.h"
#include "gc.h"

typedef struct _ExtensionEntry {
    int index;
    void (* CloseDown)(	/* called at server shutdown */
#if NeedNestedPrototypes
	struct _ExtensionEntry * /* extension */
#endif
);
    char *name;               /* extension name */
    int base;                 /* base request number */
    int eventBase;            
    int eventLast;
    int errorBase;
    int errorLast;
    int num_aliases;
    char **aliases;
    pointer extPrivate;
    unsigned short (* MinorOpcode)(	/* called for errors */
#if NeedNestedPrototypes
	ClientPtr /* client */
#endif
);
} ExtensionEntry;

/* any attempt to declare the types of the parameters to the functions
 * in EventSwapVector fails.  The functions take pointers to two events,
 * but the exact event types that are declared vary from one function 
 * to another.  You can't even put void *, void * (the ibm compiler
 * complains, anyway).
 */
extern void (* EventSwapVector[128]) ();

typedef void (* ExtensionLookupProc)(/*args indeterminate*/);

typedef struct _ProcEntry {
    char *name;
    ExtensionLookupProc proc;
} ProcEntryRec, *ProcEntryPtr;

typedef struct _ScreenProcEntry {
    int num;
    ProcEntryPtr procList;
} ScreenProcEntry;

#define    SetGCVector(pGC, VectorElement, NewRoutineAddress, Atom)    \
    pGC->VectorElement = NewRoutineAddress;

#define    GetGCValue(pGC, GCElement)    (pGC->GCElement)


extern ExtensionEntry *AddExtension(
#if NeedFunctionPrototypes
    char* /*name*/,
    int /*NumEvents*/,
    int /*NumErrors*/,
    int (* /*MainProc*/)(
#if NeedNestedPrototypes
	ClientPtr /*client*/
#endif
),
    int (* /*SwappedMainProc*/)(
#if NeedNestedPrototypes
	ClientPtr /*client*/
#endif
),
    void (* /*CloseDownProc*/)(
#if NeedNestedPrototypes
	ExtensionEntry * /*extension*/
#endif
),
    unsigned short (* /*MinorOpcodeProc*/)(
#if NeedNestedPrototypes
	ClientPtr /*client*/
#endif
	)
#endif /* NeedFunctionPrototypes */
);

extern Bool AddExtensionAlias(
#if NeedFunctionPrototypes
    char* /*alias*/,
    ExtensionEntry * /*extension*/
#endif
);

extern ExtensionLookupProc LookupProc(
#if NeedFunctionPrototypes
    char* /*name*/,
    GCPtr /*pGC*/
#endif
);

extern Bool RegisterProc(
#if NeedFunctionPrototypes
    char* /*name*/,
    GCPtr /*pGC*/,
    ExtensionLookupProc /*proc*/
#endif
);

extern Bool RegisterScreenProc(
#if NeedFunctionPrototypes
    char* /*name*/,
    ScreenPtr /*pScreen*/,
    ExtensionLookupProc /*proc*/
#endif
);

#endif /* EXTENSIONSTRUCT_H */

