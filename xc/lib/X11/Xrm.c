/*
 * $XConsortium: Xrm.c,v 1.46 90/10/30 10:28:43 rws Exp $
 */

/***********************************************************
Copyright 1987, 1988, 1990 by Digital Equipment Corporation, Maynard,
Massachusetts, and the Massachusetts Institute of Technology, Cambridge,
Massachusetts.

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

#include	"Xlibint.h"
#include	<X11/Xresource.h>
#include	<stdio.h>
#include	<ctype.h>
#include 	"XrmI.h"

#ifdef __STDC__
#define Const const
#else
#define Const /**/
#endif

/*

These Xrm routines allow very fast lookup of resources in the resource
database.  Several usage patterns are exploited:

(1) Widgets get a lot of resources at one time.  Rather than look up each from
scratch, we can precompute the prioritized list of database levels once, then
search for each resource starting at the beginning of the list.

(2) Many database levels don't contain any leaf resource nodes.  There is no
point in looking for resources on a level that doesn't contain any.  This
information is kept on a per-level basis.

(3) Sometimes the widget instance tree is structured such that you get the same
class name repeated on the fully qualified widget name.  This can result in the
same database level occuring multiple times on the search list.  The code below
only checks to see if you get two identical search lists in a row, rather than
look back through all database levels, but in practice this removes all
duplicates I've ever observed.

Joel McCormack

*/

/*

The Xrm representation has been completely redesigned to substantially reduce
memory and hopefully improve performance.

The database is structured into two kinds of tables: LTables that contain
only values, and NTables that contain only other tables.

Some invariants:

The next pointer of the top-level node table points to the top-level leaf
table, if any.

Within an LTable, for a given name, the tight value always precedes the
loose value, and if both are present the loose value is always right after
the tight value.

Within an NTable, all of the entries for a given name are contiguous,
in the order tight NTable, loose NTable, tight LTable, loose LTable.

Bob Scheifler

*/

typedef unsigned long Signature;

extern void bzero();

XrmQuark    XrmQString;

typedef	Bool (*DBEnumProc)();

typedef struct _VEntry {
    struct _VEntry	*next;		/* next in chain */
    XrmQuark		name;		/* name of this entry */
    unsigned int	tight:1;	/* 1 if it is a tight binding */
    unsigned int	string:1;	/* 1 if type is String */
    unsigned int	size:30;	/* size of value */
} VEntryRec, *VEntry;


typedef struct _DEntry {
    VEntryRec		entry;		/* entry */
    XrmRepresentation	type;		/* representation type */
} DEntryRec, *DEntry;

/* the value is right after the structure */
#define StringValue(ve) (caddr_t)((ve) + 1)
#define RepType(ve) ((DEntry)(ve))->type
/* the value is right after the structure */
#define DataValue(ve) (caddr_t)(((DEntry)(ve)) + 1)
#define RawValue(ve) (char *)((ve)->string ? StringValue(ve) : DataValue(ve))

typedef struct _NTable {
    struct _NTable	*next;		/* next in chain */
    XrmQuark		name;		/* name of this entry */
    unsigned int	tight:1;	/* 1 if it is a tight binding */
    unsigned int	leaf:1;		/* 1 if children are values */
    unsigned int	hasloose:1;	/* 1 if has loose children */
    unsigned int	pad:5;		/* unused */
    unsigned int	mask:8;		/* hash size - 1 */
    unsigned int	entries:16;	/* number of children */
} NTableRec, *NTable;

/* the buckets are right after the structure */
#define NodeBuckets(ne) ((NTable *)((ne) + 1))
#define NodeHash(ne,q) NodeBuckets(ne)[(q) & (ne)->mask]

/* leaf tables have an extra level of indirection for the buckets,
 * so that resizing can be done without invalidating a search list.
 * This is completely ugly, and wastes some memory, but the Xlib
 * spec doesn't really specify whether invalidation is OK, and the
 * old implementation did not invalidate.
 */
typedef struct _LTable {
    NTableRec		table;
    VEntry		*buckets;
} LTableRec, *LTable;

#define LeafHash(le,q) (le)->buckets[(q) & (le)->table.mask]

/* An XrmDatabase just holds a pointer to the first top-level table.
 * The type name is no longer descriptive, but better to not change
 * the Xresource.h header file.  This type also gets used to define
 * XrmSearchList, which is a complete crock, but we'll just leave it
 * and caste types as required.
 */
typedef struct _XrmHashBucketRec {
    NTable table;
} XrmHashBucketRec;

/* closure used in get/put resource */
typedef struct _VClosure {
    XrmRepresentation	*type;		/* type of value */
    XrmValuePtr		value;		/* value itself */
} VClosureRec, *VClosure;

/* closure used in get search list */
typedef struct _SClosure {
    LTable		*list;		/* search list */
    int			idx;		/* index of last filled element */
    int			limit;		/* maximum index */
} SClosureRec, *SClosure;

/* placed in XrmSearchList to indicate next table is loose only */
#define LOOSESEARCH ((LTable)1)

/* closure used in enumerate database */
typedef struct _EClosure {
    XrmDatabase db;			/* the database */
    DBEnumProc proc;			/* the user proc */
    caddr_t closure;			/* the user closure */
    XrmBindingList bindings;		/* binding list */
    XrmQuarkList quarks;		/* quark list */
} EClosureRec, *EClosure;

/* predicate to determine when to resize a hash table */
#define GrowthPred(n,m) ((n) > (((m) + 1) << 2))

#define GROW(prev) \
    if (GrowthPred((*prev)->entries, (*prev)->mask)) \
	GrowTable(prev)

/* pick a reasonable value for maximum depth of resource database */
#define MAXDBDEPTH 100

/* resourceQuarks keeps track of what quarks have been associated with values
 * in all LTables.  If a quark has never been used in an LTable, we don't need
 * to bother looking for it.
 */

static unsigned char *resourceQuarks = (unsigned char *)NULL;
static XrmQuark maxResourceQuark = -1;

/* determines if a quark has been used for a value in any database */
#define IsResourceQuark(q)  ((q) > 0 && (q) <= maxResourceQuark && \
			     resourceQuarks[(q) >> 3] & (1 << ((q) & 7)))

/* parsing types */
static XrmBits Const _xrmtypes[256] = {
    _EOF,0,0,0,0,0,0,0,0,SPACE,					/*   0. */
    EOL,0,0,0,0,0,0,0,0,0,					/*  10. */
    0,0,0,0,0,0,0,0,0,0,					/*  20. */
    0,0,SPACE,0,0,0,0,0,0,0,					/*  30. */
    0,0,LOOSE,0,0,0,TIGHT,0,ODIGIT,ODIGIT,
    ODIGIT,ODIGIT,ODIGIT,ODIGIT,ODIGIT,  			/*  50. */
    ODIGIT,0,0,SEP,0,						/*  55. */
    0,0,0,0,0,0,0,0,0,0,					/*  60. */
    0,0,0,0,0,0,0,0,0,0,					/*  70. */
    0,0,0,0,0,0,0,0,0,0,					/*  80. */
    0,0,BSLASH,0,0,0,0,0,0,0				        /*  90. */
    /* The rest will be automatically initialized to zero. */
};

void XrmInitialize()
{
    XrmQString = XrmPermStringToQuark("String");
}

#if NeedFunctionPrototypes
void XrmStringToQuarkList(
    register const char  *name,
    register XrmQuarkList quarks)   /* RETURN */
#else
void XrmStringToQuarkList(name, quarks)
    register char 	 *name;
    register XrmQuarkList quarks;   /* RETURN */
