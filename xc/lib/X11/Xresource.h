/* $XConsortium: Xresource.h,v 1.18 89/12/08 18:29:24 converse Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

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

#ifndef _Xresource_h
#define _Xresource_h

#ifdef __cplusplus
extern "C" {					/* for C++ V2.0 */
#endif

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

#if defined(CRAY) || defined(USG)
#ifndef __TYPES__
#define __TYPES__
#include <sys/types.h>			/* forgot to protect it... */
#endif /* __TYPES__ */
#else
#include <sys/types.h>
#endif /* CRAY or USG */

#ifndef NeedFunctionPrototypes
#if defined(__STDC__) || defined(__cplusplus)
#define NeedFunctionPrototypes 1
#else
#define NeedFunctionPrototypes 0
#endif /* __STDC__ */
#endif /* NeedFunctionPrototypes */

#ifndef NULL
#define NULL 0
#endif

/****************************************************************
 *
 * ||| Memory Management (move out of here!)
 *
 ****************************************************************/

extern char *Xpermalloc(
#if NeedFunctionPrototypes
    unsigned int	/* size */
#endif
);

/****************************************************************
 *
 * Quark Management
 *
 ****************************************************************/

typedef int     XrmQuark, *XrmQuarkList;
#define NULLQUARK ((XrmQuark) 0)

typedef char *XrmString;
#define NULLSTRING ((XrmString) 0)

/* find quark for string, create new quark if none already exists */
extern XrmQuark XrmStringToQuark(
#if NeedFunctionPrototypes
    XrmString 		/* string */
#endif
);

/* find string for quark */
extern XrmString XrmQuarkToString(
#if NeedFunctionPrototypes
    XrmQuark 		/* quark */
#endif
);

extern XrmQuark XrmUniqueQuark(
#if NeedFunctionPrototypes
    void
#endif
);

#define XrmStringsEqual(a1, a2) (strcmp(a1, a2) == 0)


/****************************************************************
 *
 * Conversion of Strings to Lists
 *
 ****************************************************************/

typedef enum {XrmBindTightly, XrmBindLoosely} XrmBinding, *XrmBindingList;

extern void XrmStringToQuarkList(
#if NeedFunctionPrototypes
    char*		/* string */,
    XrmQuarkList	/* quarks_return */
#endif
);

extern void XrmStringToBindingQuarkList(
#if NeedFunctionPrototypes
    char*		/* string */,
    XrmBindingList	/* bindings_return */,
    XrmQuarkList	/* quarks_return */
#endif
);

/****************************************************************
 *
 * Name and Class lists.
 *
 ****************************************************************/

typedef XrmQuark     XrmName;
typedef XrmQuarkList XrmNameList;
#define XrmNameToString(name)		XrmQuarkToString(name)
#define XrmStringToName(string)		XrmStringToQuark(string)
#define XrmStringToNameList(str, name)	XrmStringToQuarkList(str, name)

typedef XrmQuark     XrmClass;
typedef XrmQuarkList XrmClassList;
#define XrmClassToString(class)		XrmQuarkToString(class)
#define XrmStringToClass(class)		XrmStringToQuark(class)
#define XrmStringToClassList(str,class)	XrmStringToQuarkList(str, class)



/****************************************************************
 *
 * Resource Representation Types and Values
 *
 ****************************************************************/

typedef XrmQuark     XrmRepresentation;
#define XrmStringToRepresentation(string)   XrmStringToQuark(string)
#define	XrmRepresentationToString(type)   XrmQuarkToString(type)

typedef struct {
    unsigned int    size;
    caddr_t	    addr;
} XrmValue, *XrmValuePtr;


/****************************************************************
 *
 * Resource Manager Functions
 *
 ****************************************************************/

typedef struct _XrmHashBucketRec *XrmHashBucket;
typedef XrmHashBucket *XrmHashTable;
typedef XrmHashTable XrmSearchList[];
typedef struct _XrmHashBucketRec *XrmDatabase;


extern void XrmInitialize(
#if NeedFunctionPrototypes
    void
#endif
);

