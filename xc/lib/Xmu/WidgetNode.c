/*
 * $XConsortium: util.c,v 1.17 90/02/08 13:31:14 jim Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */



#include <stdio.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/WidgetNode.h>


static char *binsearch (key, base, nelems, elemsize, compar)
    char *key;				/* template of object to find */
    char *base;				/* beginning of array */
    int nelems;				/* number of elements in array */
    int elemsize;			/* sizeof an element */
    int (*compar)();			/* qsort-style compare function */
{
    int lower = 0, upper = nelems - 1;

    while (lower <= upper) {
	int middle = (lower + upper) / 2;
	char *p = base + middle * elemsize;
	int res = (*compar) (p, key);

	if (res < 0) {
	    lower = middle + 1;
	} else if (res == 0) {
	    return p;
	} else {
	    upper = middle - 1;
	}
    }

    return NULL;
}


static int compare_resource_entries (a, b)
    char *a, *b;
{
    return strcmp (((XtResourceList)a)->resource_name,
		   ((XtResourceList)b)->resource_name);
}


static XmuWidgetNode *find_resource (node, name)
    XmuWidgetNode *node;
    char *name;
{
    XmuWidgetNode *sup;
    XtResource res;

    res.resource_name = name;
    for (sup = node->superclass; 
	 sup && (XtResourceList) binsearch ((char *) &res,
					    (char *) sup->resources,
					    (int) sup->nresources, 
					    sizeof(XtResource),
					    compare_resource_entries);
	 node = sup, sup = sup->superclass) ;

    return node;
}


static void mark_resource_owner (node)
    XmuWidgetNode *node;
{
    int i;
    XtResourceList childres;

    childres = node->resources;

    for (i = 0; i < node->nresources; i++, childres++) {
	node->resourcewn[i] = find_resource (node, childres->resource_name);
    }
}


/*
 * 			       Public Interfaces
 */

void XmuWnInitializeNodes (nodearray, nnodes)
    XmuWidgetNode *nodearray;
    int nnodes;
{
    int i;
    XmuWidgetNode *wn;

    /*
     * Assume that the node array is in alphabetic order, so we need to
     * search backwards to make sure that the children are listed forward.
     */
    for (i = nnodes - 1, wn = nodearray + (nnodes - 1); i >= 0; i--, wn--) {
	WidgetClass superclass = XmuWnSuperclass(wn);
	int j;
	XmuWidgetNode *swn;
	int lablen = strlen (wn->label);
	int namelen = strlen (XmuWnClassname(wn));

	wn->lowered_label = XtMalloc (lablen + namelen + 2);
	if (!wn->lowered_label) {
	    fprintf (stderr,
		     "%s:  unable to allocate %d bytes for widget name\n",
		     "XmuWnInitializeNodes");
	    exit (1);
	}
	wn->lowered_classname = wn->lowered_label + (lablen + 1);
	XmuCopyISOLatin1Lowered (wn->lowered_label, wn->label);
	XmuCopyISOLatin1Lowered (wn->lowered_classname, XmuWnClassname(wn));
	wn->superclass = NULL;
	wn->have_resources = False;
	wn->resources = NULL;
	wn->resourcewn = NULL;
	wn->nresources = 0;
	wn->data = (XtPointer) NULL;

	/*
	 * walk up the superclass chain
	 */
	while (superclass) {
	    for (j = 0, swn = nodearray; j < nnodes; j++, swn++) {
		if (superclass == XmuWnClass(swn)) {
		    wn->superclass = swn;
		    goto done;		/* stupid C language */
	        }
	    }
	    /*
	     * Hmm, we have a hidden superclass (such as in core in R4); just
	     * ignore it and keep on walking
	     */
	    superclass = superclass->core_class.superclass;
	}
      done: 
	if (wn->superclass) {
	    wn->siblings = wn->superclass->children;
	    wn->superclass->children = wn;
	}
    }

    return;
}


void XmuWnFetchResources (node, toplevel, topnode)
    XmuWidgetNode *node;
    Widget toplevel;
    XmuWidgetNode *topnode;
{
    Widget dummy;
    XmuWidgetNode *wn;

    if (node->have_resources) return;

    dummy = XtCreateWidget (node->label, XmuWnClass(node), toplevel,
			    NULL, ZERO);
    if (dummy) XtDestroyWidget (dummy);


    /*
     * walk up tree geting resources; since we've instantiated the widget,
     * we know that all of our superclasses have been initialized
     */
    for (wn = node; wn && !wn->have_resources; wn = wn->superclass) {
	XtGetResourceList (XmuWnClass(wn), &wn->resources, &wn->nresources);
	if (wn->resources) {
	    qsort ((char *) wn->resources, wn->nresources,
		   sizeof(XtResource), compare_resource_entries);
	}
	wn->resourcewn = (XmuWidgetNode **) XtCalloc (wn->nresources,
						  sizeof (XmuWidgetNode *));
	if (!wn->resourcewn) {
	    fprintf (stderr,
		     "%s:  unable to calloc %d %d byte widget node ptrs\n",
		     "XmuWnFetchResources",wn->nresources,
		     sizeof (XmuWidgetNode *));
	    exit (1);
	}
	wn->have_resources = True;
	if (wn == topnode) break;
    }


    /*
     * Walk up tree removing all resources that appear in superclass; we can
     * mash the resource list in place since it was copied out of widget.
     */
    for (wn = node; wn; wn = wn->superclass) {
	mark_resource_owner (wn);
	if (wn == topnode) break;
    }

    return;
}


int XmuWnCountOwnedResources (node, ownernode)
    XmuWidgetNode *node, *ownernode;
{
    register int i;
    XmuWidgetNode **wn = node->resourcewn;
    int nmatches = 0;

    for (i = 0; i < node->nresources; i++, wn++) {
	if (*wn == ownernode) nmatches++;
    }
    return nmatches;
}


XmuWidgetNode *XmuWnNameToNode (nodelist, nnodes, name)
    XmuWidgetNode *nodelist;
    int nnodes;
    char *name;
{
    int i;
    XmuWidgetNode *wn;
    char tmp[1024];

    XmuCopyISOLatin1Lowered (tmp, name);
    for (i = 0, wn = nodelist; i < nnodes; i++, wn++) {
	if (strcmp (tmp, wn->lowered_label) == 0 ||
	    strcmp (tmp, wn->lowered_classname) == 0)
	  return wn;
    }
    return NULL;
}