#endif
{
    register XrmBits		bits;
    register Signature  	sig = 0;
    register char       	ch, *tname;
    register int 		i = 0;

    if ((tname = (char *)name)) {
	while (!xrm_is_EOF(bits = get_next_char(ch, tname))) {
	    if (xrm_is_tight_or_loose (bits)) {
		if (i) {
		    /* Found a complete name */
		    *quarks++ = _XrmInternalStringToQuark(name,tname -name -1,
							  sig, False);
		    i = 0;
		    sig = 0;
		}
		name = tname;
	    }
	    else {
		sig = (sig << 1) + ch; /* Compute the signature. */
		i++;
	    }
	}
	*quarks++ = _XrmInternalStringToQuark(name, tname - name - 1, sig,
					      False);
    }
    *quarks = NULLQUARK;
}

#if NeedFunctionPrototypes
void XrmStringToBindingQuarkList(
    register const char	   *name,
    register XrmBindingList bindings,   /* RETURN */
    register XrmQuarkList   quarks)     /* RETURN */
#else
void XrmStringToBindingQuarkList(name, bindings, quarks)
    register char	    *name;
    register XrmBindingList bindings;   /* RETURN */
    register XrmQuarkList   quarks;     /* RETURN */
#endif
{
    register XrmBits		bits;
    register Signature  	sig = 0;
    register char       	ch, *tname;
    register XrmBinding 	binding;
    register int 		i = 0;

    if ((tname = (char *)name)) {
	binding = XrmBindTightly;
	while (!xrm_is_EOF(bits = get_next_char(ch, tname))) {
	    if (xrm_is_tight_or_loose (bits)) {
		if (i) {
		    /* Found a complete name */
		    *bindings++ = binding;
		    *quarks++ = _XrmInternalStringToQuark(name, tname -name -1,
							  sig, False);

		    i = 0;
		    sig = 0;
		    binding = XrmBindTightly;
		}
		name = tname;

		if (xrm_is_loose(bits)) 
		    binding = XrmBindLoosely;
	    }
	    else {
		sig = (sig << 1) + ch; /* Compute the signature. */
		i++;
	    }
	}
	*bindings = binding;
	*quarks++ = _XrmInternalStringToQuark(name, tname - name - 1, sig,
					      False);
    }
    *quarks = NULLQUARK;
}

#ifdef DEBUG

static void PrintQuarkList(quarks, stream)
    XrmQuarkList    quarks;
    FILE	    *stream;
{
    Bool	    firstNameSeen;

    for (firstNameSeen = False; *quarks; quarks++) {
	if (firstNameSeen) {
	    (void) fprintf(stream, ".");
	}
	firstNameSeen = True;
	(void) fputs(XrmQuarkToString(*quarks), stream);
    }
} /* PrintQuarkList */

#endif /* DEBUG */

static XrmDatabase NewDatabase()
{
    register XrmDatabase db;

    db = (XrmDatabase) Xmalloc(sizeof(XrmHashBucketRec));
    if (db)
	db->table = (NTable)NULL;
    return db;
}

/* move all values from ftable to ttable, and free ftable's buckets.
 * ttable is quaranteed empty to start with.
 */
static void MoveValues(ftable, ttable)
    LTable ftable;
    register LTable ttable;
{
    register VEntry fentry, nfentry;
    register VEntry *prev;
    register VEntry *bucket;
    register VEntry tentry;
    register int i;

    for (i = ftable->table.mask, bucket = ftable->buckets; i >= 0; i--) {
	for (fentry = *bucket++; fentry; fentry = nfentry) {
	    prev = &LeafHash(ttable, fentry->name);
	    tentry = *prev;
	    *prev = fentry;
	    /* chain on all with same name, to preserve invariant order */
	    while ((nfentry = fentry->next) && nfentry->name == fentry->name)
		fentry = nfentry;
	    fentry->next = tentry;
	}
    }
    Xfree(ftable->buckets);
}

/* move all tables from ftable to ttable, and free ftable.
 * ttable is quaranteed empty to start with.
 */
static void MoveTables(ftable, ttable)
    NTable ftable;
    register NTable ttable;
{
    register NTable fentry, nfentry;
    register NTable *prev;
    register NTable *bucket;
    register NTable tentry;
    register int i;

    for (i = ftable->mask, bucket = NodeBuckets(ftable); i >= 0; i--) {
	for (fentry = *bucket++; fentry; fentry = nfentry) {
	    prev = &NodeHash(ttable, fentry->name);
	    tentry = *prev;
	    *prev = fentry;
	    /* chain on all with same name, to preserve invariant order */
	    while ((nfentry = fentry->next) && nfentry->name == fentry->name)
		fentry = nfentry;
	    fentry->next = tentry;
	}
    }
    Xfree(ftable);
}

/* grow the table, based on current number of entries */
static void GrowTable(prev)
    NTable *prev;
{
    register NTable table;
    register int i;

    table = *prev;
    i = table->mask;
    if (i == 255) /* biggest it gets */
	return;
    while (i < 255 && GrowthPred(table->entries, i))
	i = (i << 1) + 1;
    i++; /* i is now the new size */
    if (table->leaf) {
	register LTable ltable;
	LTableRec otable;

	ltable = (LTable)table;
	/* cons up a copy to make MoveValues look symmetric */
	otable = *ltable;
	ltable->buckets = (VEntry *)Xmalloc(i * sizeof(VEntry));
	if (!ltable->buckets) {
	    ltable->buckets = otable.buckets;
	    return;
	}
	ltable->table.mask = i - 1;
	bzero((char *)ltable->buckets, i * sizeof(VEntry));
	MoveValues(&otable, ltable);
    } else {
	register NTable ntable;

	ntable = (NTable)Xmalloc(sizeof(NTableRec) + i * sizeof(NTable));
	if (!ntable)
	    return;
	*ntable = *table;
	ntable->mask = i - 1;
	bzero((char *)NodeBuckets(ntable), i * sizeof(NTable));
	*prev = ntable;
	MoveTables(table, ntable);
    }
}

/* merge values from ftable into *pprev, destroy ftable in the process */
static void MergeValues(ftable, pprev, override)
    LTable ftable;
    NTable *pprev;
    Bool override;
{
    register VEntry fentry, tentry;
    register VEntry *prev;
    register LTable ttable;
    VEntry *bucket;
    int i;
    register XrmQuark q;

    ttable = (LTable)*pprev;
    if (ftable->table.hasloose)
	ttable->table.hasloose = 1;
    for (i = ftable->table.mask, bucket = ftable->buckets;
	 i >= 0;
	 i--, bucket++) {
	for (fentry = *bucket; fentry; ) {
	    q = fentry->name;
	    prev = &LeafHash(ttable, q);
	    tentry = *prev;
	    while (tentry && tentry->name != q)
		tentry = *(prev = &tentry->next);
	    /* note: test intentionally uses fentry->name instead of q */
	    /* permits serendipitous inserts */
	    while (tentry && tentry->name == fentry->name) {
		/* if tentry is earlier, skip it */
		if (!fentry->tight && tentry->tight) {
		    tentry = *(prev = &tentry->next);
		    continue;
		}
		if (fentry->tight != tentry->tight) {
		    /* no match, chain in fentry */
		    *prev = fentry;
		    prev = &fentry->next;
		    fentry = *prev;
		    *prev = tentry;
		    ttable->table.entries++;
		} else if (override) {
		    /* match, chain in fentry, splice out and free tentry */
		    *prev = fentry;
		    prev = &fentry->next;
		    fentry = *prev;
		    *prev = tentry->next;
		    /* free the overridden entry */
		    Xfree(tentry);
		    /* get next tentry */
		    tentry = *prev;
		} else {
		    /* match, discard fentry */
		    prev = &tentry->next;
		    tentry = fentry; /* use as a temp var */
		    fentry = fentry->next;
		    /* free the overpowered entry */
		    Xfree(tentry);
		    /* get next tentry */
		    tentry = *prev;
		}
		if (!fentry)
		    break;
	    }
	    /* at this point, tentry cannot match any fentry named q */
	    /* chain in all bindings together, preserve invariant order */
	    while (fentry && fentry->name == q) {
		*prev = fentry;
		prev = &fentry->next;
		fentry = *prev;
		*prev = tentry;
		ttable->table.entries++;
	    }
	}
    }
    Xfree(ftable->buckets);
    Xfree(ftable);
    /* resize if necessary, now that we're all done */
    GROW(pprev);
}

