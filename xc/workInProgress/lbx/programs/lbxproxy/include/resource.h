/* $XConsortium: resource.h,v 1.2 95/05/24 16:11:18 mor Exp $ */

/*

Copyright (c) 1995  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#ifndef RESOURCE_H
#define RESOURCE_H 1

#define RT_COLORMAP	((RESTYPE)1)
#define RT_CMAPENTRY	((RESTYPE)2)
#define RT_LASTPREDEF	((RESTYPE)2)
#define RT_NONE		((RESTYPE)0)

#define RC_NEVERRETAIN	((RESTYPE)1<<29)
#define RC_LASTPREDEF	RC_NEVERRETAIN
#define RC_ANY		(~(RESTYPE)0)

typedef unsigned long RESTYPE;

/* bits and fields within a resource id */
#define CLIENTOFFSET 22					/* client field */
#define RESOURCE_ID_MASK	0x3FFFFF		/* low 22 bits */
#define CLIENT_BITS(id) ((id) & 0x1fc00000)		/* hi 7 bits */
#define CLIENT_ID(id) ((int)(CLIENT_BITS(id) >> CLIENTOFFSET))
#define SERVER_BIT		0x20000000		/* use illegal bit */

typedef int (*DeleteType)(
#if NeedNestedPrototypes
    pointer /*value*/,
    XID /*id*/
#endif
);

extern RESTYPE CreateNewResourceType(
#if NeedFunctionPrototypes
    DeleteType /*deleteFunc*/
#endif
);

extern RESTYPE CreateNewResourceClass(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool InitDeleteFuncs(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool InitClientResources(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern XID FakeClientID(
#if NeedFunctionPrototypes
    int /*client*/
#endif
);

extern int LbxClientIndex(
#if NeedFunctionPrototypes
    XID /*id*/
#endif
);

extern Bool AddResource(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*type*/,
    pointer /*value*/
#endif
);

extern void FreeResource(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*skipDeleteFuncType*/
#endif
);

extern void FreeResourceByType(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*type*/,
    Bool /*skipFree*/
#endif
);

extern Bool ChangeResourceValue(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*rtype*/,
    pointer /*value*/
#endif
);

extern void FreeClientResources(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void FreeAllResources(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool LegalNewID(
#if NeedFunctionPrototypes
    XID /*id*/,
    ClientPtr /*client*/
#endif
);

extern pointer LookupIDByType(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*rtype*/
#endif
);

extern pointer LookupIDByClass(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*classes*/
#endif
);

#endif
