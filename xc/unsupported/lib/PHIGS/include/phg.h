/* $XConsortium$ */

/***********************************************************
Copyright 1989, 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#ifndef PHG_H_INCLUDED
#define PHG_H_INCLUDED

/* Main INTERNAL header file for the PHIGS library */

#include <math.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Cardinals.h>

/* Xos.h defines index to be strchr.  We don't need to do that because all
 * uses of index and strchr are #ifdef-ed in the SI API code.
 */
#ifdef index
#undef index
#endif

/* Include these after the #undef of index so that all their uses of the
 * work "index" don't get modified to strchr.  This is a workaround for the
 * Xlib hack in Xos.h.
 */
#include "PEX.h"
#include "PEXproto.h"

/* declare these here, <malloc.h> doesn't exist on all systems */
extern  char    *malloc();
extern  char    *realloc();

/* Not all environments declare errno in <errno.h> (Sony, in particular) */
extern int	 errno;
/* Not all environments' <math.h> define M_PI, so if not defined, define */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef MAXFLOAT
#define MAXFLOAT	((float)1.701411733192644299e+38)
#endif

/* Not all environments' <math.h> define _ABS, so if not defined, define */
#ifndef _ABS
#define _ABS(x) ((x) < 0 ? -(x) : (x))
#endif

/* Only include common files that most everybody uses. */

#include "phigs.h"
#include "phgtype.h"
#include "util.h"
#include "cp_ops.h"
#include "err.h"
#include "ws_type.h"
#include "phgargs.h"
#include "phgretdata.h"
#include "psl.h"
#include "errnum.h"
#include "assure.h"

#endif