/* merge tables from ftable into *pprev, destroy ftable in the process */
static void MergeTables(ftable, pprev, override)
    NTable ftable;
    NTable *pprev;
    Bool override;
{
    register NTable fentry, tentry;
    NTable nfentry;
    register NTable *prev;
    register NTable ttable;
    NTable *bucket;
    int i;
    register XrmQuark q;

    ttable = *pprev;
    if (ftable->hasloose)
	ttable->hasloose = 1;
    for (i = ftable->mask, bucket = NodeBuckets(ftable);
	 i >= 0;
	 i--, bucket++) {
	for (fentry = *bucket; fentry; ) {
	    q = fentry->name;
	    prev = &NodeHash(ttable, q);
	    tentry = *prev;
	    while (tentry && tentry->name != q)
		tentry = *(prev = &tentry->next);
	    /* note: test intentionally uses fentry->name instead of q */
	    /* permits serendipitous inserts */
	    while (tentry && tentry->name == fentry->name) {
		/* if tentry is earlier, skip it */
		if ((fentry->leaf && !tentry->leaf) ||
		    (!fentry->tight && tentry->tight)) {
		    tentry = *(prev = &tentry->next);
		    continue;
		}
		nfentry = fentry->next;
		if (fentry->leaf != tentry->leaf ||
		    fentry->tight != tentry->tight) {
		    /* no match, just chain in */
		    *prev = fentry;
		    *(prev = &fentry->next) = tentry;
		    ttable->entries++;
		} else {
		    if (fentry->leaf)
			MergeValues((LTable)fentry, prev, override);
		    else
			MergeTables(fentry, prev, override);
		    /* bump to next tentry */
		    tentry = *(prev = &(*prev)->next);
		}
		/* bump to next fentry */
		fentry = nfentry;
		if (!fentry)
		    break;
	    }
	    /* at this point, tentry cannot match any fentry named q */
	    /* chain in all bindings together, preserve invariant order */
	    while (fentry && fentry->name == q) {
		*prev = fentry;
		prev = &fentry->next;
		fentry = *prev;
		*prev = tentry;
		ttable->entries++;
	    }
	}
    }
    Xfree(ftable);
    /* resize if necessary, now that we're all done */
    GROW(pprev);
}

void XrmCombineDatabase(from, into, override)
    XrmDatabase	from, *into;
    Bool override;
{
    register NTable *prev;
    register NTable ftable, ttable, nftable;

    if (!*into) {
	*into = from;
    } else if (from) {
	if (ftable = from->table) {
	    prev = &(*into)->table;
	    ttable = *prev;
	    if (!ftable->leaf) {
		nftable = ftable->next;
		if (ttable && !ttable->leaf) {
		    /* both have node tables, merge them */
		    MergeTables(ftable, prev, override);
		    /* bump to into's leaf table, if any */
		    ttable = *(prev = &(*prev)->next);
		} else {
		    /* into has no node table, link from's in */
		    *prev = ftable;
		    *(prev = &ftable->next) = ttable;
		}
		/* bump to from's leaf table, if any */
		ftable = nftable;
	    } else {
		/* bump to into's leaf table, if any */
		if (ttable && !ttable->leaf)
		    ttable = *(prev = &ttable->next);
	    }
	    if (ftable) {
		/* if into has a leaf, merge, else insert */
		if (ttable)
		    MergeValues((LTable)ftable, prev, override);
		else
		    *prev = ftable;
	    }
	}
	Xfree(from);
    }
}

void XrmMergeDatabases(from, into)
    XrmDatabase	from, *into;
{
    XrmCombineDatabase(from, into, True);
}

/* store a value in the database, overriding any existing entry */
static void PutEntry(db, bindings, quarks, type, value)
    XrmDatabase		db;
    XrmBindingList	bindings;
    XrmQuarkList	quarks;
    XrmRepresentation	type;
    XrmValuePtr		value;
{
    register NTable *pprev, *prev;
    register NTable table;
    register XrmQuark q;
    register VEntry *vprev;
    register VEntry entry;
    NTable *nprev, *firstpprev;

#define NEWTABLE(q) \
    table = (NTable)Xmalloc(sizeof(LTableRec)); \
    if (!table) \
	return; \
    table->name = q; \
    table->hasloose = 0; \
    table->mask = 0; \
    table->entries = 0; \
    if (quarks[1]) { \
	table->leaf = 0; \
	nprev = NodeBuckets(table); \
    } else { \
	table->leaf = 1; \
	if (!(nprev = (NTable *)Xmalloc(sizeof(VEntry *)))) \
	    return; \
	((LTable)table)->buckets = (VEntry *)nprev; \
    } \
    *nprev = (NTable)NULL; \
    table->next = *prev; \
    *prev = table

    if (!db || !*quarks)
	return;
    table = *(prev = &db->table);
    /* if already at leaf, bump to the leaf table */
    if (!quarks[1] && table && !table->leaf)
	table = *(prev = &table->next);
    pprev = prev;
    if (!table || (quarks[1] && table->leaf)) {
	/* no top-level node table, create one and chain it in */
	NEWTABLE(NULLQUARK);
	table->tight = 1; /* arbitrary */
	prev = nprev;
    } else {
	/* search along until we need a value */
	while (quarks[1]) {
	    q = *quarks;
	    table = *(prev = &NodeHash(table, q));
	    while (table && table->name != q)
		table = *(prev = &table->next);
	    if (!table)
		break; /* not found */
	    if (quarks[2]) {
		if (table->leaf)
		    break; /* not found */
	    } else {
		if (!table->leaf) {
		    /* bump to leaf table, if any */
		    table = *(prev = &table->next);
		    if (!table || table->name != q)
			break; /* not found */
		    if (!table->leaf) {
			/* bump to leaf table, if any */
			table = *(prev = &table->next);
			if (!table || table->name != q)
			    break; /* not found */
		    }
		}
	    }
	    if (*bindings == XrmBindTightly) {
		if (!table->tight)
		    break; /* not found */
	    } else {
		if (table->tight) {
		    /* bump to loose table, if any */
		    table = *(prev = &table->next);
		    if (!table || table->name != q ||
			!quarks[2] != table->leaf)
			break; /* not found */
		}
	    }
	    /* found that one, bump to next quark */
	    pprev = prev;
	    quarks++;
	    bindings++;
	}
	if (!quarks[1]) {
	    /* found all the way to a leaf */
	    q = *quarks;
	    entry = *(vprev = &LeafHash((LTable)table, q));
	    while (entry && entry->name != q)
		entry = *(vprev = &entry->next);
	    /* if want loose and have tight, bump to next entry */
	    if (entry && *bindings == XrmBindLoosely && entry->tight)
		entry = *(vprev = &entry->next);
	    if (entry && entry->name == q &&
		(*bindings == XrmBindTightly) == entry->tight) {
		/* match, need to override */
		if ((type == XrmQString) == entry->string &&
		    entry->size == value->size) {
		    /* identical size and type, just overwrite */
		    bcopy((char *)value->addr, RawValue(entry), value->size);
		    return;
		}
		/* splice out and free old entry */
		*vprev = entry->next;
		Xfree(entry);
		(*pprev)->entries--;
	    }
	    /* this is where to insert */
	    prev = (NTable *)vprev;
	}
    }
    /* keep the top table, because we may have to grow it */
    firstpprev = pprev;
    /* iterate until we get to the leaf */
    while (quarks[1]) {
	/* build a new table and chain it in */
	NEWTABLE(*quarks++);
	if (*bindings++ == XrmBindTightly) {
	    table->tight = 1;
	} else {
	    table->tight = 0;
	    (*pprev)->hasloose = 1;
	}
	(*pprev)->entries++;
	pprev = prev;
	prev = nprev;
    }
    /* now allocate the value entry */
    entry = (VEntry)Xmalloc(((type == XrmQString) ?
			     sizeof(VEntryRec) : sizeof(DEntryRec)) +
			    value->size);
    if (!entry)
	return;
    entry->name = q = *quarks;
    if (*bindings == XrmBindTightly) {
	entry->tight = 1;
    } else {
	entry->tight = 0;
	(*pprev)->hasloose = 1;
    }
    /* chain it in, with a bit of type cast ugliness */
    entry->next = *((VEntry *)prev);
    *((VEntry *)prev) = entry;
    entry->size = value->size;
    if (type == XrmQString) {
	entry->string = 1;
    } else {
	entry->string = 0;
	RepType(entry) = type;
    }
    /* save a copy of the value */
    bcopy((char *)value->addr, RawValue(entry), value->size);
    (*pprev)->entries++;
    /* this is a new leaf, need to remember it for search lists */
    if (q > maxResourceQuark) {
	unsigned oldsize = maxResourceQuark + 1;
	unsigned size = (q | 0x7f) + 1; /* reallocate in reasonable chunks */
	if (resourceQuarks) 
	    resourceQuarks = (unsigned char *)Xrealloc((char *)resourceQuarks,
						       size);
	else
	    resourceQuarks = (unsigned char *)Xmalloc(size);
	if (resourceQuarks) {
	    bzero((char *)&resourceQuarks[oldsize], size - oldsize);
	    maxResourceQuark = size - 1;
	} else {
	    maxResourceQuark = -1;
	}
    }
    if (q > 0 && resourceQuarks)
	resourceQuarks[q >> 3] |= 1 << (q & 0x7);
    GROW(firstpprev);

#undef NEWTABLE
}

