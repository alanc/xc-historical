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

#include <stdio.h>
#include <X11/Xosdefs.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif

#include "list.h"

List *
ListInit()
{
	List *l;

	l = (List *)malloc(sizeof *l);
	if(!l) return l;
	l->next = l;
	l->prev = l;
	l->thing = NULL;
	return l;
}

List *
ListFirst(l)
List *l;
{
	if(l->next->thing) return l->next;
	else return NULL;
}

List *
ListNext(l)
List *l;
{
	if(l->next->thing) return l->next;
	else return NULL;
}

void
ListFreeAll(l)
List *l;
{
	void *thing;
	List *next;

	next = l->next;
	do {
		l = next;
		next = l->next;
		thing = l->thing;
		free(l);
	} while(thing);
}

List *
ListAddFirst(l, v)
List *l;
void *v;
{
	List *e;

	e = (List *)malloc(sizeof *e);
	if(!e) return NULL;

	e->thing = v;
	e->prev = l;
	e->next = e->prev->next;
	e->prev->next = e;
	e->next->prev = e;

	return e;
}

List *
ListAddLast(l, v)
List *l;
void *v;
{
	List *e;

	e = (List *)malloc(sizeof *e);
	if(!e) return NULL;

	e->thing = v;
	e->next = l;
	e->prev = e->next->prev;
	e->prev->next = e;
	e->next->prev = e;

	return e;
}

void
ListFreeOne(e)
List *e;
{
	e->next->prev = e->prev;
	e->prev->next = e->next;
	free((char *)e);
}

int
ListCount(l)
List *l;
{
	int i;
	List *e;

	i = 0;
	for(e = ListFirst(l); e; e = ListNext(e)) i++;

	return i;
}
