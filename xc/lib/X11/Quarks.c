/*
 * $XConsortium: Quarks.c,v 1.20 90/06/05 13:55:41 kit Exp $
 */

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

#include "Xlibint.h"
#include <X11/Xresource.h>

extern void bcopy();


typedef unsigned long Signature;

static XrmQuark nextQuark = 1;	/* next available quark number */
static XrmString *quarkToStringTable = NULL;
static int maxQuarks = 0;	/* max names in current quarkToStringTable */
#define QUARKQUANTUM 600;	/* how much to extend quarkToStringTable by */


/* Permanent memory allocation */

#define NEVERFREETABLESIZE 8180
static char *neverFreeTable = NULL;
static int  neverFreeTableSize = 0;

char *Xpermalloc(length)
    unsigned int length;
{
    char *ret;

#ifdef WORD64
    /* round to nearest 8-byte boundary */
    length = (length + 7) & (~7);
#else
    /* round to nearest 4-byte boundary */
    length = (length + 3) & (~3);
#endif /* WORD64 */

    if (neverFreeTableSize < length) {
	neverFreeTableSize =
	    (length > NEVERFREETABLESIZE ? length : NEVERFREETABLESIZE);
	if (! (neverFreeTable = Xmalloc((unsigned) neverFreeTableSize))) {
	    neverFreeTableSize = 0;
	    return (char *) NULL;
	}
    }
    ret = neverFreeTable;
    neverFreeTable += length;
    neverFreeTableSize -= length;
    return(ret);
}


typedef struct _NodeRec *Node;
typedef struct _NodeRec {
    Node 	next;
    Signature	sig;
    XrmQuark	quark;
    XrmString	name;
} NodeRec;

#define HASHTABLESIZE 1021	/* A prime has table size. */
static Node nodeTable[HASHTABLESIZE];

static int XrmAllocMoreQuarkToStringTable()
{
    unsigned	size;
    XrmString *newTable;

    /* Return 1 on success, 0 on failure. */

    maxQuarks += QUARKQUANTUM;
    size = (unsigned) maxQuarks * sizeof(XrmString);

    if (! quarkToStringTable) {
	if (! (quarkToStringTable = (XrmString *) Xmalloc(size))) 
	    return 0;
    }
    else {
	if (! (newTable = (XrmString *) Xrealloc((char *) quarkToStringTable,
						 size))) {
	    maxQuarks -= QUARKQUANTUM;
	    return 0;
	}
	quarkToStringTable = newTable;
    }
    return 1;
}

#if NeedFunctionPrototypes
XrmQuark _XrmInternalStringToQuark(
    register const char *name, register int len, register Signature sig)
#else
XrmQuark _XrmInternalStringToQuark(name, len, sig)
    register XrmString name;
    register int len;
    register Signature sig;
#endif
{
    register char *tname, *tnch;
    register int i;
    register Node	np;
    	     Node	*hashp;

    if (name == NULL)
	return (NULLQUARK);

    /* Look for string in hash table */

    hashp = &nodeTable[sig % HASHTABLESIZE];
    for (np = *hashp; np != NULL; np = np->next) {
	if (np->sig == sig) {	                /* Inline a string compare. */
	    for (i = len, tname = (char *)np->name, tnch = (char *)name; 
		 (--i >= 0) && (*tname++ == *tnch++) ;) {}
	    if (i < 0)		/* all characters matched. */
		return np->quark;
	}
    }
	
    if ((! (np = (Node) Xpermalloc((unsigned int) sizeof(NodeRec)))) ||
	(! (np->name = Xpermalloc((unsigned int) len + 1))))
	return NULLQUARK;
    np->next = *hashp;
    np->sig = sig;

    /*
     * Inline a strncpy(). 
     */

    tname = (char *)name;
    tnch = np->name;
    for (i = len; i != 0; i--, tname++, tnch++)
	*tnch = *tname;
    *tnch = '\0';

    np->quark = nextQuark;
    if ((nextQuark >= maxQuarks) && (! XrmAllocMoreQuarkToStringTable()))
	    return NULLQUARK;

    *hashp = np;
    quarkToStringTable[nextQuark] = np->name;
    ++nextQuark;
    return np->quark;
}

#if NeedFunctionPrototypes
XrmQuark XrmStringToQuark(
    const char *name)
#else
XrmQuark XrmStringToQuark(name)
    XrmString name;
#endif
{
    register char c, *tname;
    register int i = 0;
    register Signature sig = 0;

    if (name == NULL)
	return (NULLQUARK);

    tname = (char *)name;
    for ( ; (c = *tname++) != '\0'; i++)
	sig = (sig << 1) + c;

    return(_XrmInternalStringToQuark(name, i, sig));
}

XrmQuark XrmUniqueQuark()
{
    XrmQuark quark;

    quark = nextQuark;
    if ((nextQuark >= maxQuarks) && (! XrmAllocMoreQuarkToStringTable()))
	    return NULLQUARK;
    quarkToStringTable[nextQuark] = NULLSTRING;
    ++nextQuark;
    return (quark);
}


XrmString XrmQuarkToString(quark)
    XrmQuark quark;
{
    if (quark <= 0 || quark >= nextQuark)
    	return NULLSTRING;
    return quarkToStringTable[quark];
}