void XrmQPutResource(pdb, bindings, quarks, type, value)
    XrmDatabase		*pdb;
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    XrmRepresentation	type;
    XrmValuePtr		value;
{
    if (!*pdb) *pdb = NewDatabase();
    PutEntry(*pdb, bindings, quarks, type, value);
}

#if NeedFunctionPrototypes
void XrmPutResource(
    XrmDatabase     *pdb,
    const char	    *specifier,
    const char	    *type,
    XrmValuePtr	    value)
#else
void XrmPutResource(pdb, specifier, type, value)
    XrmDatabase     *pdb;
    char	    *specifier;
    char	    *type;
    XrmValuePtr	    value;
#endif
{
    XrmBinding	    bindings[MAXDBDEPTH+1];
    XrmQuark	    quarks[MAXDBDEPTH+1];

    if (!*pdb) *pdb = NewDatabase();
    XrmStringToBindingQuarkList(specifier, bindings, quarks);
    PutEntry(*pdb, bindings, quarks, XrmStringToQuark(type), value);
}

#if NeedFunctionPrototypes
void XrmQPutStringResource(
    XrmDatabase     *pdb,
    XrmBindingList  bindings,
    XrmQuarkList    quarks,
    const char	    *str)
#else
void XrmQPutStringResource(pdb, bindings, quarks, str)
    XrmDatabase     *pdb;
    XrmBindingList  bindings;
    XrmQuarkList    quarks;
    char	    *str;
#endif
{
    XrmValue	value;

    if (!*pdb) *pdb = NewDatabase();
    value.addr = (caddr_t) str;
    value.size = strlen(str)+1;
    PutEntry(*pdb, bindings, quarks, XrmQString, &value);
}

/*	Function Name: GetDatabase
 *	Description: Parses a string and stores it as a database.
 *	Arguments: db - the database.
 *                 str - a pointer to the string containing the database.
 *                 filename - source filename, if any.
 *	Returns: 0 if failure, 1 if totally sucessful.
 */

/*
 * This function is highly optimized to inline as much as possible. 
 * Be very careful with modifications, or simplifications, as they 
 * may adversely affect the performance.
 *
 * Chris Peterson, MIT X Consortium		5/17/90.
 */

#define LIST_SIZE 101
#define BUFFER_SIZE 100

