/*-
 * sunCursor.c --
 *	Functions for maintaining the Sun software cursor...
 *
 */

#define NEED_EVENTS
#include    "sun.h"
#include    <windowstr.h>
#include    <regionstr.h>
#include    <dix.h>
#include    <dixstruct.h>
#include    <opaque.h>

#include    <servermd.h>
#include    "mipointer.h"
#include    "cursorstr.h"

#define CURSOR_PAD  8

#define GetCursorPrivate(s) (&(GetScreenPrivate(s)->hardwareCursor))
#define SetupCursor(s)	    sunCursorPtr pCurPriv = GetCursorPrivate(s)
#define CursorByteWidth(w)  (((w) + CURSOR_PAD - 1) / CURSOR_PAD)

static void
Repad (in, out, sc, w, h)
    char	    *in, *out;
    sunCursorPtr    sc;
    int		    w, h;
{
    int	    x, y;
    char    *a, *b;
    int	    inwidth, outwidth;
    char    mask;

    inwidth = (w + BITMAP_SCANLINE_PAD - 1) / 8;
    mask = w & 7;
#if BITMAP_BIT_ORDER == MSBFirst
    if (mask == 0)
	mask = 0xff;
    else
	mask = 0xff << (8-mask);
#else
    mask = 0xff >> mask;
#endif
    outwidth = CursorByteWidth(sc->width);
    for (y = 0; y < h; y++) {
	a = in;
	b = out;
	in += inwidth;
	out += outwidth;
	for (x = 0; x < inwidth; x++)
	    *b++ = *a++;
	if (inwidth)
	    b[-1] &= mask;
	for (; x < outwidth; x++)
	    *b++ = '\0';
    }
    for (; y < sc->height; y++)
    {
	for (x = 0; x < outwidth; x++)
	    *b++ = '\0';
    }
}

static Bool
sunRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    return TRUE;
}

static Bool
sunUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupCursor(pScreen);

    if (pCursor == pCurPriv->pCursor)
	pCurPriv->pCursor = 0;
    return TRUE;
}

static void
sunComputeColors (pCurPriv, pCursor, pfbcursor, r,g,b)
    sunCursorPtr    pCurPriv;
    CursorPtr	    pCursor;
    struct fbcursor *pfbcursor;
    unsigned char   r[2], g[2], b[2];
{
    r[0] = (pCurPriv->backRed = pCursor->backRed) >> 8;
    g[0] = (pCurPriv->backGreen = pCursor->backGreen) >> 8;
    b[0] = (pCurPriv->backBlue = pCursor->backBlue) >> 8;
    r[1] = (pCurPriv->foreRed = pCursor->foreRed) >> 8;
    g[1] = (pCurPriv->foreGreen = pCursor->foreGreen) >> 8;
    b[1] = (pCurPriv->foreBlue = pCursor->foreBlue) >> 8;
    pfbcursor->cmap.index = 0;
    pfbcursor->cmap.count = 2;
    pfbcursor->cmap.red = r;
    pfbcursor->cmap.green = g;
    pfbcursor->cmap.blue = b;
}

static void
sunLoadCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupCursor(pScreen);
    struct fbcursor fbcursor;
    int	w, h;
    unsigned char   r[2], g[2], b[2];

    w = pCursor->bits->width;
    h = pCursor->bits->height;
    fbcursor.set = FB_CUR_SETSHAPE|FB_CUR_SETCUR|FB_CUR_SETHOT|FB_CUR_SETCMAP;
    fbcursor.enable = 1;
    fbcursor.image = (char *) pCursor->bits->source;
    fbcursor.mask = (char *) pCursor->bits->mask;
    fbcursor.hot.x = pCursor->bits->xhot;
    fbcursor.hot.y = pCursor->bits->yhot;
    if (w > pCurPriv->width)
	w = pCurPriv->width;
    if (h > pCurPriv->height)
	h = pCurPriv->height;
    fbcursor.size.x = w;
    fbcursor.size.y = h;
    sunComputeColors (pCurPriv, pCursor, &fbcursor, r, g, b);
    (void) ioctl (sunFbs[pScreen->myNum].fd, FBIOSCURSOR, &fbcursor);
    pCurPriv->pCursor = pCursor;
}

#ifdef NOTDEF
/* This routine isn't needed - we just do the work in LoadCursor above */
static void
sunColorCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupCursor(pScreen);
    unsigned char   r[2], g[2], b[2];
    struct fbcursor fbcursor;

    fbcursor.set = FB_CUR_SETCMAP;
    sunComputeColors (pCurPriv, pCursor, &fbcursor, r, g, b);
    (void) ioctl (sunFbs[pScreen->myNum].fd, FBIOSCURSOR, &fbcursor);
}
#endif

static void
sunPositionCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    SetupCursor(pScreen);
    struct fbcursor fbcursor;
    struct fbcurpos pos;

    pos.x = x;
    pos.y = y;
    ioctl (sunFbs[pScreen->myNum].fd, FBIOSCURPOS, &pos);
    pCurPriv->x = x;
    pCurPriv->y = y;
}

