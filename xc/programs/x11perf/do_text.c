#include "x11perf.h"

static Window w;
char **charBuf = NULL;
static GC blackgc, whitegc;
static XFontStruct *font, *bfont;
static int height, ypos;
static XTextItem *items;

#define XPOS 20
#define SEGMENTS 3
#define STARTBOLD 20
#define NUMBOLD 20

void InitText(d, p)
    Display *d;
    Parms p;
{
    int     i, j;
    XGCValues gcv;

    font = XLoadQueryFont (d, p->font);
    if (p->bfont != NULL)
	bfont = XLoadQueryFont (d, p->bfont);
    else
	bfont = NULL;
    ypos = XPOS;
    height = (font->max_bounds.ascent + font->max_bounds.descent) + 1;
    if (bfont != NULL) {
	int     h = (bfont->max_bounds.ascent + bfont->max_bounds.descent) + 1;
	if (h > height)
	    height = h;
    }
    CreatePerfStuff (d, 1, 800, HEIGHT, &w, &whitegc, &blackgc);
    gcv.font = font->fid;
    XChangeGC (d, blackgc, GCFont, &gcv);
    XChangeGC (d, whitegc, GCFont, &gcv);

    if (charBuf == NULL) {
	charBuf = (char **) malloc(p->reps*sizeof (char *));
	if (p->special)
	    items = (XTextItem *) malloc(p->reps*SEGMENTS*sizeof (XTextItem));
	else
	    items = NULL;
	for (i = 0; i < p->reps; i++) {
	    charBuf[i] = (char *) malloc (sizeof (char)*CHARS);
	    for (j = 0; j < CHARS; j++) {
		charBuf[i][j] = ' ' + (rand () % ('\177' - ' '));
		if (p->special) {
		    items[i*SEGMENTS+0].chars = &(charBuf[i][0]);
		    items[i*SEGMENTS+0].nchars = STARTBOLD;
		    items[i*SEGMENTS+0].delta = 0;
		    items[i*SEGMENTS+0].font = font->fid;
		    items[i*SEGMENTS+1].chars = &(charBuf[i][STARTBOLD]);
		    items[i*SEGMENTS+1].nchars = NUMBOLD;
		    items[i*SEGMENTS+1].delta = 3;
		    items[i*SEGMENTS+1].font = bfont->fid;
		    items[i*SEGMENTS+2].chars = &(charBuf[i][STARTBOLD + NUMBOLD]);
		    items[i*SEGMENTS+2].nchars = CHARS - (STARTBOLD + NUMBOLD);
		    items[i*SEGMENTS+2].delta = 3;
		    items[i*SEGMENTS+2].font = font->fid;
		}
	    }
	}
    }
}

void DoText(d, p)
    Display *d;
    Parms p;
{
    int     i;

    for (i = 0; i < p->reps; i++) {
	XDrawString(d, w, blackgc, XPOS, ypos, charBuf[i], CHARS);
	ypos = (ypos + height) % (HEIGHT - height);
    }
}

void DoPolyText(d, p)
    Display *d;
    Parms p;
{
    int     i;

    for (i = 0; i < p->reps; i++) {
	XDrawText(d, w, blackgc, XPOS, ypos, &items[i*SEGMENTS], SEGMENTS);
	ypos = (ypos + height) % (HEIGHT - height);
    }
}

void DoImageText(d, p)
    Display *d;
    Parms p;
{
    int     i;

    for (i = 0; i < p->reps; i++) {
	XDrawImageString(
	    d, w, (i & 1) ? blackgc : whitegc, XPOS, ypos, charBuf[i], CHARS);
	ypos = (ypos + height) % (HEIGHT - height);
    }
}

void ClearTextWin(d, numLines)
{
    XClearWindow(d, w);
}

void EndText(d, p)
    Display *d;
    Parms p;
{
    int i;

    XDestroyWindow(d, w);
    XFreeGC(d, blackgc);
    XFreeGC(d, whitegc);
    for (i = 0; i < p->reps; i++)
	free(charBuf[i]);
    free(charBuf);
    if (items != NULL)
	free(items);
    charBuf = NULL;
    XFreeFont(d, font);
    if (bfont != NULL)
	XFreeFont(d, bfont);
}

