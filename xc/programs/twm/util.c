/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute Of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/


/***********************************************************************
 *
 * $XConsortium: util.c,v 1.21 89/07/18 17:15:35 jim Exp $
 *
 * utility routines for twm
 *
 * 28-Oct-87 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: util.c,v 1.21 89/07/18 17:15:35 jim Exp $";
#endif

#include <stdio.h>
#include "twm.h"
#include "util.h"
#include "gram.h"
#include "screen.h"

int HotX, HotY;

/***********************************************************************
 *
 *  Procedure:
 *	MoveOutline - move a window outline
 *
 *  Inputs:
 *	root	    - the window we are outlining
 *	x	    - upper left x coordinate
 *	y	    - upper left y coordinate
 *	width	    - the width of the rectangle
 *	height	    - the height of the rectangle
 *      bw          - the border width of the frame
 *      th          - title height
 *
 ***********************************************************************
 */

/* ARGSUSED */
void MoveOutline(root, x, y, width, height, bw, th)
    Window root;
    int x, y, width, height, bw, th;
{
    static int	lastx = 0;
    static int	lasty = 0;
    static int	lastWidth = 0;
    static int	lastHeight = 0;
    static int	lastBW = 0;
    static int	lastTH = 0;
    int		xl, xr, yt, yb, xinnerl, xinnerr, yinnert, yinnerb;
    int		xthird, ythird;
    XSegment	outline[18];
    register XSegment	*r;

    if (x == lastx && y == lasty && width == lastWidth && height == lastHeight
	&& lastBW == bw && th == lastTH)
	return;
    
    r = outline;

#define DRAWIT() \
    if (lastWidth || lastHeight)			\
    {							\
	xl = lastx;					\
	xr = lastx + lastWidth - 1;			\
	yt = lasty;					\
	yb = lasty + lastHeight - 1;			\
	xinnerl = xl + lastBW;				\
	xinnerr = xr - lastBW;				\
	yinnert = yt + lastTH + lastBW;			\
	yinnerb = yb - lastBW;				\
	xthird = (xinnerr - xinnerl) / 3;		\
	ythird = (yinnerb - yinnert) / 3;		\
							\
	r->x1 = xl;					\
	r->y1 = yt;					\
	r->x2 = xr;					\
	r->y2 = yt;					\
	r++;						\
							\
	r->x1 = xl;					\
	r->y1 = yb;					\
	r->x2 = xr;					\
	r->y2 = yb;					\
	r++;						\
							\
	r->x1 = xl;					\
	r->y1 = yt;					\
	r->x2 = xl;					\
	r->y2 = yb;					\
	r++;						\
							\
	r->x1 = xr;					\
	r->y1 = yt;					\
	r->x2 = xr;					\
	r->y2 = yb;					\
	r++;						\
							\
	r->x1 = xinnerl + xthird;			\
	r->y1 = yinnert;				\
	r->x2 = r->x1;					\
	r->y2 = yinnerb;				\
	r++;						\
							\
	r->x1 = xinnerl + (2 * xthird);			\
	r->y1 = yinnert;				\
	r->x2 = r->x1;					\
	r->y2 = yinnerb;				\
	r++;						\
							\
	r->x1 = xinnerl;				\
	r->y1 = yinnert + ythird;			\
	r->x2 = xinnerr;				\
	r->y2 = r->y1;					\
	r++;						\
							\
	r->x1 = xinnerl;				\
	r->y1 = yinnert + (2 * ythird);			\
	r->x2 = xinnerr;				\
	r->y2 = r->y1;					\
	r++;						\
							\
	if (lastTH != 0) {				\
	    r->x1 = xl;					\
	    r->y1 = yt + lastTH;			\
	    r->x2 = xr;					\
	    r->y2 = r->y1;				\
	    r++;					\
	}						\
    }

    /* undraw the old one, if any */
    DRAWIT ();

    lastx = x;
    lasty = y;
    lastWidth = width;
    lastHeight = height;
    lastBW = bw;
    lastTH = th;

    /* draw the new one, if any */
    DRAWIT ();

#undef DRAWIT


    if (r != outline)
    {
	XDrawSegments(dpy, root, Scr->DrawGC, outline, r - outline);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	Zoom - zoom in or out of an icon
 *
 *  Inputs:
 *	wf	- window to zoom from
 *	wt	- window to zoom to
 *
 ***********************************************************************
 */

void
Zoom(wf, wt)
{
    int fx, fy, fw, fh;
    int tx, ty, tw, th;
    int xl, yt, xr, yb;
    int dx, dy, dw, dh;
    int w, h, i;
    XSegment	outline[4];

    if (!Scr->DoZoom)
	return;

    if (wf == NULL || wt == NULL)
	return;

    XGetGeometry(dpy, wf, &JunkRoot, &fx, &fy, &fw, &fh, &JunkBW, &JunkDepth);
    XGetGeometry(dpy, wt, &JunkRoot, &tx, &ty, &tw, &th, &JunkBW, &JunkDepth);

    dx = (tx - fx) / Scr->ZoomCount;
    dy = (ty - fy) / Scr->ZoomCount;
    dw = (tw - fw) / Scr->ZoomCount;
    dh = (th - fh) / Scr->ZoomCount;

    xl = fx;
    yt = fy;
    xr = fx + fw;
    yb = fy + fh;
    w = fw;
    h = fh;

    for (i = 0; i < Scr->ZoomCount; i++)
    {
	outline[0].x1 = xl;
	outline[0].y1 = yt;
	outline[0].x2 = xr;
	outline[0].y2 = yt;

	outline[1].x1 = xr;
	outline[1].y1 = yt;
	outline[1].x2 = xr;
	outline[1].y2 = yb;

	outline[2].x1 = xr;
	outline[2].y1 = yb;
	outline[2].x2 = xl;
	outline[2].y2 = yb;

	outline[3].x1 = xl;
	outline[3].y1 = yb;
	outline[3].x2 = xl;
	outline[3].y2 = yt;

	XDrawSegments(dpy, Scr->Root, Scr->DrawGC, outline, 4);

	w += dw;
	h += dh;
	xl += dx;
	yt += dy;
	xr = xl + w;
	yb = yt + h;
    }

    xl = fx;
    yt = fy;
    xr = fx + fw;
    yb = fy + fh;
    w = fw;
    h = fh;

    for (i = 0; i < Scr->ZoomCount; i++)
    {
	outline[0].x1 = xl;
	outline[0].y1 = yt;
	outline[0].x2 = xr;
	outline[0].y2 = yt;

	outline[1].x1 = xr;
	outline[1].y1 = yt;
	outline[1].x2 = xr;
	outline[1].y2 = yb;

	outline[2].x1 = xr;
	outline[2].y1 = yb;
	outline[2].x2 = xl;
	outline[2].y2 = yb;

	outline[3].x1 = xl;
	outline[3].y1 = yb;
	outline[3].x2 = xl;
	outline[3].y2 = yt;

	XDrawSegments(dpy, Scr->Root, Scr->DrawGC, outline, 4);

	w += dw;
	h += dh;
	xl += dx;
	yt += dy;
	xr = xl + w;
	yb = yt + h;
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	ExpandFilename - expand the tilde character to HOME
 *		if it is the first character of the filename
 *
 *  Returned Value:
 *	a pointer to the new name
 *
 *  Inputs:
 *	name	- the filename to expand
 *
 ***********************************************************************
 */

char *
ExpandFilename(name)
char *name;
{
    char *newname;

    if (name[0] != '~')
	return (name);

    newname = (char *)malloc(HomeLen + strlen(name) + 2);
    sprintf(newname, "%s/%s", Home, &name[1]);

    return (newname);
}

/***********************************************************************
 *
 *  Procedure:
 *	GetUnknownIcon - read in the bitmap file for the unknown icon
 *
 *  Inputs:
 *	name - the filename to read
 *
 ***********************************************************************
 */

void
GetUnknownIcon(name)
char *name;
{
    if ((Scr->UnknownPm = GetBitmap(name)) != NULL)
    {
	XGetGeometry(dpy, Scr->UnknownPm, &JunkRoot, &JunkX, &JunkY,
	    &Scr->UnknownWidth, &Scr->UnknownHeight, &JunkBW, &JunkDepth);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GetBitmap - read in a bitmap file
 *
 *  Returned Value:
 *	the pixmap associated with the bitmap
 *
 *  Inputs:
 *	name	- the filename to read
 *
 ***********************************************************************
 */

Pixmap
GetBitmap(name)
char *name;
{
    char *bigname;
    int status;
    Pixmap pm;

    if  (name == NULL)
	return (NULL);

    name = ExpandFilename(name);
    bigname = name;

    status = XReadBitmapFile(dpy, Scr->Root, bigname, &JunkWidth,
	&JunkHeight, &pm, &HotX, &HotY);

    if (status != BitmapSuccess && Scr->IconDirectory && name[0] != '/')
    {
	bigname = (char *)malloc(strlen(name) + strlen(Scr->IconDirectory) + 2);
	sprintf(bigname, "%s/%s", Scr->IconDirectory, name);
	status = XReadBitmapFile(dpy, Scr->Root, bigname, &JunkWidth,
	    &JunkHeight, &pm, &HotX, &HotY);
    }

    if (status != BitmapSuccess && name[0] != '/')
    {
	bigname = (char *)malloc(strlen(name) + strlen(BITMAPS) + 2);
	sprintf(bigname, "%s/%s", BITMAPS, name);
	status = XReadBitmapFile(dpy, Scr->Root, bigname, &JunkWidth,
	    &JunkHeight, &pm, &HotX, &HotY);
    }

    switch(status)
    {
	case BitmapSuccess:
	    break;

	case BitmapFileInvalid:
	    fprintf(stderr, ".twmrc: invalid bitmap file \"%s\"\n", bigname);
	    break;

	case BitmapNoMemory:
	    fprintf(stderr, ".twmrc: out of memory \"%s\"\n", bigname);
	    break;

	case BitmapOpenFailed:
	    fprintf(stderr, ".twmrc: failed to open bitmap file \"%s\"\n",
		bigname);
	    break;

	default:
	    fprintf(stderr,".twmrc: bitmap error = 0x%x on file \"%s\"\n",
		status, bigname);
	    break;
    }

    if (status != BitmapSuccess)
	return (NULL);

    return (pm);
}

int
GetColor(kind, what, name)
int kind;
int *what;
char *name;
{
    XColor color, junkcolor;
    Status stat;

#ifndef TOM
    if (!Scr->FirstTime)
	return;
#endif

    if (Scr->Monochrome != kind)
	return;

    /*
     * small hack to avoid extra roundtrip for color allocation
     */
    if (!((name[0] == '#')
	  ? (XParseColor (dpy, Scr->CMap, name, &color) &&
	     XAllocColor (dpy, Scr->CMap, &color))
	  : XAllocNamedColor (dpy, Scr->CMap, name, &color, &junkcolor))) {
	fprintf (stderr, "twm: invalid color \"%s\"\n", name);
	return;
    }

    *what = color.pixel;
}

GetFont(font)
MyFont *font;
{
    char *deffontname = "fixed";

    if (font->font != NULL)
	XFreeFont(dpy, font->font);

    if ((font->font = XLoadQueryFont(dpy, font->name)) == NULL)
    {
	if (Scr->DefaultFont.name) {
	    deffontname = Scr->DefaultFont.name;
	}
	if ((font->font = XLoadQueryFont(dpy, deffontname)) == NULL)
	{
	    fprintf(stderr, "twm: couldn't open font \"%s\" or \"%s\"\n",
		    font->name, deffontname);
	    exit(1);
	}

    }
    font->height = font->font->ascent + font->font->descent;
    font->y = font->font->ascent;
}


#ifdef NOPUTENV
/*
 * define our own putenv() if the system doesn't have one.
 * putenv(s): place s (a string of the form "NAME=value") in
 * the environment; replacing any existing NAME.  s is placed in
 * environment, so if you change s, the environment changes (like
 * putenv on a sun).  Binding removed if you putenv something else
 * called NAME.
 */
int
putenv(s)
    char *s;
{
    extern char *index();
    char *v;
    int varlen, idx;
    extern char **environ;
    char **newenv;
    static int virgin = 1; /* true while "environ" is a virgin */

    v = index(s, '=');
    if(v == 0)
	return 0; /* punt if it's not of the right form */
    varlen = (v + 1) - s;

    for (idx = 0; environ[idx] != 0; idx++) {
	if (strncmp(environ[idx], s, varlen) == 0) {
	    if(v[1] != 0) { /* true if there's a value */
		environ[idx] = s;
		return 0;
	    } else {
		do {
		    environ[idx] = environ[idx+1];
		} while(environ[++idx] != 0);
		return 0;
	    }
	}
    }
    
    /* add to environment (unless no value; then just return) */
    if(v[1] == 0)
	return 0;
    if(virgin) {
	register i;

	newenv = (char **) malloc((unsigned) ((idx + 2) * sizeof(char*)));
	if(newenv == 0)
	    return -1;
	for(i = idx-1; i >= 0; --i)
	    newenv[i] = environ[i];
	virgin = 0;     /* you're not a virgin anymore, sweety */
    } else {
	newenv = (char **) realloc((char *) environ,
				   (unsigned) ((idx + 2) * sizeof(char*)));
	if (newenv == 0)
	    return -1;
    }

    environ = newenv;
    environ[idx] = s;
    environ[idx+1] = 0;
    
    return 0;
}
#endif /* NOPUTENV */
