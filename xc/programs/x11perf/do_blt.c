/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#include "x11perf.h"
#include <stdio.h>

#define NUMPOINTS 100

static Pixmap   pix;
static XImage   *image;
static XPoint   points[NUMPOINTS];
static XSegment *segsa, *segsb;

#define NegMod(x, y) ((y) - (((-x)-1) % (7)) - 1)

int InitScroll(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i, x, y;

    points[0].x = points[0].y = y = 0;
    for (i = 1; i != NUMPOINTS/2; i++) {    
	if (i & 1) {
	    points[i].x = WIDTH-1;
	} else {
	    points[i].x = 0;
	}
	y += HEIGHT / (NUMPOINTS/2);
	points[i].y = y;
    }
    
    x = 0;
    for (i = NUMPOINTS/2; i!= NUMPOINTS; i++) {
	if (i & 1) {
	    points[i].y = HEIGHT-1;
	} else {
	    points[i].y = 0;
	}
	x += WIDTH / (NUMPOINTS/2);
	points[i].x = x;
    }
	
    XDrawLines(xp->d, xp->w, xp->fggc, points, NUMPOINTS, CoordModeOrigin);
    return reps;
}

void DoScroll(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i, size, x, y, xorg, yorg, delta;

    size = p->special;
    xorg = 0;   yorg = 0;
    x    = 0;   y    = 0;
    if (xp->version == VERSION1_2) {
	delta = 1;
    } else {
	/* Version 1.2 only scrolled up by 1 scanline, which made hardware
	   using page-mode access to VRAM look better on paper than it would
	   perform in a more realistic scroll.  So we've changed to scroll by
	   the height of the 6x13 fonts. */
	delta = 13;
    }

    for (i = 0; i != reps; i++) {
	XCopyArea(xp->d, xp->w, xp->w, xp->fggc, x, y + delta,
	    size, size, x, y);
	y += size;
	if (y + size + delta > HEIGHT) {
	    yorg += delta;
	    if (yorg >= size || yorg + size + delta > HEIGHT) {
		yorg = 0;
		xorg++;
		if (xorg >= size || xorg + size > WIDTH) {
		    xorg = 0;
		}
	    }
	    y = yorg;
	    x += size;
	    if (x + size > WIDTH) {
		x = xorg;
	    }
	}
    }
}

void MidScroll(xp, p)
    XParms  xp;
    Parms   p;
{
    XClearWindow(xp->d, xp->w);
    XDrawLines(xp->d, xp->w, xp->fggc, points, NUMPOINTS, CoordModeOrigin);
}

void EndScroll(xp, p)
    XParms  xp;
    Parms   p;
{
}

static void InitCopyLocations(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int x1, y1, x2, y2, size, i;
    int xinc, yinc;
    int width, height;

    /* Try to exercise all alignments of src and destination equally, as well
       as all 4 top-to-bottom/bottom-to-top, left-to-right, right-to-left
       copying directions.  Computation done here just to make sure slow
       machines aren't measuring anything but the XCopyArea calls.
    */
    size = p->special;
    xinc = (size & ~3) + 1;
    yinc = xinc + 3;

    width = (WIDTH - size) & ~31;
    height = (HEIGHT - size) & ~31;
    
    x1 = 0;
    y1 = 0;
    x2 = width;
    y2 = height;
    
    segsa = (XSegment *)malloc(reps * sizeof(XSegment));
    segsb = (XSegment *)malloc(reps * sizeof(XSegment));
    for (i = 0; i != reps; i++) {
	segsa[i].x1 = x1;
	segsa[i].y1 = y1;
	segsa[i].x2 = x2;
	segsa[i].y2 = y2;

	/* Move x2, y2, location backward */
	x2 -= xinc;
	if (x2 < 0) {
	    x2 = NegMod(x2, width);
	    y2 -= yinc;
	    if (y2 < 0) {
		y2 = NegMod(y2, height);
	    }
	}

	segsb[i].x1 = x1;
	segsb[i].y1 = y1;
	segsb[i].x2 = x2;
	segsb[i].y2 = y2;

	/* Move x1, y1 location forward */
	x1 += xinc;
	if (x1 > width) {
	    x1 %= 32;
	    y1 += yinc;
	    if (y1 > height) {
		y1 %= 32;
	    }
	}
    } /* end for */
}


int InitCopyWin(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    (void) InitScroll(xp, p, reps);
    InitCopyLocations(xp, p, reps);
    return reps;
}

int InitCopyPix(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    GC		pixgc;
    (void) InitCopyWin(xp, p, reps);

    /* Create pixmap to write stuff into, and initialize it */
    pix = XCreatePixmap(xp->d, xp->w, WIDTH, HEIGHT, xp->vinfo.depth);
    pixgc = XCreateGC(xp->d, pix, 0, 0);
    /* need a gc with GXcopy cos pixmaps contain junk on creation. mmm */
    XCopyArea(xp->d, xp->w, pix, pixgc, 0, 0, WIDTH, HEIGHT, 0, 0);
    XFreeGC(xp->d, pixgc);
    return reps;
}

