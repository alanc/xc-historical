/* $XConsortium$ */

/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
******************************************************************************/

#include "XIElibint.h"


Status
_XieRegisterTechFunc (group, technique, techfunc)

int group;
int technique;
XieTechFuncPtr techfunc;

{
    XieTechFuncRec *ptr = _XieTechFuncs[group - 1];
    XieTechFuncRec *prev = NULL, *newrec;
    int	status = 1;

    newrec = (XieTechFuncRec *) Xmalloc (sizeof (XieTechFuncRec));
    newrec->technique = technique;
    newrec->techfunc = techfunc;

    if (ptr == NULL)
    {
	_XieTechFuncs[group - 1] = newrec;
	newrec->next = NULL;
    }
    else
    {
	while (ptr && ptr->technique < technique)
	{
	    prev = ptr;
	    ptr = ptr->next;
	}

	if (ptr == NULL)
	{
	    prev->next = newrec;
	    newrec->next = NULL;
	}
	else
	{
	    if (ptr->technique == technique)
	    {
		Xfree ((char *) newrec);
		status = 0;
	    }
	    else if (prev = NULL)
	    {
		newrec->next = _XieTechFuncs[group - 1];
		_XieTechFuncs[group - 1] = newrec;
	    }
	    else
	    {
		newrec->next = prev->next;
		prev->next = newrec;
	    }
	}
    }

    return (status);
}



XieTechFuncPtr
_XieLookupTechFunc (group, technique)

int group;
int technique;

{
    XieTechFuncRec *ptr = _XieTechFuncs[group - 1];
    int found = 0;

    while (ptr && !found)
    {
	if (ptr->technique == technique)
	    found = 1;
	else
	    ptr = ptr->next;
    }

    if (found)
	return (ptr->techfunc);
    else
	return (NULL);
}


