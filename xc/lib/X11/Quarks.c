/*
 * $XConsortium: Quarks.c,v 1.22 90/10/07 20:30:22 rws Exp $
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

typedef unsigned long Signature;
typedef unsigned long Entry;

static XrmQuark nextQuark = 1;	/* next available quark number */
static unsigned long quarkMask = 0;
static Entry zero = 0;
static Entry *quarkTable = &zero; /* crock */
static unsigned long quarkRehash;
static XrmString **stringTable = NULL;
static XrmQuark nextUniq = -1;	/* next quark from XrmUniqueQuark */

#define QUANTUMSHIFT	8
#define QUANTUMMASK	((1<<QUANTUMSHIFT)-1)
#define CHUNKPER	8
#define CHUNKMASK	((CHUNKPER<<QUANTUMSHIFT)-1)

#define LARGEQUARK	0x40000000L
#define QUARKSHIFT	16
#define QUARKMASK	((LARGEQUARK-1)>>QUARKSHIFT)
#define SIGMASK		((1L<<QUARKSHIFT)-1)

#define HASH(sig) ((sig) & quarkMask)
#define REHASHVAL(sig) ((((sig) % quarkRehash) + 2) | 1)
#define REHASH(idx,rehash) idx = ((idx + rehash) & quarkMask)
#define NAME(q) stringTable[(q)>>QUANTUMSHIFT][(q)&QUANTUMMASK]

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

static Bool
ExpandQuarkTable()
{
    unsigned long oldmask, newmask;
    register char c, *s;
    register Entry *oldentries, *entries;
    register Entry entry;
    register int oldidx, newidx, rehash;
    Signature sig;
    XrmQuark q;

    oldentries = quarkTable;
    if (oldmask = quarkMask)
	newmask = (oldmask << 1) + 1;
    else {
	stringTable = (XrmString **)Xmalloc(sizeof(XrmString *) * CHUNKPER);
	if (!stringTable)
	    return False;
	stringTable[0] = (XrmString *)Xmalloc(sizeof(XrmString) *
					      (QUANTUMMASK+1));
	if (!stringTable[0])
	    return False;
	newmask = 0x1ff;
    }
    entries = (Entry *)Xmalloc(sizeof(Entry) * (newmask + 1));
    if (!entries)
	return False;
    bzero((char *)entries, sizeof(Entry) * (newmask + 1));
    quarkTable = entries;
    quarkMask = newmask;
    quarkRehash = quarkMask - 2;
    for (oldidx = 0; oldidx <= oldmask; oldidx++) {
	if (entry = oldentries[oldidx]) {
	    if (entry & LARGEQUARK)
		q = entry & (LARGEQUARK-1);
	    else
		q = (entry >> QUARKSHIFT) & QUARKMASK;
	    for (sig = 0, s = NAME(q); c = *s++; )
		sig = (sig << 1) + c;
	    newidx = HASH(sig);
	    if (entries[newidx]) {
		rehash = REHASHVAL(sig);
		do {
		    newidx = REHASH(newidx, rehash);
		} while (entries[newidx]);
	    }
	    entries[newidx] = entry;
	}
    }
    if (oldmask)
	Xfree((char *)oldentries);
    return True;
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
    register XrmQuark q;
    register Entry entry;
    register int idx, rehash;
    register int i;
    register char *s1, *s2;
    XrmString **new;

    if (!len)
	return (NULLQUARK);
    rehash = 0;
    idx = HASH(sig);
    while (entry = quarkTable[idx]) {
	if (entry & LARGEQUARK)
	    q = entry & (LARGEQUARK-1);
	else {
	    if ((entry - sig) & SIGMASK)
		goto nomatch;
	    q = (entry >> QUARKSHIFT) & QUARKMASK;
	}
	for (i = len, s1 = (char *)name, s2 = NAME(q); --i >= 0; ) {
	    if (*s1++ != *s2++)
		goto nomatch;
	}
	if (*s2) {
nomatch:    if (!rehash)
		rehash = REHASHVAL(sig);
	    idx = REHASH(idx, rehash);
	    continue;
	}
	return q;
    }
    if (nextUniq == nextQuark)
	return NULLQUARK;
    if ((nextQuark + (nextQuark >> 2)) > quarkMask) {
	if (!ExpandQuarkTable())
	    return NULLQUARK;
	return _XrmInternalStringToQuark(name, len, sig);
    }
    q = nextQuark;
    if (!(q & QUANTUMMASK)) {
	if (!(q & CHUNKMASK)) {
	    if (!(new = (XrmString **)Xrealloc((char *)stringTable,
					       sizeof(XrmString *) *
					       ((q >> QUANTUMSHIFT) +
						CHUNKPER))))
		return NULLQUARK;
	    stringTable = new;
	}
	if (!(stringTable[q >> QUANTUMSHIFT] =
	      (XrmString *)Xmalloc(sizeof(XrmString) * (QUANTUMMASK+1))))
	    return NULLQUARK;
    }
    s1 = Xpermalloc(len+1);
    if (!s1)
	return NULLQUARK;
    NAME(q) = s1;
    for (i = len, s2 = (char *)name; --i >= 0; )
	*s1++ = *s2++;
    *s1++ = '\0';
    if (q <= QUARKMASK)
	entry = (q << QUARKSHIFT) | (sig & SIGMASK);
    else
	entry = q | LARGEQUARK;
    quarkTable[idx] = entry;
    nextQuark++;
    return q;
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

    if (!name)
	return (NULLQUARK);
    
    for (tname = (char *)name; c = *tname++; i++)
	sig = (sig << 1) + c;

    return _XrmInternalStringToQuark(name, i, sig);
}

XrmQuark XrmUniqueQuark()
{
    if (nextUniq == nextQuark)
	return NULLQUARK;
    return nextUniq--;
}

XrmString XrmQuarkToString(quark)
    XrmQuark quark;
{
    if (quark <= 0 || quark >= nextQuark)
    	return NULLSTRING;
    return NAME(quark);
}