extern void XrmDestroyDatabase(
#if NeedFunctionPrototypes
    XrmDatabase		/* database */    
#endif
);

extern void XrmQPutResource(
#if NeedFunctionPrototypes
    XrmDatabase*	/* database */,
    XrmBindingList	/* bindings */,
    XrmQuarkList	/* quarks */,
    XrmRepresentation	/* type */,
    XrmValue*		/* value */
#endif
);

extern void XrmPutResource(
#if NeedFunctionPrototypes
    XrmDatabase*	/* database */,
    char*		/* specifier */,
    char*		/* type */,
    XrmValue*		/* value */
#endif
);

extern void XrmQPutStringResource(
#if NeedFunctionPrototypes
    XrmDatabase*	/* database */,
    XrmBindingList      /* bindings */,
    XrmQuarkList	/* quarks */,
    char*		/* value */
#endif
);

extern void XrmPutStringResource(
#if NeedFunctionPrototypes
    XrmDatabase*	/* database */,
    char*		/* specifier */,
    char*		/* value */
#endif
);

extern void XrmPutLineResource(
#if NeedFunctionPrototypes
    XrmDatabase*	/* database */,
    char*		/* line */
#endif
);

extern  XrmQGetResource(
#if NeedFunctionPrototypes
    XrmDatabase		/* database */,
    XrmNameList		/* quark_name */,
    XrmClassList	/* quark_class */,
    XrmRepresentation*	/* quark_type_return */,
    XrmValue*		/* value_return */
#endif
);

extern Bool XrmGetResource(
#if NeedFunctionPrototypes
    XrmDatabase		/* database */,
    char*		/* str_name */,
    char*		/* str_class */,
    char**		/* str_type_return */,
    XrmValue*		/* value_return */
#endif
);

extern Bool XrmQGetSearchList(
#if NeedFunctionPrototypes
    XrmDatabase		/* database */,
    XrmNameList		/* names */,
    XrmClassList	/* classes */,
    XrmSearchList	/* list_return */,
    int			/* list_length */
#endif
);

extern Bool XrmQGetSearchResource(
#if NeedFunctionPrototypes
    XrmSearchList	/* list */,
    XrmName		/* name */,
    XrmClass		/* class */,
    XrmRepresentation*	/* type_return */,
    XrmValue*		/* value_return */
#endif
);

/****************************************************************
 *
 * Resource Database Management
 *
 ****************************************************************/

extern XrmDatabase XrmGetFileDatabase(
#if NeedFunctionPrototypes
    char*		/* filename */
#endif
);

extern XrmDatabase XrmGetStringDatabase(
#if NeedFunctionPrototypes
    char*		/* data */  /*  null terminated string */
#endif
);

extern void XrmPutFileDatabase(
#if NeedFunctionPrototypes
    XrmDatabase		/* database */,
    char*		/* filename */
#endif
);

extern void XrmMergeDatabases(
#if NeedFunctionPrototypes
    XrmDatabase		/* source_db */,
    XrmDatabase*	/* target_db */
#endif
);



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
    XrmoptionSkipLine,  /* Ignore this option and the rest of argv	    */
    XrmoptionSkipNArgs	/* Ignore this option and the next 
			   OptionDescRes.value arguments in argv */
} XrmOptionKind;

typedef struct {
    char	    *option;	    /* Option abbreviation in argv	    */
    char	    *specifier;     /* Resource specifier		    */
    XrmOptionKind   argKind;	    /* Which style of option it is	    */
    caddr_t	    value;	    /* Value to provide if XrmoptionNoArg   */
} XrmOptionDescRec, *XrmOptionDescList;


extern void XrmParseCommand(
#if NeedFunctionPrototypes
    XrmDatabase*	/* database */,
    XrmOptionDescList	/* table */,
    int			/* table_count */,
    char*		/* name */,
    int*		/* argc_in_out */,
    char**		/* argv_in_out */		     
#endif
);

#ifdef __cplusplus
}						/* for C++ V2.0 */
#endif

#endif /* _Xresource_h */
/* DON'T ADD STUFF AFTER THIS #endif */