Bool InitGetImage(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    (void) InitCopyWin(xp, p, reps);

    /* Create image to stuff bits into */
    image = XGetImage(xp->d, xp->w, 0, 0, WIDTH, HEIGHT, ~0,
		      p->font==0?ZPixmap:XYPixmap);
    if(image==0){
	printf("XGetImage failed\n");
	return False;
    }	
    return reps;
}

Bool InitPutImage(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    if(!InitGetImage(xp, p, reps))return False;
    XClearWindow(xp->d, xp->w);
    return reps;
}

static void CopyArea(xp, p, reps, src, dst)
    XParms  xp;
    Parms   p;
    int     reps;
    Drawable src, dst;
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XCopyArea(xp->d, src, dst, xp->fggc,
	    sa->x1, sa->y1, size, size, sa->x2, sa->y2);
	XCopyArea(xp->d, src, dst, xp->fggc,
	    sa->x2, sa->y2, size, size, sa->x1, sa->y1);
	XCopyArea(xp->d, src, dst, xp->fggc,
	    sb->x2, sb->y2, size, size, sb->x1, sb->y1);
	XCopyArea(xp->d, src, dst, xp->fggc,
	    sb->x1, sb->y1, size, size, sb->x2, sb->y2);
    }
}

void DoCopyWinWin(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    CopyArea(xp, p, reps, xp->w, xp->w);
}

void DoCopyPixWin(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    CopyArea(xp, p, reps, pix, xp->w);
}

void DoCopyWinPix(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    CopyArea(xp, p, reps, xp->w, pix);
    xp->p = pix;	/* HardwareSync will now sync on pixmap */
}

void DoCopyPixPix(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    CopyArea(xp, p, reps, pix, pix);
    xp->p = pix;	/* HardwareSync will now sync on pixmap */
}

void DoGetImage(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i, size;
    XSegment *sa, *sb;
    int format;

    size = p->special;
    format = (p->font == 0) ? ZPixmap : XYPixmap;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XDestroyImage(image);
	image = XGetImage(xp->d, xp->w, sa->x1, sa->y1, size, size,
	    ~0, format);
	if (image) XDestroyImage(image);
	image = XGetImage(xp->d, xp->w, sa->x2, sa->y2, size, size,
	    ~0, format);
	if (image) XDestroyImage(image);
	image = XGetImage(xp->d, xp->w, sb->x2, sb->y2, size, size,
	    ~0, format);
	if (image) XDestroyImage(image);
	image = XGetImage(xp->d, xp->w, sb->x1, sb->y1, size, size,
	    ~0, format);
/*

One might expect XGetSubImage to be slightly faster than XGetImage.  Go look
at the code in Xlib.  MIT X11R3 ran approximately 30 times slower for a 500x500
rectangle.

	(void) XGetSubImage(xp->d, xp->w, sa->x1, sa->y1, size, size,
	    ~0, ZPixmap, image, sa->x2, sa->y2);
	(void) XGetSubImage(xp->d, xp->w, sa->x2, sa->y2, size, size,
	    ~0, ZPixmap, image, sa->x1, sa->y1);
	(void) XGetSubImage(xp->d, xp->w, sb->x2, sb->y2, size, size,
	    ~0, ZPixmap, image, sb->x2, sb->y2);
	(void) XGetSubImage(xp->d, xp->w, sb->x1, sb->y1, size, size,
	    ~0, ZPixmap, image, sb->x2, sb->y2);
*/
    }
}

void DoPutImage(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XPutImage(xp->d, xp->w, xp->fggc, image,
	    sa->x1, sa->y1, sa->x2, sa->y2, size, size);
	XPutImage(xp->d, xp->w, xp->fggc, image,
	    sa->x2, sa->y2, sa->x1, sa->y1, size, size);
	XPutImage(xp->d, xp->w, xp->fggc, image,
	    sb->x2, sb->y2, sb->x2, sb->y2, size, size);
	XPutImage(xp->d, xp->w, xp->fggc, image,
	    sb->x1, sb->y1, sb->x2, sb->y2, size, size);
    }
}

#ifdef MITSHM

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

static XImage		shm_image;
static XShmSegmentInfo	shminfo;

