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

/***********************************************************************
 *
 * $XConsortium: util.c,v 1.14 89/04/07 11:56:46 toml Exp $
 *
 * utility routines for twm
 *
 * 28-Oct-87 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: util.c,v 1.14 89/04/07 11:56:46 toml Exp $";
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
 *
 ***********************************************************************
 */

void
MoveOutline(root, x, y, width, height)
    Window root;
    int x, y, width, height;
{
    static int	lastx = 0;
    static int	lasty = 0;
    static int	lastWidth = 0;
    static int	lastHeight = 0;
    int		xl, xr, yt, yb;
    int		xthird, ythird;
    XSegment	outline[16];
    XSegment	*r = outline;

    if (x == lastx && y == lasty && width == lastWidth && height == lastHeight)
	return;
    
    xthird = lastWidth/3;
    ythird = lastHeight/3;
    xl = lastx;
    xr = lastx + lastWidth - 1;
    yt = lasty;
    yb = lasty + lastHeight - 1;

    if (lastWidth || lastHeight)
    {
	r->x1 = xl;
	r->y1 = yt;
	r->x2 = xr;
	r++->y2 = yt;

	r->x1 = xl;
	r->y1 = yb;
	r->x2 = xr;
	r++->y2 = yb;

	r->x1 = xl;
	r->y1 = yt;
	r->x2 = xl;
	r++->y2 = yb;

	r->x1 = xr;
	r->y1 = yt;
	r->x2 = xr;
	r++->y2 = yb;

	r->x1 = xl + xthird;
	r->y1 = yt;
	r->x2 = r->x1;
	r++->y2 = yb;

	r->x1 = xl + (2 * xthird);
	r->y1 = yt;
	r->x2 = r->x1;
	r++->y2 = yb;

	r->x1 = xl;
	r->y1 = yt + ythird;
	r->x2 = xr;
	r->y2 = r->y1;
	r++;

	r->x1 = xl;
	r->y1 = yt + (2 * ythird);
	r->x2 = xr;
	r->y2 = r->y1;
	r++;
    }

    lastx = x;
    lasty = y;
    lastWidth = width;
    lastHeight = height;
    xthird = lastWidth/3;
    ythird = lastHeight/3;
    xl = lastx;
    xr = lastx + lastWidth - 1;
    yt = lasty;
    yb = lasty + lastHeight - 1;

    if (lastWidth || lastHeight)
    {
	r->x1 = xl;
	r->y1 = yt;
	r->x2 = xr;
	r++->y2 = yt;

	r->x1 = xl;
	r->y1 = yb;
	r->x2 = xr;
	r++->y2 = yb;

	r->x1 = xl;
	r->y1 = yt;
	r->x2 = xl;
	r++->y2 = yb;

	r->x1 = xr;
	r->y1 = yt;
	r->x2 = xr;
	r++->y2 = yb;

	r->x1 = xl + xthird;
	r->y1 = yt;
	r->x2 = r->x1;
	r++->y2 = yb;

	r->x1 = xl + (2 * xthird);
	r->y1 = yt;
	r->x2 = r->x1;
	r++->y2 = yb;

	r->x1 = xl;
	r->y1 = yt + ythird;
	r->x2 = xr;
	r->y2 = r->y1;
	r++;

	r->x1 = xl;
	r->y1 = yt + (2 * ythird);
	r->x2 = xr;
	r->y2 = r->y1;
	r++;
    }
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

#ifdef NOT_USED_ANYMORE
/***********************************************************************
 *
 *  Procedure:
 *	MakeCenteredPixmap - make a pixmap centered in a space
 *
 *  Returned Value:
 *	pid	- the pixmap id
 *
 *  Inputs:
 *	w	- the window to associate the pixmap with
 *	gc	- the graphics context to use
 *	cleargc - graphics context for clearing pixmap background;  can pass
 *		     NULL if width <= pwidth && height <= pheight
 *	width	- the width of the pixmap to create
 *	height  - the height of the pixmap to create
 *	data	- pointer to the pixmap data
 *	pwidth	- the width of the pixmap
 *	pheight	- the height of the pixmap
 *
 ***********************************************************************
 */

Pixmap
MakeCenteredPixmap(w, gc, cleargc, width, height, data, pwidth, pheight, pdepth)
    Drawable w;
    GC gc, cleargc;
    int width, height;
    short *data;
    int pwidth, pheight, pdepth;
{
    XImage ximage;
    Pixmap pid;
    int dx, dy;

    pid = XCreatePixmap(dpy, w, width, height, pdepth);
    if (cleargc)
	XFillRectangle(dpy, pid, cleargc, 0, 0, width, height);

    ximage.height = pheight;
    ximage.width = pwidth;
    ximage.xoffset = 0;
    ximage.format = XYBitmap;
    ximage.data = (char *) data;
    ximage.byte_order = LSBFirst;
    ximage.bitmap_unit = 16;
    ximage.bitmap_bit_order = LSBFirst;
    ximage.bitmap_pad = 16;
    ximage.bytes_per_line = (pwidth + 7) / 8;
    ximage.depth = 1;

    dx = (width - pwidth) / 2;
    dy = (height - pheight) / 2;

    XPutImage(dpy, pid, gc, &ximage, 0, 0, dx, dy, pwidth, pheight);
    return (pid);
}

/***********************************************************************
 *
 *  Procedure:
 *	MakePixmap - make a pixmap
 *
 *  Returned Value:
 *	pid	- the pixmap id
 *
 *  Inputs:
 *	w	- the window to associate the pixmap with
 *	gc	- the graphics context to use
 *	data	- pointer to the pixmap data
 *	width	- the width of the pixmap
 *	height	- the height of the pixmap
 *
 ***********************************************************************
 */

Pixmap
MakePixmap(w, gc, data, width, height)
    Drawable w;
    GC gc;
    short *data;
    int width, height;
{
    return (MakeCenteredPixmap(w, gc, NULL, width, height, data,
	width, height, Scr->d_depth));
}

/***********************************************************************
 *
 *  Procedure:
 *	Make1Pixmap - make a pixmap of depth 1
 *
 *  Returned Value:
 *	pid	- the pixmap id
 *
 *  Inputs:
 *	w	- the window to associate the pixmap with
 *	gc	- the graphics context to use
 *	data	- pointer to the pixmap data
 *	width	- the width of the pixmap
 *	height	- the height of the pixmap
 *
 ***********************************************************************
 */

Pixmap
Make1Pixmap(w, gc, data, width, height)
    Drawable w;
    GC gc;
    short *data;
    int width, height;
{
    return (MakeCenteredPixmap(w, gc, NULL, width, height, data,
	width, height, 1));
}
#endif NOT_USED_ANYMORE

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

#ifndef TOM
    if (!Scr->FirstTime)
	return;
#endif

    if (Scr->Monochrome != kind)
	return;

    if (!XParseColor(dpy, Scr->CMap, name, &color))
    {
	fprintf(stderr, "twm: invalid color \"%s\"\n", name);
	return;
    }

    if (!XAllocColor(dpy, Scr->CMap, &color))
    {
	fprintf(stderr, "twm: invalid color \"%s\"\n", name);
	return;
    }

    *what = color.pixel;
}

GetFont(font)
MyFont *font;
{
    char *deffontname = "fixed";
#ifndef TOM
    if (!Scr->FirstTime)
	return;
#endif

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
    font->y = font->font->ascent + 1;
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
#endif NOPUTENV