static int GetDatabase(db, str, filename)
    XrmDatabase db;
    register char *str;
    char *filename;
{
    register char *ptr;
    register XrmBits bits = 0;
    register char c;
    register Signature sig;
    register char *ptr_max;
    register XrmQuarkList t_quarks;
    register XrmBindingList t_bindings;

    int alloc_chars = BUFSIZ, return_value = 1;
    char buffer[BUFSIZ], *value_str;
    XrmQuark quarks[LIST_SIZE];
    XrmBinding bindings[LIST_SIZE];
    XrmValue value;
    static void GetIncludeFile();

    if (!db)
	return 0;

    if (!(value_str = Xmalloc(sizeof(char) * alloc_chars)))
	return 0;

    while (!xrm_is_EOF(bits)) {

	/*
	 * First: Remove extra whitespace. 
	 */

	do {
	    bits = get_next_char(c, str);
	} while xrm_is_space(bits);

	/*
	 * Ignore empty lines.
	 */

	if (xrm_is_EOL(bits))
	    continue;		/* start a new line. */

	/*
	 * Second: check the first character in a line to see if it is
	 * "!" signifying a comment, or "#" signifying a directive.
	 */

	if (c == '!') { /* Comment, spin to next newline */
	    while (!xrm_is_EOL(bits = get_next_char(c, str))) {}
	    continue;		/* start a new line. */
	}

	if (c == '#') { /* Directive */
	    /* remove extra whitespace */
	    while (xrm_is_space(bits = get_next_char(c, str))) {};
	    /* only "include" directive is currently defined */
	    if (!strncmp(str-1, "include", 7)) {
		str += (7-1);
		/* remove extra whitespace */
		while (xrm_is_space(bits = get_next_char(c, str))) {};
		/* must have a starting " */
		if (c == '"') {
		    char *fname = str;
		    do {
			bits = get_next_char(c, str);
		    } while (c != '"' && !xrm_is_EOL(bits));
		    /* must have an ending " */
		    if (c == '"')
			GetIncludeFile(db, filename, fname, str - fname - 1);
		}
	    }
	    /* spin to next newline */
	    if (xrm_is_EOL(bits))
		continue;
	    while (!xrm_is_EOL(bits = get_next_char(c, str))) {}
	    continue;		/* start a new line. */
	}

	/*
	 * Third: loop through the LHS of the resource specification
	 * storing characters and converting this to a Quark.
	 *
	 * If the number of quarks is greater than LIST_SIZE - 1.  This
	 * function will trash your memory.
	 *
	 * If the length of any quark is larger than BUFSIZ this function
	 * will also trash memory.
	 */
	
	t_bindings = bindings;
	t_quarks = quarks;

	sig = 0;
	ptr = buffer;
	*t_bindings = XrmBindTightly;	
	for(;;) {
	    if (!xrm_is_tight_or_loose(bits)) {
		while (!xrm_is_end_of_quark(bits)) {
		    *ptr++ = c;
		    sig = (sig << 1) + c; /* Compute the signature. */
		    bits = get_next_char(c, str);
		}

		*t_quarks++ = _XrmInternalStringToQuark(buffer, ptr - buffer,
							sig, False);
	    
		if (xrm_is_separator(bits))  {
		    if (!xrm_is_space(bits))
			break;

		    /* Remove white space */
		    do {
			*ptr++ = c;
			sig = (sig << 1) + c; /* Compute the signature. */
		    } while (xrm_is_space(bits = get_next_char(c, str)));

		    /* 
		     * The spec doesn't permit it, but support spaces
		     * internal to resource name/class 
		     */

		    if (xrm_is_separator(bits))
			break;
		    t_quarks--;
		    continue;
		}

		if (xrm_is_tight(bits))
		    *(++t_bindings) = XrmBindTightly;
		else
		    *(++t_bindings) = XrmBindLoosely;

		sig = 0;
		ptr = buffer;
	    }
	    else {
		/*
		 * Magic unspecified feature #254.
		 *
		 * If two separators appear with no Text between them then
		 * ignore them.
		 *
		 * If anyone of those separators is a '*' then the binding 
		 * will be loose, otherwise it will be tight.
		 */

		if (xrm_is_loose(bits))
		    *t_bindings = XrmBindLoosely;
	    }

	    bits = get_next_char(c, str);
	} 

	*t_quarks = NULLQUARK;

	/*
	 * Make sure that there is a ':' in this line.
	 */

	if (!xrm_is_real_separator(bits)) {
	    XrmBits old_bits;

	    if (xrm_is_EOL(bits)) 
		continue;

	    /*
	     * A parsing error has occured, toss everything on the line
	     * a new_line can still be escaped with a '\'.
	     */

	    return_value = 0;
	    do {
		old_bits = bits;
		bits = get_next_char(c, str);
	    } while (!(xrm_is_EOF(bits) ||
		       (xrm_is_EOL(bits) && !xrm_is_backslash(old_bits))));
	    continue;
	}

	/*
	 * I now have a quark and binding list for the entire left hand
	 * side.  "c" currently points to the ":" separating the left hand
	 * side for the right hand side.  It is time to begin processing
	 * the right hand side.
	 */

	/* 
	 * Fourth: Remove more whitespace
	 */

	for(;;) {
	    XrmBits old_bits;

	    if (xrm_is_space(bits = get_next_char(c, str)))
		continue;
	    if (!xrm_is_backslash(bits))
		break;
	    old_bits = bits;
	    bits = get_next_char(c, str);
	    if (xrm_is_EOL(bits) && !xrm_is_EOF(bits))
		continue;
	    str--;
	    bits = old_bits;
	    break;
	}

	/* 
	 * Fifth: Process the right hand side.
	 */

	ptr = value_str;
	ptr_max = ptr + alloc_chars - 4;

	for(;;) {

	    /*
	     * Tight loop for the normal case:  Non backslash, non-end of value
	     * character that will fit into the allocated buffer.
	     */

	    while (!(xrm_is_backslash_or_EOV(bits) ||
		     (ptr >= ptr_max)) ) {
		*ptr++ = c;
		bits = get_next_char(c, str);
	    }

	    if (xrm_is_end_of_value(bits))
		break;

	    if (xrm_is_backslash(bits)) {
		char temp[3];
		int count;

		/*
		 * We need to do some magic after a backslash.
		 */

		bits = get_next_char(c, str); 

		if (xrm_is_EOL(bits)) 
		    if (xrm_is_EOF(bits)) 
			goto done;
		    else {
			bits = get_next_char(c, str);
			continue;
		    }

		/*
		 * "\n" means insert a newline.
		 */
		  
		if (c == 'n') {
		    *ptr++ = A_NEW_LINE;
		    bits = get_next_char(c, str);
		    continue;
		}

		/*
		 * "\\" completes to just one backslash.
		 */

		if (xrm_is_backslash(bits)) {
		    *ptr++ = c;	                /* we know that c == '\'. */
		    bits = get_next_char(c, str);
		    continue;
		}

		/*
		 * pick up to three octal digits after the '\'.
		 */
		
		count = 0;
		while (xrm_is_odigit(bits) && count < 3) {
		    temp[count] = c;
		    bits = get_next_char(c, str);
		    count++;
		}
		
		/*
		 * If we found three digits then insert that octal code into
		 * into the value string as a character.
		 */
		
		if (count == 3) {
		    *ptr++ = (unsigned char) ((temp[0] - ZERO) * 0100 +
					      (temp[1] - ZERO) * 010 +
					      (temp[2] - ZERO));
		}
		else {
		    int tcount;

		    /* 
		     * Otherwise just insert those characters into the 
		     * string, since no special processing is needed on
		     * numerics we can skip the special processing.
		     */

		    for (tcount = 0; tcount < count; tcount++) {
			*ptr++ = temp[tcount]; /* print them in 
						  the correct order */
		    }
		}
	    }
	    
	    /* 
	     * It is important to make sure that there is room for at least
	     * four more characters in the buffer, since I can add that
	     * many characters into the buffer after a backslash has occured.
	     */

	    if (ptr >= ptr_max) {
		char * temp_str;

		alloc_chars += BUFSIZ/10;		
		temp_str = Xrealloc(value_str, sizeof(char) * alloc_chars);

		if (!value_str) {
		    return_value = 0;
		    goto done;
		}

		ptr = temp_str + (ptr - value_str); /* reset pointer. */
		value_str = temp_str;
		ptr_max = value_str + alloc_chars - 4;
	    }
	}

	/*
	 * Lastly: Terminate the value string, and store this entry 
	 * 	   into the database.
	 */

	*ptr++ = STRING_TERMINATOR;

	/* Store it in database */
	value.size = ptr - value_str;
	value.addr = (caddr_t) value_str;
	
	PutEntry(db, bindings, quarks, XrmQString, &value);
    }

 done:

    Xfree(value_str);
    return return_value;
}

#if NeedFunctionPrototypes
void XrmPutStringResource(
    XrmDatabase *pdb,
    const char	*specifier,
    const char	*str)
#else
void XrmPutStringResource(pdb, specifier, str)
    XrmDatabase *pdb;
    char	*specifier;
    char	*str;
#endif
{
    XrmValue	value;
    XrmBinding	bindings[MAXDBDEPTH+1];
    XrmQuark	quarks[MAXDBDEPTH+1];

    if (!*pdb) *pdb = NewDatabase();
    XrmStringToBindingQuarkList(specifier, bindings, quarks);
    value.addr = (caddr_t) str;
    value.size = strlen(str)+1;
    PutEntry(*pdb, bindings, quarks, XrmQString, &value);
}


#if NeedFunctionPrototypes
void XrmPutLineResource(
    XrmDatabase *pdb,
    const char	*line)
#else
void XrmPutLineResource(pdb, line)
    XrmDatabase *pdb;
    char	*line;
#endif
{
    if (!*pdb) *pdb = NewDatabase();
    (void) GetDatabase(*pdb, line, (char *)NULL);
} 

#if NeedFunctionPrototypes
XrmDatabase XrmGetStringDatabase(
    const char	    *data)
#else
XrmDatabase XrmGetStringDatabase(data)
    char	    *data;
#endif
{
    XrmDatabase     db;

    db = NewDatabase();
    (void) GetDatabase(db, data, (char *)NULL);
    return db;
}

/*	Function Name: ReadInFile
 *	Description: Reads the file into a buffer.
 *	Arguments: filename - the name of the file.
 *	Returns: An allocated string containing the contents of the file.
 */

