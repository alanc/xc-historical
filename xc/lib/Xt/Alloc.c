/* $XConsortium: Alloc.c,v 1.31 90/08/29 13:04:09 swick Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/*
 * X Toolkit Memory Allocation Routines
 *
 * Uses Xlib memory management, which is spec'd to be re-entrant.
 */

#include <X11/Xlib.h>
#include <X11/Xlibos.h>
#include "IntrinsicI.h"

extern void exit();

void _XtAllocError(type)
    String type;
{
    Cardinal num_params = 1;
    if (type == NULL) type = "local memory allocation";
    XtErrorMsg("allocError", type, XtCXtToolkitError,
	       "Cannot perform %s", &type, &num_params);
}

#ifdef XTTRACEMEMORY
#undef XtMalloc
#undef XtRealloc
#undef XtCalloc
#undef XtFree
#endif

char *XtMalloc(size)
    unsigned size;
{
    char *ptr;
    if ((ptr = Xmalloc(size)) == NULL)
        _XtAllocError("malloc");

    return(ptr);
}

char *XtRealloc(ptr, size)
    char     *ptr;
    unsigned size;
{
   if (ptr == NULL) return(XtMalloc(size));
   else if ((ptr = Xrealloc(ptr, size)) == NULL)
	_XtAllocError("realloc");

   return(ptr);
}

char *XtCalloc(num, size)
    unsigned num, size;
{
    char *ptr;

    if ((ptr = Xcalloc(num, size)) == NULL)
	_XtAllocError("calloc");

    return(ptr);
}

void XtFree(ptr)
    char *ptr;
{
   if (ptr != NULL) Xfree(ptr);
}

typedef struct stat {
    struct stat *prev, *next;
    char *file;
    int line;
    unsigned size;
    unsigned long seq;
    XtPointer heap;
} Stats, *StatsPtr;

static StatsPtr XtMemory = (StatsPtr)NULL;
static unsigned long ActiveXtMemory = 0;
static unsigned long XtSeqId = 0;
static unsigned long XtSeqBreakpoint = ~0;

#define StatsSize(n) ((((n) + 3) & ~3) + sizeof(Stats))
#define ToStats(ptr) ((StatsPtr)(ptr - sizeof(Stats)))
#define ToMem(ptr) (((char *)ptr) + sizeof(Stats))

#define CHAIN(ptr,len,hp) \
    ptr->next = XtMemory; \
    if (XtMemory) \
        XtMemory->prev = ptr; \
    XtMemory = ptr; \
    ptr->prev = (StatsPtr)NULL; \
    ptr->file = file; \
    ptr->line = line; \
    ptr->size = len; \
    ptr->heap = hp; \
    ActiveXtMemory += len; \
    ptr->seq = XtSeqId; \
    if (XtSeqId == XtSeqBreakpoint) \
	_XtBreakpoint(ptr); \
    XtSeqId++

/*ARGUSED*/
static void _XtBreakpoint(mem)
    StatsPtr mem;
{
    mem->seq = XtSeqId; /* avoid being optimized out of existence */
}

char *_XtMalloc(size, file, line)
    unsigned size;
    char *file;
    int line;
{
    StatsPtr ptr;
    unsigned newsize;

    newsize = StatsSize(size);
    if ((ptr = (StatsPtr)Xmalloc(newsize)) == NULL)
        _XtAllocError("malloc");
    CHAIN(ptr, size, NULL);
    return(ToMem(ptr));
}

char *_XtRealloc(ptr, size, file, line)
    char     *ptr;
    unsigned size;
    char *file;
    int line;
{
   char *newptr;

   newptr = _XtMalloc(size, file, line);
   if (ptr) {
       bcopy(ptr, newptr, ToStats(ptr)->size);
       _XtFree(ptr);
   }
   return(newptr);
}

char *_XtCalloc(num, size, file, line)
    unsigned num, size;
    char *file;
    int line;
{
    StatsPtr ptr;
    unsigned total, newsize;

    total = num * size;
    newsize = StatsSize(total);
    if ((ptr = (StatsPtr)Xcalloc(1, newsize)) == NULL)
        _XtAllocError("calloc");
    CHAIN(ptr, total, NULL);
    return(ToMem(ptr));
}

void _XtFree(ptr)
    char *ptr;
{
   register StatsPtr stp;

   if (ptr) {
       stp = ToStats(ptr);
       ActiveXtMemory -= stp->size;
       if (stp->prev)
	   stp->prev->next = stp->next;
       else
	   XtMemory = stp->next;
       if (stp->next)
	   stp->next->prev = stp->prev;
       Xfree((char *)stp);
   }
}

char *_XtHeapMalloc(heap, size, file, line)
    XtPointer heap;
    unsigned size;
    char *file;
    int line;
{
    StatsPtr ptr;
    unsigned newsize;

    newsize = StatsSize(size);
    if ((ptr = (StatsPtr)Xmalloc(newsize)) == NULL)
        _XtAllocError("malloc");
    CHAIN(ptr, size, heap);
    return(ToMem(ptr));
}

void _XtHeapFree(heap)
    register XtPointer heap;
{
    register StatsPtr mem, next;

    for (mem = XtMemory; mem; mem = next) {
	next = mem->next;
	if (mem->heap == heap) {
	   ActiveXtMemory -= mem->size;
	   if (mem->prev)
	       mem->prev->next = next;
	   else
	       XtMemory = next;
	   if (next)
	       next->prev = mem->prev;
	   Xfree((char *)mem);
	}
    }
}

#include <stdio.h>

static void _XtPrintMemory(filename)
char * filename;
{
    register StatsPtr mem;
    FILE *f;

    if (filename == NULL)
	f = stderr;
    else 
	f = fopen(filename, "w");
    fprintf(f, "total size: %d\n", ActiveXtMemory);
    for (mem = XtMemory; mem; mem = mem->next) {
	fprintf(f, "size: %6d  seq: %5d  %12s(%4d)  %s\n",
		mem->size, mem->seq,
		mem->file, mem->line, mem->heap ? "heap" : "");
    }
    if (filename) fclose(f);
}
