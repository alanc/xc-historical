#ifndef VMS
#include <X11/Xatom.h>
#else
#include <decw$include/Xatom.h>
#endif
#include "x11perf.h"

static GC gc;
static Window win[2];

void InitValGC(d, p)
    Display *d;
    Parms p;
{
    int width = 10, height=10;

    CreatePerfStuff(d, 2, width, height, win, &gc, NULL);
    XDrawPoint(d, win[1], gc, 5, 5);    
}

void DoValGC(d, p)
    Display *d;
    Parms p;
{
    int i;
    XGCValues gcv;

    for (i=0; i < p->reps; i++) {
        gcv.background = BlackPixel(d, 0);
        XChangeGC(d, gc, GCBackground , &gcv);
        XDrawPoint(d, win[0], gc, 5, 5);       

        gcv.background = WhitePixel(d, 0);
        XChangeGC(d, gc, GCBackground , &gcv);
        XDrawPoint(d, win[1], gc, 5, 5);       
    }
}

void EndValGC(d, numVals)
    Display *d;
    int numVals;
{
    XFreeGC(d, gc);
    XDestroyWindow(d, win[0]);
    XDestroyWindow(d, win[1]);
}