static char * 
ReadInFile(filename)
char * filename;
{
    register int fd, size;
    char * filebuf;

    if ( (fd = OpenFile(filename)) == -1 ) 
	return (char *)NULL;

    GetSizeOfFile(filename, size);
	
    if (!(filebuf = Xmalloc(size + 1))) { /* leave room for '\0' */
	close(fd);
	return (char *)NULL;
    }

    if (ReadFile(fd, filebuf, size) != size) { /* If we didn't read the
						  correct number of bytes. */
	CloseFile(fd);
	Xfree(filebuf);
	return (char *)NULL;
    }
    CloseFile(fd);

    filebuf[size] = '\0';	/* NULL terminate it. */
    return filebuf;
}

static void
GetIncludeFile(db, base, fname, fnamelen)
    XrmDatabase db;
    char *base;
    char *fname;
    int fnamelen;
{
    int len;
    char *str;
    char realfname[BUFSIZ];

    if (fnamelen <= 0 || fnamelen >= BUFSIZ)
	return;
    if (*fname != '/' && base && (str = rindex(base, '/'))) {
	len = str - base + 1;
	if (len + fnamelen >= BUFSIZ)
	    return;
	strncpy(realfname, base, len);
	strncpy(realfname + len, fname, fnamelen);
	realfname[len + fnamelen] = '\0';
    } else {
	strncpy(realfname, fname, fnamelen);
	realfname[fnamelen] = '\0';
    }
    if (!(str = ReadInFile(realfname)))
	return;
    (void) GetDatabase(db, str, realfname);
    Xfree(str);
}

#if NeedFunctionPrototypes
XrmDatabase XrmGetFileDatabase(
    const char 	    *filename)
#else
XrmDatabase XrmGetFileDatabase(filename)
    char 	    *filename;
#endif
{
    XrmDatabase db;
    char *str;

    if (!(str = ReadInFile(filename)))
	return (XrmDatabase)NULL;

    db = NewDatabase();
    (void) GetDatabase(db, str, filename);
    Xfree(str);
    return db;
}

#if NeedFunctionPrototypes
void XrmCombineFileDatabase(
    const char 	    *filename,
    XrmDatabase     *target,
    Bool             override)
#else
void XrmCombineFileDatabase(filename, target, override)
    char        *filename;
    XrmDatabase *target;
    Bool         override;
#endif
{
    char *str;

    if (!override) {
	XrmCombineDatabase(XrmGetFileDatabase(filename), target, False);
	return;
    }
    if (!(str = ReadInFile(filename)))
	return;
    if (!*target)
	*target = NewDatabase();
    (void) GetDatabase(*target, str, filename);
    Xfree(str);
}

/* call the user proc for every value in the table, arbitrary order.
 * stop if user proc returns True.  level is current depth in database.
 */
static Bool EnumValues(table, level, closure)
    LTable		table;
    register int	level;
    register EClosure	closure;
{
    register VEntry *bucket;
    register int i;
    register VEntry entry;
    XrmValue value;
    XrmRepresentation type;

    if (level >= MAXDBDEPTH)
	return False;
    closure->quarks[level + 1] = NULLQUARK;
    for (i = table->table.mask, bucket = table->buckets;
	 i >= 0;
	 i--, bucket++) {
	for (entry = *bucket; entry; entry = entry->next) {
	    closure->bindings[level] = (entry->tight ?
					XrmBindTightly : XrmBindLoosely);
	    closure->quarks[level] = entry->name;
	    value.size = entry->size;
	    if (entry->string) {
		type = XrmQString;
		value.addr = StringValue(entry);
	    } else {
		type = RepType(entry);
		value.addr = DataValue(entry);
	    }
	    if ((*closure->proc)(&closure->db, closure->bindings,
				 closure->quarks, &type, &value,
				 closure->closure))
		return True;
	}
    }
    return False;
}

/* recurse on every table in the table, arbitrary order.
 * stop if user proc returns True.  level is current depth in database.
 */
static Bool EnumTables(table, level, closure)
    NTable		table;
    register int	level;
    register EClosure	closure;
{
    register NTable *bucket;
    register int i;
    register NTable entry;

    if (level >= MAXDBDEPTH)
	return False;
    for (i = table->mask, bucket = NodeBuckets(table);
	 i >= 0;
	 i--, bucket++) {
	for (entry = *bucket; entry; entry = entry->next) {
	    closure->bindings[level] = (entry->tight ?
					XrmBindTightly : XrmBindLoosely);
	    closure->quarks[level] = entry->name;
	    if (entry->leaf) {
		if (EnumValues((LTable)entry, level + 1, closure))
		    return True;
	    } else {
		if (EnumTables(entry, level + 1, closure))
		    return True;
	    }
	}
    }
    return False;
}

/* call the proc for every value in the database, arbitrary order.
 * stop if the proc returns True.
 */
static Bool EnumerateDatabase(db, proc, closure)
    XrmDatabase db;
    caddr_t     closure;
    DBEnumProc	proc;
{
    XrmBinding  bindings[MAXDBDEPTH+1];
    XrmQuark	quarks[MAXDBDEPTH+1];
    register NTable table;
    EClosureRec	eclosure;
   
    eclosure.db = db;
    eclosure.proc = proc;
    eclosure.closure = closure;
    eclosure.bindings = bindings;
    eclosure.quarks = quarks;
    table = db->table;
    if (table && !table->leaf) {
	if (EnumTables(table, 0, &eclosure))
	    return True;
	table = table->next;
    }
    if (table)
	return EnumValues((LTable)table, 0, &eclosure);
    return False;
}

static void PrintBindingQuarkList(bindings, quarks, stream)
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    FILE		*stream;
{
    Bool	firstNameSeen;

    for (firstNameSeen = False; *quarks; bindings++, quarks++) {
	if (*bindings == XrmBindLoosely) {
	    (void) fprintf(stream, "*");
	} else if (firstNameSeen) {
	    (void) fprintf(stream, ".");
	}
	firstNameSeen = True;
	(void) fputs(XrmQuarkToString(*quarks), stream);
    }
}

/* output out the entry in correct file syntax */
/*ARGSUSED*/
static Bool DumpEntry(db, bindings, quarks, type, value, stream)
    XrmDatabase		*db;
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    XrmRepresentation   *type;
    XrmValuePtr		value;
    FILE		*stream;
{

    register unsigned int	i;
    register char		*s;
    register char		c;

    if (*type != XrmQString)
	(void) putc('!', stream);
    PrintBindingQuarkList(bindings, quarks, stream);
    s = value->addr;
    i = value->size;
    if (*type == XrmQString) {
	(void) fputs(":\t", stream);
	if (i)
	    i--;
    }
    else
	fprintf(stream, "=%s:\t", XrmRepresentationToString(*type));
    while (i--) {
	c = *s++;
	if (c == '\n') {
	    if (i)
		(void) fputs("\\n\\\n", stream);
	    else
		(void) fputs("\\n", stream);
	} else if (c == '\\')
	    (void) fputs("\\\\", stream);
	else if ((c < ' ' && c != '\t') ||
		 ((unsigned char)c >= 0x7f && (unsigned char)c < 0xa0))
	    (void) fprintf(stream, "\\%03o", (unsigned char)c);
	else
	    (void) putc(c, stream);
    }
    (void) putc('\n', stream);
    return False;
}

#ifdef DEBUG

void PrintTable(table, file)
    NTable table;
    FILE *file;
{
    XrmBinding  bindings[MAXDBDEPTH+1];
    XrmQuark	quarks[MAXDBDEPTH+1];
    EClosureRec closure;

    closure.db = (XrmDatabase)NULL;
    closure.proc = DumpEntry;
    closure.closure = (caddr_t)file;
    closure.bindings = bindings;
    closure.quarks = quarks;
    if (table->leaf)
	EnumValues((LTable)table, 0, &closure);
    else
	EnumTables(table, 0, &closure);
}

