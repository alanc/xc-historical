/* $XConsortium$ */
/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
******************************************************************************/

typedef struct _List {
    struct _List	*prev;
    struct _List	*next;
    void	*thing;
} List;

#if	0
extern List *ListInit();
extern List *ListFirst(List *);
extern List *ListNext(List *);
extern void ListFreeAll(List *);
extern void ListFreeOne(List *);
extern List *ListAddFirst(List *, void *);
extern List *ListAddLast(List *, void *);
extern int ListCount(List *);
#else
extern List *ListInit();
extern List *ListFirst();
extern List *ListNext();
extern void ListFreeAll();
extern void ListFreeOne();
extern List *ListAddFirst();
extern List *ListAddLast();
extern int ListCount();
#endif
