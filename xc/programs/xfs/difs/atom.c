/* $XConsortium: atom.c,v 1.5 94/04/12 22:00:42 dpw Exp $ */
/*
 * font server atom manipulations
 */
/*
Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation 
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices,
 * or Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Network Computing Devices, or Digital
 * make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES, AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)atom.c	4.1	5/2/91
 *
 */

#include "misc.h"
#include "resource.h"

#define InitialTableSize 100
#define	FSA_LAST_PREDEFINED	0 /* only None is predefined */

typedef struct _Node {
    struct _Node *left,
               *right;
    Atom        a;
    unsigned int fingerPrint;
    char       *string;
}           NodeRec, *NodePtr;

static Atom lastAtom = None;
static NodePtr atomRoot = (NodePtr) NULL;
static unsigned long tableLength;
static NodePtr *nodeTable;

Atom
MakeAtom(string, len, makeit)
    char       *string;
    unsigned    len;
    Bool        makeit;
{
    register NodePtr *np;
    unsigned    i;
    int         comp;
    register unsigned int fp = 0;

    np = &atomRoot;
    for (i = 0; i < (len + 1) / 2; i++) {
	fp = fp * 27 + string[i];
	fp = fp * 27 + string[len - 1 - i];
    }
    while (*np != (NodePtr) NULL) {
	if (fp < (*np)->fingerPrint)
	    np = &((*np)->left);
	else if (fp > (*np)->fingerPrint)
	    np = &((*np)->right);
	else {			/* now start testing the strings */
	    comp = strncmp(string, (*np)->string, (int) len);
	    if ((comp < 0) || ((comp == 0) && (len < strlen((*np)->string))))
		np = &((*np)->left);
	    else if (comp > 0)
		np = &((*np)->right);
	    else
		return (*np)->a;
	}
    }
    if (makeit) {
	register NodePtr nd;

	nd = (NodePtr) fsalloc(sizeof(NodeRec));
	if (!nd)
	    return BAD_RESOURCE;
	if (lastAtom < FSA_LAST_PREDEFINED) {
	    nd->string = string;
	} else {
	    nd->string = (char *) fsalloc(len + 1);
	    if (!nd->string) {
		fsfree(nd);
		return BAD_RESOURCE;
	    }
	    strncpy(nd->string, string, (int) len);
	    nd->string[len] = 0;
	}
	if ((lastAtom + 1) >= tableLength) {
	    NodePtr    *table;

	    table = (NodePtr *) fsrealloc(nodeTable,
					tableLength * (2 * sizeof(NodePtr)));
	    if (!table) {
		if (nd->string != string)
		    fsfree(nd->string);
		fsfree(nd);
		return BAD_RESOURCE;
	    }
	    tableLength <<= 1;
	    nodeTable = table;
	}
	*np = nd;
	nd->left = nd->right = (NodePtr) NULL;
	nd->fingerPrint = fp;
	nd->a = (++lastAtom);
	*(nodeTable + lastAtom) = nd;
	return nd->a;
    } else
	return None;
}

ValidAtom(atom)
    Atom        atom;
{
    return (atom != None) && (atom <= lastAtom);
}

char       *
NameForAtom(atom)
    Atom        atom;
{
    NodePtr     node;

    if (atom > lastAtom)
	return 0;
    if ((node = nodeTable[atom]) == (NodePtr) NULL)
	return 0;
    return node->string;
}

static void
atom_error()
{
    FatalError("initializing atoms\n");
}

static void
free_atom(patom)
    NodePtr     patom;
{
    if (patom->left)
	free_atom(patom->left);
    if (patom->right)
	free_atom(patom->right);
    if (patom->a > FSA_LAST_PREDEFINED)
	fsfree(patom->string);
    fsfree(patom);
}

static void
free_all_atoms()
{
    if (atomRoot == (NodePtr) NULL)
	return;
    free_atom(atomRoot);
    atomRoot = (NodePtr) NULL;
    fsfree(nodeTable);
    nodeTable = (NodePtr *) NULL;
    lastAtom = None;
}

void
InitAtoms()
{
    free_all_atoms();
    tableLength = InitialTableSize;
    nodeTable = (NodePtr *) fsalloc(InitialTableSize * sizeof(NodePtr));
    if (!nodeTable)
	atom_error();
    nodeTable[None] = (NodePtr) NULL;
    lastAtom = FSA_LAST_PREDEFINED;
}