#endif /* DEBUG */

#if NeedFunctionPrototypes
void XrmPutFileDatabase(
    XrmDatabase db,
    const char 	*fileName)
#else
void XrmPutFileDatabase(db, fileName)
    XrmDatabase db;
    char 	*fileName;
#endif
{
    FILE	*file;
    
    if (!db) return;
    if (!(file = fopen(fileName, "w"))) return;
    (void)EnumerateDatabase(db, DumpEntry, (caddr_t) file);
    fclose(file);
}

/* macros used in get/search functions */

/* find an entry named *list, with leafness given by leaf */
#define NFIND(list) \
    q = *list; \
    entry = NodeHash(table, q); \
    while (entry && entry->name != q) \
	entry = entry->next; \
    if (leaf && entry && !entry->leaf) { \
	entry = entry->next; \
	if (entry && !entry->leaf) \
	    entry = entry->next; \
	if (entry && entry->name != q) \
	    entry = (NTable)NULL; \
    }

/* find entries named *list, leafness leaf, tight or loose, and call get */
#define GTIGHTLOOSE(list,looseleaf) \
    NFIND(list); \
    if (entry) { \
	if (leaf == entry->leaf) { \
	    if ((*get)(entry, names+1, classes+1, closure)) \
		return True; \
	    if (entry->tight) { \
		entry = entry->next; \
		if (entry && entry->name == q && leaf == entry->leaf) { \
		    if ((*get)(entry, names+1, classes+1, closure)) \
			return True; \
		} \
	    } \
	} else if (entry->leaf && entry->hasloose) { \
	    if (looseleaf((LTable)entry, names+1, classes+1, closure)) \
		return True; \
	} \
    }

/* find entries named *list, leafness leaf, loose only, and call get */
#define GLOOSE(list,looseleaf) \
    NFIND(list); \
    if (entry && entry->tight) { \
	entry = entry->next; \
	if (entry && entry->name != q) \
	    entry = (NTable)NULL; \
    } \
    if (entry) { \
	if (leaf == entry->leaf) { \
	    if ((*get)(entry, names+1, classes+1, closure)) \
		return True; \
	} else if (entry->leaf && entry->hasloose) { \
	    if (looseleaf((LTable)entry, names+1, classes+1, closure)) \
		return True; \
	} \
    }

/* add tight/loose entry to the search list, return True if list is full */
/*ARGSUSED*/
static Bool AppendLEntry(table, names, classes, closure)
    LTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    register SClosure	closure;
{
    /* check for duplicate */
    if (closure->idx >= 0 && closure->list[closure->idx] == table)
	return False;
    if (closure->idx == closure->limit)
	return True;
    /* append it */
    closure->idx++;
    closure->list[closure->idx] = table;
    return False;
}

/* add loose entry to the search list, return True if list is full */
/*ARGSUSED*/
static Bool AppendLooseLEntry(table, names, classes, closure)
    LTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    register SClosure	closure;
{
    /* check for duplicate */
    if (closure->idx >= 0 && closure->list[closure->idx] == table)
	return False;
    if (closure->idx >= closure->limit - 1)
	return True;
    /* append it */
    closure->idx++;
    closure->list[closure->idx] = LOOSESEARCH;
    closure->idx++;
    closure->list[closure->idx] = table;
    return False;
}

/* search for a leaf table */
static Bool SearchNEntry(table, names, classes, closure)
    NTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    SClosure		closure;
{
    register NTable	entry;
    register XrmQuark	q;
    register unsigned int leaf;
    Bool		(*get)();

    if (names[1]) {
	get = SearchNEntry; /* recurse */
	leaf = 0;
    } else {
	get = AppendLEntry; /* bottom of recursion */
	leaf = 1;
    }
    GTIGHTLOOSE(names, AppendLooseLEntry);   /* do name, tight and loose */
    GTIGHTLOOSE(classes, AppendLooseLEntry); /* do class, tight and loose */
    if (table->hasloose) {
	while (1) {
	    names++;
	    classes++;
	    if (!*names)
		break;
	    if (!names[1]) {
		get = AppendLEntry; /* bottom of recursion */
		leaf = 1;
	    }
	    GLOOSE(names, AppendLooseLEntry);   /* loose names */
	    GLOOSE(classes, AppendLooseLEntry); /* loose classes */
	}
    }
    /* now look for matching leaf node */
    entry = table->next;
    if (entry && !entry->leaf)
	entry = entry->next;
    if (entry && entry->tight != table->tight)
	entry = entry->next;
    if (!entry || entry->name != table->name || !entry->hasloose)
	return False;
    /* found it */
    return AppendLooseLEntry((LTable)entry, names, classes, closure);
}

Bool XrmQGetSearchList(db, names, classes, searchList, listLength)
    XrmDatabase     db;
    XrmNameList	    names;
    XrmClassList    classes;
    XrmSearchList   searchList;	/* RETURN */
    int		    listLength;
{
    register NTable	table;
    SClosureRec		closure;

    if (listLength <= 0)
	return False;
    closure.list = (LTable *)searchList;
    closure.idx = -1;
    closure.limit = listLength - 2;
    if (db) {
	table = db->table;
	if (*names) {
	    if (table && !table->leaf &&
		SearchNEntry(table, names, classes, &closure))
		return False;
	} else {
	    if (table && !table->leaf)
		table = table->next;
	    if (table && AppendLEntry((LTable)table, names, classes, &closure))
		return False;
	}
    }
    closure.list[closure.idx + 1] = (LTable)NULL;
    return True;
}

Bool XrmQGetSearchResource(searchList, name, class, pType, pValue)
	     XrmSearchList	searchList;
    register XrmName		name;
    register XrmClass		class;
    	     XrmRepresentation	*pType;  /* RETURN */
    	     XrmValue		*pValue; /* RETURN */
{
    register LTable *list;
    register LTable table;
    register VEntry entry;
    int flags;

/* find tight or loose entry */
#define VTIGHTLOOSE(q) \
    entry = LeafHash(table, q); \
    while (entry && entry->name != q) \
	entry = entry->next; \
    if (entry) \
	break

/* find loose entry */
#define VLOOSE(q) \
    entry = LeafHash(table, q); \
    while (entry && entry->name != q) \
	entry = entry->next; \
    if (entry) { \
	if (!entry->tight) \
	    break; \
	entry = entry->next; \
	if (entry && entry->name == q) \
	    break; \
    }

    list = (LTable *)searchList;
    /* figure out which combination of name and class we need to search for */
    flags = 0;
    if (IsResourceQuark(name))
	flags = 2;
    if (IsResourceQuark(class))
	flags |= 1;
    if (!flags) {
	/* neither name nor class has ever been used to name a resource */
	table = (LTable)NULL;
    } else if (flags == 3) {
	/* both name and class */
	while (table = *list++) {
	    if (table != LOOSESEARCH) {
		VTIGHTLOOSE(name);  /* do name, tight and loose */
		VTIGHTLOOSE(class); /* do class, tight and loose */
	    } else {
		table = *list++;
		VLOOSE(name);  /* do name, loose only */
		VLOOSE(class); /* do class, loose only */
	    }
	}
    } else {
	/* just one of name or class */
	if (flags == 1)
	    name = class;
	while (table = *list++) {
	    if (table != LOOSESEARCH) {
		VTIGHTLOOSE(name); /* tight and loose */
	    } else {
		table = *list++;
		VLOOSE(name); /* loose only */
	    }
	}
    }
    if (table) {
	/* found a match */
	if (entry->string) {
	    *pType = XrmQString;
	    pValue->addr = StringValue(entry);
	} else {
	    *pType = RepType(entry);
	    pValue->addr = DataValue(entry);
	}
	pValue->size = entry->size;
	return True;
    }
    *pType = NULLQUARK;
    pValue->addr = (caddr_t)NULL;
    pValue->size = 0;
    return False;

#undef VTIGHTLOOSE
#undef VLOOSE
}

