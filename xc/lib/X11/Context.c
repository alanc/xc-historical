/* $XConsortium: Context.c,v 1.16 92/07/31 17:43:27 rws Exp $ */

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

/* This module implements a simple sparse array.

   XSaveContext(a,b,c,d) will store d in position (a,b,c) of the array.
   XFindContext(a,b,c,&d) will set d to be the value in position (a,b,c).
   XDeleteContext(a,b,c) will delete the entry in (a,b,c).

   a is a display id, b is a resource id, and c is a Context.  d is just an
   XPointer.  This code will work with any range of parameters, but is geared
   to be most efficient with very few (one or two) different a's.

*/

#include "Xlibint.h"
#include "Xutil.h"
#ifdef XTHREADS
#include "locking.h"
#endif

#define INITHASHMASK 63 /* Number of entries originally in the hash table. */

typedef struct _TableEntryRec {	/* Stores one entry. */
    XID 			rid;
    XContext			context;
    XPointer			data;
    struct _TableEntryRec	*next;
} TableEntryRec, *TableEntry;

typedef struct _XContextDB {	/* Stores hash table for one display. */
    TableEntry *table;		/* Pointer to array of hash entries. */
    int mask;			/* Current size of hash table minus 1. */
    int numentries;		/* Number of entries currently in table. */
#ifdef XTHREADS
    LockInfoRec linfo;
#endif
} DBRec, *DB;

/* Given an XID and a context, returns a value between 0 and HashSize-1.
   Currently, this requires that HashSize be a power of 2.
*/

#define Hash(db,rid,context) \
    (db)->table[(((rid) << 1) + context) & (db)->mask]

/* Resize the given db */

static void ResizeTable(db)
    register DB db;
{
    TableEntry *otable;
    register TableEntry entry, next, *pold, *head;
    register int i, j;

    otable = db->table;
    for (i = INITHASHMASK+1; (i + i) < db->numentries; )
	i += i;
    db->table = (TableEntry *) Xcalloc((unsigned)i, sizeof(TableEntry));
    if (!db->table) {
	db->table = otable;
	return;
    }
    j = db->mask + 1;
    db->mask = i - 1;
    for (pold = otable ; --j >= 0; pold++) {
	for (entry = *pold; entry; entry = next) {
	    next = entry->next;
	    head = &Hash(db, entry->rid, entry->context);
	    entry->next = *head;
	    *head = entry;
	}
    }
    Xfree((char *) otable);
}

static void _XFreeContextDB(display)
    Display *display;
{
    register DB db;
    register int i;
    register TableEntry *pentry, entry, next;

    if (db = display->context_db) {
	for (i = db->mask + 1, pentry = db->table ; --i >= 0; pentry++) {
	    for (entry = *pentry; entry; entry = next) {
		next = entry->next;
		Xfree((char *)entry);
	    }
	}
	Xfree((char *) db->table);
	Xfree((char *) db);
    }
}

/* Public routines. */

/* Save the given value of data to correspond with the keys XID and context.
   Returns nonzero error code if an error has occured, 0 otherwise.
   Possible errors are Out-of-memory.
*/   

#if NeedFunctionPrototypes
int XSaveContext(
    Display *display,
    register XID rid,
    register XContext context,
    _Xconst char* data)
#else
int XSaveContext(display, rid, context, data)
    Display *display;
    register XID rid;
    register XContext context;
    XPointer data;
#endif
{
    DB *pdb;
    register DB db;
    TableEntry *head;
    register TableEntry entry;

    LockDisplay(display);
    pdb = &display->context_db;
    db = *pdb;
    UnlockDisplay(display);
    if (!db) {
	db = (DB) Xmalloc(sizeof(DBRec));
	if (!db)
	    return XCNOMEM;
	db->mask = INITHASHMASK;
	db->table = (TableEntry *)Xcalloc(db->mask + 1, sizeof(TableEntry));
	if (!db->table) {
	    Xfree((char *)db);
	    return XCNOMEM;
	}
	db->numentries = 0;
	CreateMutex(&db->linfo);
	LockDisplay(display);
	*pdb = db;
	display->free_funcs->context_db = _XFreeContextDB;
	UnlockDisplay(display);
    }
    LockMutex(&db->linfo);
    head = &Hash(db, rid, context);
    UnlockMutex(&db->linfo);
    for (entry = *head; entry; entry = entry->next) {
	if (entry->rid == rid && entry->context == context) {
	    entry->data = (XPointer)data;
	    return 0;
	}
    }
    entry = (TableEntry) Xmalloc(sizeof(TableEntryRec));
    if (!entry)
	return XCNOMEM;
    entry->rid = rid;
    entry->context = context;
    entry->data = (XPointer)data;
    entry->next = *head;
    *head = entry;
    LockMutex(&db->linfo);
    db->numentries++;
    if (db->numentries > (db->mask << 2))
	ResizeTable(db);
    UnlockMutex(&db->linfo);
    return 0;
}



/* Given an XID and context, returns the associated data.  Note that data 
   here is a pointer since it is a return value.  Returns nonzero error code
   if an error has occured, 0 otherwise.  Possible errors are Entry-not-found.
*/

int XFindContext(display, rid, context, data)
    Display *display;
    register XID rid;
    register XContext context;
    XPointer *data;		/* RETURN */
{
    register DB db;
    register TableEntry entry;

    LockDisplay(display);
    db = display->context_db;
    UnlockDisplay(display);
    if (!db)
	return XCNOENT;
    LockMutex(&db->linfo);
    for (entry = Hash(db, rid, context); entry; entry = entry->next)
    {
	if (entry->rid == rid && entry->context == context) {
	    *data = (XPointer)entry->data;
	    UnlockMutex(&db->linfo);
	    return 0;
	}
    }
    UnlockMutex(&db->linfo);
    return XCNOENT;
}



/* Deletes the entry for the given XID and context from the datastructure.
   This returns the same thing that FindContext would have returned if called
   with the same arguments.
*/

int XDeleteContext(display, rid, context)
    Display *display;
    register XID rid;
    register XContext context;
{
    register DB db;
    register TableEntry entry, *prev;

    LockDisplay(display);
    db = display->context_db;
    UnlockDisplay(display);
    if (!db)
	return XCNOENT;
    LockMutex(&db->linfo);
    for (prev = &Hash(db, rid, context);
	 entry = *prev;
	 prev = &entry->next) {
	if (entry->rid == rid && entry->context == context) {
	    *prev = entry->next;
	    Xfree((char *) entry);
	    db->numentries--;
	    if (db->numentries < db->mask && db->mask > INITHASHMASK)
		ResizeTable(db);
	    UnlockMutex(&db->linfo);
	    return 0;
	}
    }
    UnlockMutex(&db->linfo);
    return XCNOENT;
}
