/* $XConsortium: validate.h,v 5.3 93/09/29 17:07:15 dpw Exp $ */

/*
Copyright 1989 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.  M.I.T. makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
*/

#ifndef VALIDATE_H
#define VALIDATE_H

#include "miscstruct.h"
#include "regionstr.h"

typedef enum { VTOther, VTStack, VTMove, VTUnmap, VTMap } VTKind;

/* union _Validate is now device dependent; see mivalidate.h for an example */
typedef union _Validate *ValidatePtr;

#define UnmapValData ((ValidatePtr)1)

#endif /* VALIDATE_H */