/* look for a tight/loose value */
static Bool GetVEntry(table, names, classes, closure)
    LTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    VClosure		closure;
{
    register VEntry entry;
    register XrmQuark q;

    /* try name first */
    q = *names;
    entry = LeafHash(table, q);
    while (entry && entry->name != q)
	entry = entry->next;
    if (!entry) {
	/* not found, try class */
	q = *classes;
	entry = LeafHash(table, q);
	while (entry && entry->name != q)
	    entry = entry->next;
	if (!entry)
	    return False;
    }
    if (entry->string) {
	*closure->type = XrmQString;
	closure->value->addr = StringValue(entry);
    } else {
	*closure->type = RepType(entry);
	closure->value->addr = DataValue(entry);
    }
    closure->value->size = entry->size;
    return True;
}

/* look for a loose value */
static Bool GetLooseVEntry(table, names, classes, closure)
    LTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    VClosure		closure;
{
    register VEntry	entry;
    register XrmQuark	q;

#define VLOOSE(list) \
    q = *list; \
    entry = LeafHash(table, q); \
    while (entry && entry->name != q) \
	entry = entry->next; \
    if (entry && entry->tight) { \
	entry = entry->next; \
	if (entry && entry->name != q) \
	    entry = (VEntry)NULL; \
    }

    /* bump to last component */
    while (names[1]) {
	names++;
	classes++;
    }
    VLOOSE(names);  /* do name, loose only */
    if (!entry) {
	VLOOSE(classes); /* do class, loose only */
	if (!entry)
	    return False;
    }
    if (entry->string) {
	*closure->type = XrmQString;
	closure->value->addr = StringValue(entry);
    } else {
	*closure->type = RepType(entry);
	closure->value->addr = DataValue(entry);
    }
    closure->value->size = entry->size;
    return True;

#undef VLOOSE
}

/* recursive search for a value */
static Bool GetNEntry(table, names, classes, closure)
    NTable		table;
    XrmNameList		names;
    XrmClassList 	classes;
    VClosure		closure;
{
    register NTable	entry;
    register XrmQuark	q;
    register unsigned int leaf;
    Bool		(*get)();
    NTable		otable;

    if (names[2]) {
	get = GetNEntry; /* recurse */
	leaf = 0;
    } else {
	get = GetVEntry; /* bottom of recursion */
	leaf = 1;
    }
    GTIGHTLOOSE(names, GetLooseVEntry);   /* do name, tight and loose */
    GTIGHTLOOSE(classes, GetLooseVEntry); /* do class, tight and loose */
    if (table->hasloose) {
	while (1) {
	    names++;
	    classes++;
	    if (!names[1])
		break;
	    if (!names[2]) {
		get = GetVEntry; /* bottom of recursion */
		leaf = 1;
	    }
	    GLOOSE(names, GetLooseVEntry);   /* do name, loose only */
	    GLOOSE(classes, GetLooseVEntry); /* do class, loose only */
	}
    }
    /* look for a matching leaf table */
    otable = table;
    table = table->next;
    if (table && !table->leaf)
	table = table->next;
    if (table && table->tight != otable->tight)
	table = table->next;
    if (!table || table->name != otable->name || !table->hasloose)
	return False;
    /* found */
    return GetLooseVEntry((LTable)table, names, classes, closure);
}

Bool XrmQGetResource(db, names, classes, pType, pValue)
    XrmDatabase         db;
    XrmNameList		names;
    XrmClassList 	classes;
    XrmRepresentation	*pType;  /* RETURN */
    XrmValuePtr		pValue;  /* RETURN */
{
    register NTable table;
    VClosureRec closure;

    if (db && *names) {
	closure.type = pType;
	closure.value = pValue;
	table = db->table;
	if (names[1]) {
	    if (table && !table->leaf &&
		GetNEntry(table, names, classes, &closure))
		return True;
	} else {
	    if (table && !table->leaf)
		table = table->next;
	    if (table && GetVEntry((LTable)table, names, classes, &closure))
		return True;
	}
    }
    *pType = NULLQUARK;
    pValue->addr = (caddr_t)NULL;
    pValue->size = 0;
    return False;
}

#if NeedFunctionPrototypes
Bool XrmGetResource(db, name_str, class_str, pType_str, pValue)
    XrmDatabase         db;
    const char		*name_str;
    const char		*class_str;
    XrmString		*pType_str;  /* RETURN */
    XrmValuePtr		pValue;      /* RETURN */
#else
Bool XrmGetResource(db, name_str, class_str, pType_str, pValue)
    XrmDatabase         db;
    XrmString		name_str;
    XrmString		class_str;
    XrmString		*pType_str;  /* RETURN */
    XrmValuePtr		pValue;      /* RETURN */
#endif
{
    XrmName		names[MAXDBDEPTH+1];
    XrmClass		classes[MAXDBDEPTH+1];
    XrmRepresentation   fromType;
    Bool		result;

    XrmStringToNameList(name_str, names);
    XrmStringToClassList(class_str, classes);
    result = XrmQGetResource(db, names, classes, &fromType, pValue);
    (*pType_str) = XrmQuarkToString(fromType);
    return result;
}

/* destroy all values, plus table itself */
static void DestroyLTable(table)
    LTable table;
{
    register int i;
    register VEntry *buckets;
    register VEntry entry, next;

    buckets = table->buckets;
    for (i = table->table.mask; i >= 0; i--, buckets++) {
	for (next = *buckets; entry = next; ) {
	    next = entry->next;
	    Xfree(entry);
	}
    }
    Xfree(table->buckets);
    Xfree(table);
}

/* destroy all contained tables, plus table itself */
static void DestroyNTable(table)
    NTable table;
{
    register int i;
    register NTable *buckets;
    register NTable entry, next;

    buckets = NodeBuckets(table);
    for (i = table->mask; i >= 0; i--, buckets++) {
	for (next = *buckets; entry = next; ) {
	    next = entry->next;
	    if (entry->leaf)
		DestroyLTable((LTable)entry);
	    else
		DestroyNTable(entry);
	}
    }
    Xfree(table);
}

void XrmDestroyDatabase(db)
    XrmDatabase   db;
{
    register NTable table, next;

    if (db) {
	for (next = db->table; table = next; ) {
	    next = table->next;
	    if (table->leaf)
		DestroyLTable((LTable)table);
	    else
		DestroyNTable(table);
	}
	Xfree(db);
    }
}

typedef struct _GRNData {
    char *name;
    XrmRepresentation type;
    XrmValuePtr value;
} GRNData;

/*ARGSUSED*/
static Bool SameValue(db, bindings, quarks, type, value, data)
    XrmDatabase		*db;
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    XrmRepresentation   *type;
    XrmValuePtr		value;
    GRNData		*data;
{
    if ((*type == data->type) && (value->size == data->value->size) &&
	!strncmp((char *)value->addr, (char *)data->value->addr, value->size))
    {
	data->name = XrmQuarkToString(*quarks); /* XXX */
	return True;
    }
    return False;
}

/* Gross internal hack */
char *_XrmGetResourceName(rdb, type_str, pValue)
    XrmDatabase rdb;
    XrmString type_str;
    XrmValuePtr pValue;
{
    GRNData data;
    data.name = (char *)NULL;
    data.type = XrmStringToQuark(type_str);
    data.value = pValue;

    (void)EnumerateDatabase(rdb, SameValue, (caddr_t)&data);
    return data.name;
}