static int haderror;
static int (*origerrorhandler)();
shmerrorhandler(d,e)
Display *d;
XErrorEvent *e;
{
    haderror++;
    if(e->error_code==BadAccess)fprintf(stderr,"failed to attach shared memory\n");
    else (*origerrorhandler)(d,e);
}
int InitShmPutImage (xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int	image_size;

    if(!InitGetImage(xp, p, reps))return False;
    if (!XShmQueryExtension(xp->d))
	return False;
    XClearWindow(xp->d, xp->w);
    shm_image = *image;
    image_size = image->bytes_per_line * image->height;
    /* allow XYPixmap choice: */
    if(p->font)image_size *= xp->vinfo.depth;
    shminfo.shmid = shmget(IPC_PRIVATE, image_size, IPC_CREAT|0777);
    if (shminfo.shmid < 0)
    {
	perror ("shmget");
	return False;
    }
    shminfo.shmaddr = (char *) shmat(shminfo.shmid, 0, 0);
    if (shminfo.shmaddr == ((char *) -1))
    {
	perror ("shmat");
	shmctl (shminfo.shmid, IPC_RMID, 0);
	return False;
    }
    shminfo.readOnly = True;
    XSync(xp->d,True);
    haderror = False;
    origerrorhandler = XSetErrorHandler(shmerrorhandler);
    XShmAttach (xp->d, &shminfo);
    XSync(xp->d,True);	/* wait for error or ok */
    XSetErrorHandler(origerrorhandler);
    if(haderror){
	if(shmdt (shminfo.shmaddr)==-1)
	    perror("shmdt:");
	if(shmctl (shminfo.shmid, IPC_RMID, 0)==-1)
	    perror("shmctl rmid:");
	return False;
    }
    shm_image.data = shminfo.shmaddr;
    bcopy (image->data, shm_image.data, image_size);
    shm_image.obdata = (char *) &shminfo;
    return reps;
}

void DoShmPutImage(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XShmPutImage(xp->d, xp->w, xp->fggc, &shm_image,
	    sa->x1, sa->y1, sa->x2, sa->y2, size, size, False);
	XShmPutImage(xp->d, xp->w, xp->fggc, &shm_image,
	    sa->x2, sa->y2, sa->x1, sa->y1, size, size, False);
	XShmPutImage(xp->d, xp->w, xp->fggc, &shm_image,
	    sb->x2, sb->y2, sb->x2, sb->y2, size, size, False);
	XShmPutImage(xp->d, xp->w, xp->fggc, &shm_image,
	    sb->x1, sb->y1, sb->x2, sb->y2, size, size, False);
    }
}

void EndShmPutImage(xp, p)
    XParms  xp;
    Parms   p;
{
    void    EndGetImage();

    EndGetImage (xp, p);
    XShmDetach (xp->d, &shminfo);
    XSync(xp->d, False);	/* need server to detach so can remove id */
    if(shmdt (shminfo.shmaddr)==-1)
	perror("shmdt:");
    if(shmctl (shminfo.shmid, IPC_RMID, 0)==-1)
	perror("shmctl rmid:");
}

#endif


void MidCopyPix(xp, p)
    XParms  xp;
    Parms   p;
{
    XClearWindow(xp->d, xp->w);
}

void EndCopyWin(xp, p)
    XParms  xp;
    Parms   p;
{
    EndScroll(xp, p);
    free(segsa);
    free(segsb);
}

void EndCopyPix(xp, p)
    XParms  xp;
    Parms   p;
{
    EndCopyWin(xp, p);
    XFreePixmap(xp->d, pix);
    /*
     * Ensure that the next test doesn't try and sync on the pixmap
     */
    xp->p = (Pixmap)0;
}

void EndGetImage(xp, p)
    XParms  xp;
    Parms   p;
{
    EndCopyWin(xp, p);
    if (image) XDestroyImage(image);
}

Bool InitCopyPlane(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int		i;
    XGCValues   gcv;
    GC		pixgc;

    for (i = 0; i != NUMPOINTS; i++) {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    InitCopyLocations(xp, p, reps);

    /* Create bitmap to write stuff into, and initialize it */
    pix = XCreatePixmap(xp->d, xp->w, WIDTH, HEIGHT, 1);
    gcv.graphics_exposures = False;
    gcv.foreground = 0;
    gcv.background = 1;
    pixgc = XCreateGC(xp->d, pix, 
		GCForeground | GCBackground | GCGraphicsExposures, &gcv);
    XFillRectangle(xp->d, pix, pixgc, 0, 0, WIDTH, HEIGHT);
    gcv.foreground = 1;
    gcv.background = 0;
    XChangeGC(xp->d, pixgc, GCForeground | GCBackground, &gcv);
    XDrawLines(xp->d, pix, pixgc, points, NUMPOINTS, CoordModeOrigin);
    XFreeGC(xp->d, pixgc);

    return reps;
}

void DoCopyPlane(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int		i, size;
    XSegment    *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XCopyPlane(xp->d, pix, xp->w, xp->fggc,
	    sa->x1, sa->y1, size, size, sa->x2, sa->y2, 1);
	XCopyPlane(xp->d, pix, xp->w, xp->fggc,
	    sa->x2, sa->y2, size, size, sa->x1, sa->y1, 1);
	XCopyPlane(xp->d, pix, xp->w, xp->fggc,
	    sb->x2, sb->y2, size, size, sb->x1, sb->y1, 1);
	XCopyPlane(xp->d, pix, xp->w, xp->fggc,
	    sb->x1, sb->y1, size, size, sb->x2, sb->y2, 1);
    }
}

