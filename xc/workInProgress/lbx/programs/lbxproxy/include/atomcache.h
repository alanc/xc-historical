/*
 * Copyright 1988-1993 Network Computing Devices, Inc.  All rights reserved.
 * An unpublished work.
 * 
 * $NCDId: @(#)atomcache.h,v 1.1 1993/12/01 23:46:44 lemke Exp $
 */

#ifndef _ATOMCACHE_H_
#define _ATOMCACHE_H_

#ifndef X_PROTOCOL
typedef unsigned long   Atom;
typedef unsigned long   XID;
#endif

#ifndef None
#define None    0l
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

extern char *NameForAtom();
extern Atom MakeAtom();

#endif				/* _ATOMCACHE_H_ */
