/* $Header: Xresource.h,v 1.1 87/09/12 12:27:11 toddb Exp $ */

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifndef _Xresource_h
#define _Xresource_h

/****************************************************************
 ****************************************************************
 ***                                                          ***
 ***                                                          ***
 ***          X Resource Manager Intrinsics                   ***
 ***                                                          ***
 ***                                                          ***
 ****************************************************************
 ****************************************************************/



/****************************************************************
 *
 * Miscellaneous definitions
 *
 ****************************************************************/

#include	<sys/types.h>

#ifndef NULL
#define NULL 0
#endif

/****************************************************************
 *
 * ||| Memory Management (move out of here!)
 *
 ****************************************************************/

extern char *Xpermalloc();
    /* unsigned int size;   */

/****************************************************************
 *
 * Quark Management
 *
 ****************************************************************/

typedef int     XrmQuark, *XrmQuarkList;
#define NULLQUARK ((XrmQuark) 0)

typedef char *XrmAtom;
#define NULLATOM ((XrmAtom) 0)

/* find quark for atom, create new quark if none already exists */
extern XrmQuark XrmAtomToQuark(); /* name */
    /* XrmAtom name; */

/* find atom for quark */
extern XrmAtom XrmQuarkToAtom(); /* quark */
    /* XrmQuark name; */

extern XrmQuark XrmUniqueQuark();

#define XrmAtomsEqual(a1, a2) (strcmp(a1, a2) == 0)


/****************************************************************
 *
 * Conversion of Strings to Lists
 *
 ****************************************************************/

extern void XrmStringToQuarkList();
    /* char		*name;  */
    /* XrmQuarkList     quarks; */  /* RETURN */

extern void XrmStringToBindingQuarkList();
    /* char		*name;      */
    /* XrmBindingList   bindings;   */  /* RETURN */
    /* XrmQuarkList     quarks;     */  /* RETURN */


/****************************************************************
 *
 * Name and Class lists.
 *
 ****************************************************************/

typedef XrmQuark     XrmName;
typedef XrmQuarkList XrmNameList;
#define XrmNameToAtom(name)		XrmQuarkToAtom(name)
#define XrmAtomToName(atom)		XrmAtomToQuark(atom)
#define XrmStringToNameList(str, name)	XrmStringToQuarkList(str, name)

typedef XrmQuark     XrmClass;
typedef XrmQuarkList XrmClassList;
#define XrmClassToAtom(class)		XrmQuarkToAtom(class)
#define XrmAtomToClass(class)		XrmAtomToQuark(class)
#define XrmStringToClassList(str,class)	XrmStringToQuarkList(str, class)



/****************************************************************
 *
 * Resource Representation Types and Values
 *
 ****************************************************************/

typedef XrmQuark     XrmRepresentation;
#define XrmAtomToRepresentation(atom)   XrmAtomToQuark(atom)
#define	XrmRepresentationToAtom(type)   XrmQuarkToAtom(type)

typedef struct {
    unsigned int    size;
    caddr_t	    addr;
} XrmValue, *XrmValuePtr;


/****************************************************************
 *
 * Resource Manager Functions
 *
 ****************************************************************/

typedef enum {XrmBindTightly, XrmBindLoosely} XrmBinding, *XrmBindingList;

typedef struct _XrmHashBucketRec *XrmHashBucket;
typedef XrmHashBucket *XrmHashTable;
typedef XrmHashTable XrmSearchList[];
typedef struct _XrmHashBucketRec *XrmDatabase;


extern void XrmInitialize();

extern void XrmQPutResource();
    /* XrmDatabase	    *pdb;	*/
    /* XrmBindingList       bindings;   */
    /* XrmQuarkList	    quarks;     */
    /* XrmRepresentation    type;       */
    /* XrmValue		    *value;	*/

extern void XrmPutResource();
    /* XrmDatabase	    *pdb;       */
    /* char		    *specifier; */
    /* char		    *type;      */
    /* XrmValue		    *value;     */

extern void XrmQPutStringResource();
    /* XrmDatabase	    *pdb;       */
    /* XrmBindingList       bindings;   */
    /* XrmQuarkList	    quarks;     */
    /* char		    *str;       */

extern void XrmPutStringResource();
    /* XrmDatabase	    *pdb;       */
    /* char		    *specifier; */
    /* char		    *str;       */

extern void XrmPutLineResource();
    /* XrmDatabase	    *pdb;       */
    /* char		    *line;	*/

extern  XrmQGetResource();
    /* XrmDatabase	    db;		*/
    /* XrmNameList	    names;      */
    /* XrmClassList	    classes;    */
    /* XrmRepresentation    *type;      */  /* RETURN */
    /* XrmValue		    *value;     */  /* RETURN */

extern Bool XrmGetResource();
    /* XrmDatabase	    db;		*/
    /* char		    *name_str;  */
    /* char		    *class_str; */
    /* char		    *type;      */  /* RETURN */
    /* XrmValue		    *value;     */  /* RETURN */

extern void XrmQGetSearchList();
    /* XrmDatabase	    db;		*/
    /* XrmNameList	    names;      */
    /* XrmClassList	    classes;    */
    /* XrmSearchList	    searchList; */  /* RETURN */

extern Bool XrmQGetSearchResource();
    /* SearchList	    searchList; */
    /* XrmName		    name;       */
    /* XrmClass		    class;      */
    /* XrmRepresentation    *type;      */  /* RETURN */
    /* XrmValue		    *value;     */  /* RETURN */

/****************************************************************
 *
 * Resource Database Management
 *
 ****************************************************************/

extern XrmDatabase XrmGetFileDatabase();
    /* char	    *filename;  */

extern XrmDatabase XrmGetStringDatabase();
    /* char	    *data;      */  /*  null terminated string */

extern void XrmPutFileDatabase();
    /* XrmDatabase  db;		*/
    /* char	    *filename   */

extern void XrmMergeDatabases();
    /* XrmDatabase  new;	*/
    /* XrmDatabase  *into;      */  /* RETURN */




/****************************************************************
 *
 * Command line option mapping to resource entries
 *
 ****************************************************************/

typedef enum {
    XrmoptionNoArg,	/* Value is specified in OptionDescRec.value	    */
    XrmoptionIsArg,     /* Value is the option string itself		    */
    XrmoptionStickyArg, /* Value is characters immediately following option */
    XrmoptionSepArg,    /* Value is next argument in argv		    */
    XrmoptionResArg,	/* Resource and value in next argument in argv      */
    XrmoptionSkipArg,   /* Ignore this option and the next argument in argv */
    XrmoptionSkipLine   /* Ignore this option and the rest of argv	    */
} XrmOptionKind;

typedef struct {
    char	    *option;	    /* Option abbreviation in argv	    */
    char	    *specifier;     /* Resource specifier		    */
    XrmOptionKind   argKind;	    /* Which style of option it is	    */
    caddr_t	    value;	    /* Value to provide if XrmoptionNoArg   */
} XrmOptionDescRec, *XrmOptionDescList;

extern void XrmParseCommand();
    /* XrmDatabase	    *pdb;	    */
    /* XrmOptionDescList    options;	    */
    /* int		    num_options;    */
    /* char		    *prefix;	    */
    /* int		    *argc;	    */
    /* char		    **argv;	    */



#endif _Xresource_h
/* DON'T ADD STUFF AFTER THIS #endif */
