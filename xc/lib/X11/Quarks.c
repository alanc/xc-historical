/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**/

#include "Xlibint.h"
#include "Xresource.h"

extern void bcopy();


typedef int Signature;

static XrmQuark nextQuark = 1;	/* next available quark number */
static XrmAtom *quarkToAtomTable = NULL;
static int maxQuarks = 0;	/* max names in current quarkToAtomTable */
#define QUARKQUANTUM 600;	/* how much to extend quarkToAtomTable by */


/* Permanent memory allocation */

#define NEVERFREETABLESIZE 8180
static char *neverFreeTable = NULL;
static int  neverFreeTableSize = 0;

char *Xpermalloc(length)
    unsigned int length;
{
    char *ret;

    /* round to nearest 4-byte boundary */
    length = (length + 3) & (~3);
    if (neverFreeTableSize < length) {
	neverFreeTableSize =
	    (length > NEVERFREETABLESIZE ? length : NEVERFREETABLESIZE);
	neverFreeTable = Xmalloc(neverFreeTableSize);
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
    XrmAtom	name;
} NodeRec;

#define HASHTABLESIZE 1024
#define HASHTABLEMASK 1023
static Node nodeTable[HASHTABLESIZE];



/* predefined quarks */

/* Representation types */

XrmQuark  XrmQBoolean;
XrmQuark  XrmQColor;
XrmQuark  XrmQCursor;
XrmQuark  XrmQDims;
XrmQuark  XrmQDisplay;
XrmQuark  XrmQFile;
XrmQuark  XrmQFont;
XrmQuark  XrmQFontStruct;
XrmQuark  XrmQGeometry;
XrmQuark  XrmQInt;
XrmQuark  XrmQPixel;
XrmQuark  XrmQPixmap;
XrmQuark  XrmQPointer;
XrmQuark  XrmQString;
XrmQuark  XrmQWindow;

/* "Enumeration" constants */

XrmQuark  XrmQEfalse;
XrmQuark  XrmQEno;
XrmQuark  XrmQEoff;
XrmQuark  XrmQEon;
XrmQuark  XrmQEtrue;
XrmQuark  XrmQEyes;


static XrmAllocMoreQuarkToAtomTable()
{
    unsigned	size;

    maxQuarks += QUARKQUANTUM;
    size = (unsigned) maxQuarks * sizeof(XrmAtom);
    if (quarkToAtomTable == (XrmAtom *)NULL)
	quarkToAtomTable = (XrmAtom *) Xmalloc(size);
    else
	quarkToAtomTable =
		(XrmAtom *) Xrealloc((char *) quarkToAtomTable, size);
}

XrmQuark XrmAtomToQuark(name)
    register XrmAtom name;
{
    register Signature 	sig = 0;
    register Signature	scale = 27;
    register XrmAtom	tname;
    register Node	np;
    register XrmAtom	npn;
    	     Node	*hashp;
	     unsigned	strLength;

    if (name == NULL)
	return (NULLQUARK);

    /* Compute atom signature (sparse 32-bit hash value) */
    for (tname = name; *tname != '\0'; tname++)
	sig = sig*scale + (unsigned int) *tname;
    strLength = tname - name + 1;

    /* Look for atom in hash table */
    hashp = &nodeTable[sig & HASHTABLEMASK];
    for (np = *hashp; np != NULL; np = np->next) {
	if (np->sig == sig) {
	    for (npn=np->name, tname = name;
	     ((scale = *tname) != 0) && (scale == *npn); ++tname, ++npn) {};
	    if (scale == *npn) {
	        return np->quark;
	    }
	}
    }

    /* Not found, add atom to hash table */
    np = (Node) Xpermalloc(sizeof(NodeRec));
    np->next = *hashp;
    *hashp = np;
    np->sig = sig;
    bcopy(name, (np->name = Xpermalloc(strLength)), (int) strLength);
    np->quark = nextQuark;

    if (nextQuark >= maxQuarks)
	XrmAllocMoreQuarkToAtomTable();

    quarkToAtomTable[nextQuark] = np->name;
    ++nextQuark;
    return np->quark;
}

extern XrmQuark XrmUniqueQuark()
{
    XrmQuark quark;

    quark = nextQuark;
    if (nextQuark >= maxQuarks)
	XrmAllocMoreQuarkToAtomTable();
    quarkToAtomTable[nextQuark] = NULLATOM;
    ++nextQuark;
    return (quark);
}


XrmAtom XrmQuarkToAtom(quark)
    XrmQuark quark;
{
    if (quark <= 0 || quark >= nextQuark)
    	return NULLATOM;
    return quarkToAtomTable[quark];
}

