/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name  of Evans & Sutherland  not be used in advertising or publi-    **/
/**    city pertaining to distribution  of the software without  specif-    **/
/**    ic, written prior permission.                                        **/
/**                                                                         **/
/**    EVANS  & SUTHERLAND  DISCLAIMS  ALL  WARRANTIES  WITH  REGARD  TO    **/
/**    THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILI-    **/
/**    TY AND FITNESS, IN NO EVENT SHALL EVANS &  SUTHERLAND  BE  LIABLE    **/
/**    FOR  ANY  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY  DAM-    **/
/**    AGES  WHATSOEVER RESULTING FROM  LOSS OF USE,  DATA  OR  PROFITS,    **/
/**    WHETHER   IN  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    **/
/**    ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE  OR PER-    **/
/**    FORMANCE OF THIS SOFTWARE.                                           **/
/*****************************************************************************/

/**********************************************************************
 *
 * $XConsortium: gc.c,v 1.11 89/04/12 18:55:38 jim Exp $
 *
 * Open the fonts and create the GCs
 *
 * 31-Mar-88 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: gc.c,v 1.11 89/04/12 18:55:38 jim Exp $";
#endif /* lint */

#include <stdio.h>
#include "twm.h"
#include "util.h"
#include "screen.h"

static XFontStruct *dfont;		/* my default font */
static char *dfontname;

/***********************************************************************
 *
 *  Procedure:
 *	CreateGCs - open fonts and create all the needed GC's.  I only
 *		    want to do this once, hence the first_time flag.
 *
 ***********************************************************************
 */

void
CreateGCs()
{
    static ScreenInfo *prevScr = NULL;
    XGCValues	    gcv;
    unsigned long   gcm, mask;

    if (!Scr->FirstTime || prevScr == Scr)
	return;

    prevScr = Scr;

    /* create GC's */

    if (Scr->ReverseVideo)
    {
	Scr->DefaultC.back = Scr->Black;
	Scr->DefaultC.fore = Scr->White;
    }
    else
    {
	Scr->DefaultC.fore = Scr->Black;
	Scr->DefaultC.back = Scr->White;
    }

    gcm = 0;
    gcm |= GCFunction;	    gcv.function = GXxor;
    gcm |= GCLineWidth;	    gcv.line_width = 0;
    gcm |= GCForeground;    gcv.foreground = 0xfd;
    gcm |= GCSubwindowMode; gcv.subwindow_mode = IncludeInferiors;

    Scr->DrawGC = XCreateGC(dpy, Scr->Root, gcm, &gcv);

    gcm = 0;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    /*
     * Prevent GraphicsExpose and NoExpose events.  We'd only get NoExpose
     * events anyway;  they cause BadWindow errors from XGetWindowAttributes
     * call in FindScreenInfo (events.c) (since drawable is a pixmap).
     */
    gcm |= GCGraphicsExposures;  gcv.graphics_exposures = False;
    gcm |= GCLineWidth;	    gcv.line_width = 0;

    Scr->NormalGC = XCreateGC(dpy, Scr->Root, gcm, &gcv);
}
