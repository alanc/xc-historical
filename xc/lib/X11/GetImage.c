#include "copyright.h"

/* $Header: XGetImage.c,v 11.15 87/09/01 14:49:28 toddb Locked $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include "Xlibint.h"
#include <errno.h>

#define ROUNDUP(nbytes, pad) (((((nbytes) - 1) + (pad)) / (pad)) * (pad))

extern XImage *XCreateImage();

XImage *XGetImage (dpy, d, x, y, width, height, plane_mask, format)
     register Display *dpy;
     Drawable d;
     int x, y;
     unsigned int width, height;
     long plane_mask;
     int format;	/* either XYFormat or ZFormat */
{
	xGetImageReply rep;
	register xGetImageReq *req;
	char *data;
	long nbytes;
	XImage *image;
	LockDisplay(dpy);
	GetReq (GetImage, req);
	/*
	 * first set up the standard stuff in the request
	 */
	req->drawable = d;
	req->x = x;
	req->y = y;
	req->width = width;
	req->height = height;
	req->planeMask = plane_mask;
	req->format = format;
	
	if (_XReply (dpy, (xReply *) &rep, 0, xFalse) == 0) {
		UnlockDisplay(dpy);
		SyncHandle();
		return (XImage *)NULL;
	}
		
	nbytes = (long)rep.length << 2;
	data = (char *) Xmalloc((unsigned) nbytes);
        _XReadPad (dpy, data, nbytes);
	image = XCreateImage(dpy, _XVIDtoVisual(dpy, rep.visual),
		(int) rep.depth, format, 0, data, width, height,
 		((format == ZPixmap) ? _XGetScanlinePad(dpy, rep.depth)
 				     : dpy->bitmap_pad), 0);
	

	UnlockDisplay(dpy);
	SyncHandle();
	return (image);
}
XGetSubImage(dpy, d, x, y, width, height, plane_mask, format,
               dest_image, dest_x, dest_y)
     register Display *dpy;
     Drawable d;
     int x, y;
     unsigned int width, height;
     unsigned long plane_mask;
     int format;	/* either XYFormat or ZFormat */
     XImage *dest_image;
     int dest_x, dest_y;
{
	XImage *temp_image;
	temp_image = XGetImage(dpy, d, x, y, width, height, 
				plane_mask, format);
	_XSetImage(temp_image, dest_image, dest_x, dest_y);
	_XDestroyImage(temp_image);
}	