static void
sunDisplayCursor (pScreen, pCursor, x, y)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    int		x, y;
{
    SetupCursor(pScreen);

    if (pCurPriv->pCursor != pCursor)
	sunLoadCursor (pScreen, pCursor);
    /* a check for cursor color change could be made here,
     * instead we rely on using miRecolorCursor which unrealizes
     * and rerealizes the cursor, setting pCurPriv->pCursor = 0 in
     * unrealize above solves the recoloring problem!
     */
    if (pCurPriv->x != x || pCurPriv->y != y)
	sunPositionCursor (pScreen, x, y);
}

static void
sunUndisplayCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupCursor (pScreen);
    struct fbcursor fbcursor;

    fbcursor.set = FB_CUR_SETCUR;
    fbcursor.enable = 0;
    (void) ioctl (sunFbs[pScreen->myNum].fd, FBIOSCURSOR, &fbcursor);
    return;
}

miPointerSpriteFuncRec sunPointerSpriteFuncs = {
    sunRealizeCursor,
    sunUnrealizeCursor,
    sunDisplayCursor,
    sunUndisplayCursor,
};

static ScreenPtr    pQueryBestSizeScreen;

static void
sunQueryBestSize (class, pwidth, pheight)
    int	class;
    short   *pwidth, *pheight;
{
    SetupCursor (pQueryBestSizeScreen);	/* sorry, no screen pointer here */

    switch (class)
    {
    case CursorShape:
	if (*pwidth > pCurPriv->width)
	    *pwidth = pCurPriv->width;
	if (*pheight > pCurPriv->height)
	    *pheight = pCurPriv->height;
	break;
    default:
	mfbQueryBestSize (class, pwidth, pheight);
	break;
    }
}

extern miPointerCursorFuncRec	sunPointerCursorFuncs;

Bool
sunCursorInitialize (pScreen)
    ScreenPtr	pScreen;
{
    SetupCursor (pScreen);
    int	    fd;
    struct fbcursor fbcursor;
    struct fbcurpos maxsize;
    char    *source, *mask;
    int	    h, w;
    int	    size;

    if (ioctl (sunFbs[pScreen->myNum].fd, FBIOGCURMAX, &maxsize) == -1)
	return FALSE;
    pCurPriv->width = maxsize.x;
    pCurPriv->height= maxsize.y;
    pCurPriv->pCursor = 0;
    pCurPriv->x = -1;
    pCurPriv->y = -1;
    pScreen->QueryBestSize = sunQueryBestSize;
    pQueryBestSizeScreen = pScreen;
    miPointerInitialize (pScreen,
			 &sunPointerSpriteFuncs,
			 &sunPointerCursorFuncs);
    
}

/*
 * The following struct is from win_cursor.h.  This file can't be included 
 * directly, because it drags in all of the SunView attribute stuff along 
 * with it.
 */

#ifdef SUN_WINDOWS

struct cursor {
    short       cur_xhot, cur_yhot;	/* offset of mouse position from shape */
    int         cur_function;		/* relationship of shape to screen */
    struct pixrect *cur_shape;		/* memory image to use */
    int         flags;			/* various options */
    short       horiz_hair_thickness;	/* horizontal crosshair height */
    int         horiz_hair_op;		/* drawing op       */
    int         horiz_hair_color;	/* color            */
    short       horiz_hair_length;	/* width           */
    short       horiz_hair_gap;		/* gap             */
    short       vert_hair_thickness;	/* vertical crosshair width  */
    int         vert_hair_op;		/* drawing op       */
    int         vert_hair_color;	/* color            */
    short       vert_hair_length;	/* height           */
    short       vert_hair_gap;		/* gap              */
};
#endif SUN_WINDOWS

void
sunInitCursor ()
{
    if ( sunUseSunWindows() ) {
#ifdef SUN_WINDOWS
	static struct cursor cs;
	static struct pixrect pr;
	/* 
	 * Give the pixwin an empty cursor so that the kernel's cursor drawing 
	 * doesn't conflict with our cursor drawing.
	 */
	cs.cur_xhot = cs.cur_yhot = cs.cur_function = 0;
	cs.flags = 0;
	cs.cur_shape = &pr;
	pr.pr_size.x = pr.pr_size.y = 0;
	win_setcursor( windowFd, &cs );
#endif SUN_WINDOWS
    }
}


#ifdef SUN_WINDOWS
/*
 * We need to find out when dix warps the mouse so we can
 * keep SunWindows in sync.
 */

extern int sunIgnoreEvent;

Bool
sunSetCursorPosition(pScreen, x, y, generateEvent)
	ScreenPtr pScreen;
	int x, y;
	Bool generateEvent;
{
    SetupScreen(pScreen);

    pScreen->SetCursorPosition = pPrivate->SetCursorPosition;
    (*pScreen->SetCursorPosition) (pScreen, x, y, generateEvent);
    if (sunUseSunWindows())
	if (!sunIgnoreEvent)
	    win_setmouseposition(windowFd, x, y);
    pScreen->SetCursorPosition = sunSetCursorPosition;
    return TRUE;
}
#endif







