#include "copyright.h"

/* $Header: XMacros.c,v 11.15 87/07/14 16:31:08 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1987	*/

#include "Xlibint.h"
/*
 * This file makes full definitions of routines for each macro.
 * We do not expect C programs to use these, but other languages may
 * need them.
 */

int XConnectionNumber(dpy) Display *dpy; { return (ConnectionNumber(dpy)); }

Window XRootWindow (dpy, scr)
    Display *dpy; int scr;
	{	return (RootWindow(dpy,scr));	}

int XDefaultScreen(dpy) Display *dpy; { return (DefaultScreen(dpy)); }

Window XDefaultRootWindow (dpy)
    Display *dpy;
	{	return (RootWindow(dpy,DefaultScreen(dpy)));	}

Visual *XDefaultVisual(dpy, scr)
    Display *dpy; int scr;
	{	return (DefaultVisual(dpy, scr)); }

GC XDefaultGC(dpy, scr) 
    Display *dpy; int scr;
	{	return (DefaultGC(dpy,scr)); }

long XBlackPixel(dpy, scr) 
    Display *dpy; int scr; 
	{	return (BlackPixel(dpy, scr)); }

long XWhitePixel(dpy, scr) 
    Display *dpy; int scr; 
	{	return (WhitePixel(dpy,scr)); }

long XAllPlanes() { return AllPlanes; }

int XQLength(dpy) Display *dpy; { return (QLength(dpy)); }

int XDisplayWidth(dpy, scr)
    Display *dpy; int scr;
	{ return (DisplayWidth(dpy,scr)); }

int XDisplayHeight(dpy, scr)
    Display *dpy; int scr;
	{ return (DisplayHeight(dpy, scr)); }

int XDisplayWidthMM(dpy, scr)
    Display *dpy; int scr;
	{ return (DisplayWidthMM(dpy, scr)); }

int XDisplayHeightMM(dpy, scr)
    Display *dpy; int scr;
	{ return (DisplayHeightMM(dpy, scr)); }

int XDisplayPlanes(dpy, scr)
    Display *dpy; int scr;
	{ return (DisplayPlanes(dpy, scr)); }

int XDisplayCells(dpy, scr)
    Display *dpy; int scr;
	{ return (DisplayCells (dpy, scr)); }

int XScreenCount(dpy) Display *dpy; { return (ScreenCount(dpy)); }

char *XServerVendor(dpy)  Display *dpy; { return (ServerVendor(dpy)); }

int XProtocolVersion(dpy)  Display *dpy; { return (ProtocolVersion(dpy)); }

int XProtocolRevision(dpy)  Display *dpy; { return (ProtocolRevision(dpy));}

int XVendorRelease(dpy)  Display *dpy; { return (VendorRelease(dpy)); }

char *XDisplayString(dpy)  Display *dpy; { return (DisplayString(dpy)); }

int XDefaultDepth(dpy, scr) 
    Display *dpy; int scr;
	{ return(DefaultDepth(dpy, scr)); }

Colormap XDefaultColormap(dpy, scr)
    Display *dpy; int scr;
	{ return (DefaultColormap(dpy, scr)); }

int XBitmapUnit(dpy) Display *dpy; { return (BitmapUnit(dpy)); }

int XBitmapBitOrder(dpy) Display *dpy; { return (BitmapBitOrder(dpy)); }

int XBitmapPad(dpy) Display *dpy; { return (BitmapPad(dpy)); }

int XImageByteOrder(dpy) Display *dpy; { return (ImageByteOrder(dpy)); }
/*
 * These macros are used to give some sugar to the image routines so that
 * naive people are more comfortable with them.
 */
#undef XDestroyImage(ximage)
XDestroyImage(ximage)
	XImage *ximage;
{
	return((*((ximage)->f.destroy_image))((ximage)));
}
#undef XGetPixel(ximage, x, y)
unsigned long XGetPixel(ximage, x, y)
	XImage *ximage;
	int x, y;
{
	return ((*((ximage)->f.get_pixel))((ximage), (x), (y)));
}
#undef XPutPixel(ximage, x, y, pixel)
int XPutPixel(ximage, x, y, pixel)
	XImage *ximage;
	int x, y;
	unsigned long pixel;
{
	return((*((ximage)->f.put_pixel))((ximage), (x), (y), (pixel)));
}
#undef XSubImage(ximage, x, y, width, height)
XImage *XSubImage(ximage, x, y, width, height)
	XImage *ximage;
	int x, y;
	unsigned int width, height;
{
	return((*((ximage)->f.sub_image))((ximage), (x),
		(y), (width), (height)));
}
#undef XAddPixel(ximage, value)
int XAddPixel(ximage, value)
	XImage *ximage;
	unsigned long value;
{
	return((*((ximage)->f.add_pixel))((ximage), (value)));
}


XNoOp (dpy)
    register Display *dpy;
{
    register xReq *req;

    LockDisplay(dpy);
    GetEmptyReq(NoOperation, req);

    UnlockDisplay(dpy);
}

