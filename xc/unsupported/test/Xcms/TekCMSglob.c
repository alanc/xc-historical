/*
 *	Copyright (c) 1988, Tektronix, Inc.
 *	Confidential -- All proprietary rights reserved.
 *	Licensed Material -- Property of Tektronix, Inc.
 *
 *	This program is the property of Tektronix, Inc. or others
 *	from whom Tektronix has obtained a licensing right, and is
 *	considered by Tektronix to be confidential.  It is protected
 *	by U.S. copyright law as an unpublished work and is
 *	furnished pursuant to a written license agreement.  It may
 *	not be used, copied or otherwise reproduced, or disclosed to
 *	others except in accordance with the terms and conditions of
 *	that agreement.
 *
 *	NAME
 *		globals.c -- globals
 *
 *	DESCRIPTION
 */
#ifndef LINT
static char *copy_notice = "Copyright 1988 Tektronix, Inc.";
#ifdef RCS_ID
static char *rcsid=  "$Header: TekCMSglob.c,v 1.4 91/01/22 22:03:28 alt Exp $";
#endif RCS_ID
#endif LINT


/*
 *      EXTERNAL INCLUDES
 *              Include files that must be exported to any package or
 *              program using this package.
 */
/* this file contains the typedef for LtDefineEntry */
#include "LibTest.h"


/*
 *      INTERNAL INCLUDES
 *              Include files that need NOT be exported to any package or
 *              program using this package.
 */
#include "TekCMS.h"

/*
 *	INTERNALS
 *		Declarations that are local to this module.
 *		(ignored by 'autohdr').
 */

/*
 *	EXTERNALS
 *		Declarations that are needed by calling modules.
 *		When using 'autohdr', these declaration will be placed
 *		in the resulting header file.
 */

#define THOUSAND	1000

LtDefineEntry ErrorTbl[] = {
    "Success",		Success,
    "BadRequest",	BadRequest,
    "BadValue",		BadValue,
    "BadWindow",	BadWindow,
    "BadPixmap",	BadPixmap,
    "BadAtom",		BadAtom,
    "BadCursor",	BadCursor,
    "BadFont",		BadFont,
    "BadMatch",		BadMatch,
    "BadDrawable",	BadDrawable,
    "BadAccess",	BadAccess,
    "BadAlloc",		BadAlloc,
    "BadColor",		BadColor,
    "BadGC",		BadGC,
    "BadIDChoice",	BadIDChoice,
    "BadName",		BadName,
    "BadLength",	BadLength,
    "BadImplementation",    BadImplementation,
    "FirstExtensionError",  FirstExtensionError,
    "LastExtensionError",   LastExtensionError,
    "",			    0
};

LtDefineEntry AllocTbl[] = {
    "AllocNone",	AllocNone,
    "AllocAll",		AllocAll,
    "illegala1",	THOUSAND,
    "",			0
};

LtDefineEntry FormatTbl[] = {
    "RGBi",		XCMS_RGBi_FORMAT,
    "RGB",		XCMS_RGB_FORMAT,
    "UNDEFINED",	XCMS_UNDEFINED_FORMAT,
    "CIEXYZ",		XCMS_CIEXYZ_FORMAT,
    "CIExyY",		XCMS_CIExyY_FORMAT,
    "CIEuvY",		XCMS_CIEuvY_FORMAT,
    "CIELab",		XCMS_CIELab_FORMAT,
    "CIELuv",		XCMS_CIELuv_FORMAT,
    "TekHVC",		XCMS_TekHVC_FORMAT,
    "",			0
};
