#include "x11perf.h"

static Window w;
static char **charBuf;
static GC fggc, bggc;
static XFontStruct *font, *bfont;
static int height, ypos;
static XTextItem *items;
static int charsPerLine, totalLines;

#define XPOS 20
#define SEGMENTS 3


Bool InitText(d, p)
    Display *d;
    Parms p;
{
    int     i, j;
    char    ch;
    XGCValues gcv;

    font = XLoadQueryFont (d, p->font);
    if (font == NULL) {
	printf("Could not load font '%s', benchmark omitted\n", p->font);
	return False;
    }

    if (p->bfont != NULL) {
	bfont = XLoadQueryFont (d, p->bfont);
	if (bfont == NULL) {
	    printf("Could not load font '%s', benchmark omitted\n", p->bfont);
	    return False;
	}
    } else {
	bfont = NULL;
    }
    ypos = XPOS;
    height = (font->max_bounds.ascent + font->max_bounds.descent) + 1;
    if (bfont != NULL) {
	int     h = (bfont->max_bounds.ascent + bfont->max_bounds.descent) + 1;
	if (h > height)
	    height = h;
    }
    CreatePerfStuff (d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);
    gcv.font = font->fid;
    XChangeGC (d, fggc, GCFont, &gcv);
    XChangeGC (d, bggc, GCFont, &gcv);

    charsPerLine = p->objects;
    charsPerLine = (charsPerLine + 3) & ~3;
    p->objects = charsPerLine;

    totalLines = '\177' - ' ' + 1;
    if (totalLines > p->reps) totalLines = p->reps;

    charBuf = (char **) malloc(totalLines*sizeof (char *));
    if (p->special)
	items = (XTextItem *) malloc(totalLines*SEGMENTS*sizeof (XTextItem));

    for (i = 0; i < totalLines; i++) {
	charBuf[i] = (char *) malloc (sizeof (char)*charsPerLine);
	ch = i + ' ';
	for (j = 0; j < charsPerLine; j++) {
	    charBuf[i][j] = ch;
	    if (ch == '\177') ch = ' '; else ch++;
	}
	if (p->special) {
	    items[i*SEGMENTS+0].chars = &(charBuf[i][0]);
	    items[i*SEGMENTS+0].nchars = charsPerLine/4;
	    items[i*SEGMENTS+0].delta = 0;
	    items[i*SEGMENTS+0].font = font->fid;
	    items[i*SEGMENTS+1].chars = &(charBuf[i][charsPerLine/4]);
	    items[i*SEGMENTS+1].nchars = charsPerLine/2;
	    items[i*SEGMENTS+1].delta = 3;
	    items[i*SEGMENTS+1].font = bfont->fid;
	    items[i*SEGMENTS+2].chars = &(charBuf[i][3*charsPerLine/4]);
	    items[i*SEGMENTS+2].nchars = charsPerLine/4;
	    items[i*SEGMENTS+2].delta = 3;
	    items[i*SEGMENTS+2].font = font->fid;
	}
    }
    return True;
}

void DoText(d, p)
    Display *d;
    Parms p;
{
    int     i, line, startLine;

    startLine = 0;
    line = 0;
    for (i = 0; i < p->reps; i++) {
	XDrawString(d, w, fggc, XPOS, ypos, charBuf[line], charsPerLine);
	ypos += height;
	if (ypos > HEIGHT - height) {
	    /* Wraparound to top of window */
	    ypos = XPOS;
	    line = startLine;
	    startLine = (startLine + 1) % totalLines;
	}
	line = (line + 1) % totalLines;
    }
}

void DoPolyText(d, p)
    Display *d;
    Parms p;
{
    int     i, line, startLine;

    startLine = 0;
    line = 0;
    for (i = 0; i < p->reps; i++) {
	XDrawText(d, w, fggc, XPOS, ypos, &items[line*SEGMENTS], SEGMENTS);
	ypos += height;
	if (ypos > HEIGHT - height) {
	    /* Wraparound to top of window */
	    ypos = XPOS;
	    line = startLine;
	    startLine = (startLine + 1) % totalLines;
	}
	line = (line + 1) % totalLines;
    }
}

void DoImageText(d, p)
    Display *d;
    Parms p;
{
    int     i, line, startLine;

    startLine = 0;
    line = 0;
    for (i = 0; i < p->reps; i++) {
	XDrawImageString(
	    d, w, fggc, XPOS, ypos, charBuf[line], charsPerLine);
	ypos += height;
	if (ypos > HEIGHT - height) {
	    /* Wraparound to top of window */
	    ypos = XPOS;
	    startLine = (startLine + 17) % totalLines;
	    line = startLine;
	}
	line = (line + 1) % totalLines;
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
    XFreeGC(d, fggc);
    XFreeGC(d, bggc);
    for (i = 0; i < totalLines; i++)
	free(charBuf[i]);
    free(charBuf);
    if (p->special)
	free(items);
    XFreeFont(d, font);
    if (bfont != NULL)
	XFreeFont(d, bfont);
}

