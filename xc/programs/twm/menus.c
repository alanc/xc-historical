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
 * $XConsortium: menus.c,v 1.70 89/06/13 09:50:02 jim Exp $
 *
 * twm menu code
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[] =
"$XConsortium: menus.c,v 1.70 89/06/13 09:50:02 jim Exp $";
#endif

#include <stdio.h>
#include <signal.h>
#include <X11/Xos.h>
#include "twm.h"
#include "gc.h"
#include "menus.h"
#include "events.h"
#include "util.h"
#include "gram.h"
#include "screen.h"
#include "pull.bm"

#define SYNC XSync(dpy, 0);

extern XEvent Event;

char *getenv();

#ifdef macII
#define vfork fork
#endif

int RootFunction = NULL;
MenuRoot *ActiveMenu = NULL;		/* the active menu */
MenuItem *ActiveItem = NULL;		/* the active menu item */
int MoveFunction;			/* either F_MOVE or F_FORCEMOVE */
int WindowMoved = FALSE;

int ConstMove = FALSE;		/* constrained move variables */
int ConstMoveDir;
int ConstMoveX;
int ConstMoveY;
int ConstMoveXL;
int ConstMoveXR;
int ConstMoveYT;
int ConstMoveYB;
 
int StartingX, StartingY, RestorePointer;
static Cursor LastCursor;

ScreenInfo *FindScreenInfo();
extern char *Action;
extern int Context;
extern TwmWindow *ButtonWindow, *Tmp_win;
extern XEvent Event, ButtonEvent;
extern char *InitFile;

/***********************************************************************
 *
 *  Procedure:
 *	InitMenus - initialize menu roots
 *
 ***********************************************************************
 */

void
InitMenus()
{
    int i, j, k;
    FuncKey *key, *tmp;

    for (i = 0; i < MAX_BUTTONS+1; i++)
	for (j = 0; j < NUM_CONTEXTS; j++)
	    for (k = 0; k < MOD_SIZE; k++)
	    {
		Scr->Mouse[i][j][k].func = NULL;
		Scr->Mouse[i][j][k].item = NULL;
	    }

    Scr->DefaultFunction.func = NULL;
    Scr->WindowFunction.func = NULL;

    Scr->Mouse[1][C_TITLE][0].func = F_RAISE;
    Scr->Mouse[2][C_TITLE][0].func = F_MOVE;
    Scr->Mouse[3][C_TITLE][0].func = F_LOWER;

    Scr->Mouse[1][C_ICON][0].func = F_ICONIFY;
    Scr->Mouse[2][C_ICON][0].func = F_MOVE;
    Scr->Mouse[3][C_ICON][0].func = F_LOWER;

    Scr->Mouse[1][C_ICONMGR][0].func = F_ICONIFY;
    Scr->Mouse[2][C_ICONMGR][0].func = F_ICONIFY;
    Scr->Mouse[3][C_ICONMGR][0].func = F_ICONIFY;

    if (FirstScreen)
    {
	for (key = Scr->FuncKeyRoot.next; key != NULL;)
	{
	    free(key->name);
	    tmp = key;
	    key = key->next;
	    free(tmp);
	}
	Scr->FuncKeyRoot.next = NULL;
    }

}

/***********************************************************************
 *
 *  Procedure:
 *	AddFuncKey - add a function key to the list
 *
 *  Inputs:
 *	name	- the name of the key
 *	cont	- the context to look for the key press in
 *	mods	- modifier keys that need to be pressed
 *	func	- the function to perform
 *	win_name- the window name (if any)
 *	action	- the action string associated with the function (if any)
 *
 ***********************************************************************
 */

Bool AddFuncKey (name, cont, mods, func, win_name, action)
    char *name;
    int cont, mods, func;
    char *win_name;
    char *action;
{
    FuncKey *tmp;
    KeySym keysym;
    KeyCode keycode;

    /*
     * Don't let a 0 keycode go through, since that means AnyKey to the
     * XGrabKey call in GrabKeys().
     */
    if ((keysym = XStringToKeysym(name)) == NoSymbol ||
	(keycode = XKeysymToKeycode(dpy, keysym)) == 0)
    {
	return False;
    }

    /* see if there already is a key defined for this context */
    for (tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->keysym == keysym &&
	    tmp->cont == cont &&
	    tmp->mods == mods)
	    break;
    }

    if (tmp == NULL)
    {
	tmp = (FuncKey *) malloc(sizeof(FuncKey));
	tmp->next = Scr->FuncKeyRoot.next;
	Scr->FuncKeyRoot.next = tmp;
    }

    tmp->name = name;
    tmp->keysym = keysym;
    tmp->keycode = keycode;
    tmp->cont = cont;
    tmp->mods = mods;
    tmp->func = func;
    tmp->win_name = win_name;
    tmp->action = action;

    return True;
}

PaintEntry(mr, mi, exposure)
MenuRoot *mr;
MenuItem *mi;
int exposure;
{
    XGCValues gcv;
    int new_colors;
    int y_offset;
    int text_y;
    GC gc;

#ifdef DEBUG_MENUS
    fprintf(stderr, "Paint entry\n");
#endif
    y_offset = mi->item_num * Scr->EntryHeight;
    text_y = y_offset + Scr->MenuFont.y;

    if (mi->func != F_TITLE)
    {
	int x, y;

	if (mi->state)
	{
	    XSetForeground(dpy, Scr->NormalGC, mi->hi_back);

	    XFillRectangle(dpy, mr->w, Scr->NormalGC, 0, y_offset,
		mr->width, Scr->EntryHeight);

	    FBF(mi->hi_fore, mi->hi_back, Scr->MenuFont.font->fid);

	    XDrawString(dpy, mr->w, Scr->NormalGC, mi->x,
		text_y, mi->item, mi->strlen);

	    gc = Scr->NormalGC;
	}
	else
	{
	    if (mi->user_colors || !exposure)
	    {
		XSetForeground(dpy, Scr->NormalGC, mi->back);

		XFillRectangle(dpy, mr->w, Scr->NormalGC, 0, y_offset,
		    mr->width, Scr->EntryHeight);

		FBF(mi->fore, mi->back, Scr->MenuFont.font->fid);
		gc = Scr->NormalGC;
	    }
	    else
		gc = Scr->MenuGC;

	    XDrawString(dpy, mr->w, gc, mi->x,
		text_y, mi->item, mi->strlen);
	}

	if (mi->func == F_MENU)
	{

	    /* create the pull right pixmap if needed */
	    if (Scr->pullPm == NULL)
	    {
		Scr->pullPm = XCreatePixmapFromBitmapData(dpy, Scr->Root,
		    pull_bits, pull_width, pull_height, 1, 0, 1);
	    }
	    x = mr->width - pull_width - 5;
	    y = y_offset + ((Scr->EntryHeight - pull_height)/2);
	    XCopyPlane(dpy, Scr->pullPm, mr->w, gc, 0, 0,
		pull_width, pull_height, x, y, 1);
	}
    }
    else
    {
	int x, y;
	
	XSetForeground(dpy, Scr->NormalGC, mi->back);

	/* fill the rectangle with the title background color */
	XFillRectangle(dpy, mr->w, Scr->NormalGC, 0, y_offset,
	    mr->width, Scr->EntryHeight);

	{
	    XSetForeground(dpy, Scr->NormalGC, mi->fore);
	    /* now draw the dividing lines */
	    XDrawLine(dpy, mr->w, Scr->NormalGC, 0, y_offset,
		mr->width, y_offset);
	    y = ((mi->item_num+1) * Scr->EntryHeight)-1;
	    XDrawLine(dpy, mr->w, Scr->NormalGC, 0, y, mr->width, y);
	}

	FBF(mi->fore, mi->back, Scr->MenuFont.font->fid);
	/* finally render the title */
	XDrawString(dpy, mr->w, Scr->NormalGC, mi->x,
	    text_y, mi->item, mi->strlen);
    }
}
    

PaintMenu(mr, e)
MenuRoot *mr;
XEvent *e;
{
    MenuItem *mi;
    int y, x, y_offset;
    GC gc;

#ifdef DEBUG_MENUS
    fprintf(stderr, "PaintMenu  %d, %d  (%d x %d)\n",
	e->xexpose.x, e->xexpose.y, e->xexpose.width, e->xexpose.height);
#endif
    x = mr->width - 20;

    for (mi = mr->first; mi != NULL; mi = mi->next)
    {
	y_offset = mi->item_num * Scr->EntryHeight;

	/* be smart about handling the expose, redraw only the entries
	 * that we need to
	 */
	if (e->xexpose.y < (y_offset + Scr->EntryHeight) &&
	    (e->xexpose.y + e->xexpose.height) > y_offset)
	{
	    PaintEntry(mr, mi, True);
	}
    }
    XSync(dpy, 0);
}

UpdateMenu()
{
    MenuRoot *mr, *tmp;
    MenuItem *mi;
    int i, x, y, x_root, y_root, entry;
    int done;
    int first = TRUE;
    int save_x, save_y;

    while (TRUE)
    {
	while (XCheckMaskEvent(dpy, ButtonPressMask | ButtonReleaseMask |
	    EnterWindowMask | ExposureMask, &Event))
	{
	    if (XFindContext(dpy, Event.xany.window,
		TwmContext,&Tmp_win) == XCNOENT)
		Tmp_win = NULL;

	    if (XFindContext(dpy, Event.xany.window,
		ScreenContext, &Scr) == XCNOENT)
		Scr = FindScreenInfo(Event.xany.window);

	    if (Scr == NULL)
		continue;

	    if (Event.type == ButtonRelease || Cancel)
		if (!Scr->NoGrabServer)
		    XUngrabServer(dpy);

	    if (Event.type >= 0 && Event.type < LASTEvent)
	        (*EventHandler[Event.type])();

	    if (Event.type == ButtonRelease || Cancel)
		return;
	}

	/* if we haven't recieved the enter notify yet, wait */
	if (ActiveMenu && !ActiveMenu->entered)
	    continue;

	done = FALSE;
	XQueryPointer( dpy, ActiveMenu->w, &JunkRoot, &JunkChild,
	    &x_root, &y_root, &x, &y, &JunkMask);
	if (first)
	{
	    save_x = x_root;
	    save_y = y_root;
	    first = FALSE;
	}

	/* if the mouse has moved we don't have to put it back */
	if (save_x != x_root || save_y != y_root)
	    RestorePointer = FALSE;

	XFindContext(dpy, ActiveMenu->w, ScreenContext, &Scr);

	if (x < 0 || y < 0 || x >= ActiveMenu->width || y >= ActiveMenu->height)
	{
	    if (ActiveItem && ActiveItem->func != F_TITLE)
	    {
		ActiveItem->state = 0;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	    ActiveItem = NULL;
	    continue;
	}

	/* look for the entry that the mouse is in */
	entry = y / Scr->EntryHeight;
	for (i = 0, mi = ActiveMenu->first; mi != NULL; i++, mi=mi->next)
	{
	    if (i == entry)
		break;
	}

	/* if there is an active item, we might have to turn it off */
	if (ActiveItem)
	{
	    /* is the active item the one we are on ? */
	    if (ActiveItem->item_num == entry && ActiveItem->state)
		done = TRUE;

	    /* if we weren't on the active entry, let's turn the old
	     * active one off 
	     */
	    if (!done && ActiveItem->func != F_TITLE)
	    {
		ActiveItem->state = 0;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	}

	/* if we weren't on the active item, change the active item and turn
	 * it on 
	 */
	if (!done)
	{
	    ActiveItem = mi;
	    if (ActiveItem->func != F_TITLE && !ActiveItem->state)
	    {
		ActiveItem->state = 1;
		PaintEntry(ActiveMenu, ActiveItem, False);
	    }
	}

	/* now check to see if we were over the arrow of a pull right entry */
	if (ActiveItem->func == F_MENU && ((ActiveMenu->width - x) < 20))
	{
	    MenuRoot *save;

	    save = ActiveMenu;
	    PopUpMenu(ActiveItem->sub, x_root, y_root);

	    /* if the menu did get popped up, unhighlight the active item */
	    if (save != ActiveMenu && ActiveItem->state)
	    {
		ActiveItem->state = 0;
		PaintEntry(save, ActiveItem, False);
		ActiveItem = NULL;
	    }
	}
	XFlush(dpy);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	NewMenuRoot - create a new menu root
 *
 *  Returned Value:
 *	(MenuRoot *)
 *
 *  Inputs:
 *	name	- the name of the menu root
 *
 ***********************************************************************
 */

MenuRoot *
NewMenuRoot(name)
    char *name;
{
    MenuRoot *tmp;
    unsigned long valuemask;
    XSetWindowAttributes attributes;


    tmp = (MenuRoot *) malloc(sizeof(MenuRoot));
    tmp->hi_fore = -1;
    tmp->hi_back = -1;
    tmp->name = name;
    tmp->prev = NULL;
    tmp->first = NULL;
    tmp->last = NULL;
    tmp->items = 0;
    tmp->width = 0;
    tmp->mapped = NEVER_MAPPED;
    tmp->pull = FALSE;
    tmp->w = NULL;
    tmp->shadow = NULL;
    tmp->real_menu = FALSE;

    if (Scr->MenuList == NULL)
    {
	Scr->MenuList = tmp;
	Scr->MenuList->next = NULL;
    }

    if (Scr->LastMenu == NULL)
    {
	Scr->LastMenu = tmp;
	Scr->LastMenu->next = NULL;
    }
    else
    {
	Scr->LastMenu->next = tmp;
	Scr->LastMenu = tmp;
	Scr->LastMenu->next = NULL;
    }

    if (strcmp(name, TWM_WINDOWS) == 0)
	Scr->Windows = tmp;

    return (tmp);
}

/***********************************************************************
 *
 *  Procedure:
 *	AddToMenu - add an item to a root menu
 *
 *  Returned Value:
 *	(MenuItem *)
 *
 *  Inputs:
 *	menu	- pointer to the root menu to add the item
 *	item	- the text to appear in the menu
 *	action	- the string to possibly execute
 *	sub	- the menu root if it is a pull-right entry
 *	func	- the numeric function
 *	fore	- foreground color string
 *	back	- background color string
 *
 ***********************************************************************
 */

MenuItem *
AddToMenu(menu, item, action, sub, func, fore, back)
    MenuRoot *menu;
    char *item, *action;
    MenuRoot *sub;
    int func;
    char *fore, *back;
{
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    MenuItem *tmp;
    int width;

#ifdef DEBUG_MENUS
    fprintf(stderr, "adding menu item=\"%s\", action=%s, sub=%d, f=%d\n",
	item, action, sub, func);
#endif

    tmp = (MenuItem *) malloc(sizeof(MenuItem));
    tmp->root = menu;

    if (menu->first == NULL)
    {
	menu->first = tmp;
	tmp->prev = NULL;
    }
    else
    {
	menu->last->next = tmp;
	tmp->prev = menu->last;
    }
    menu->last = tmp;

    tmp->item = item;
    tmp->strlen = strlen(item);
    tmp->action = action;
    tmp->next = NULL;
    tmp->sub = NULL;
    tmp->state = 0;
    tmp->func = func;

    width = XTextWidth(Scr->MenuFont.font, item, tmp->strlen);
    if (width <= 0)
	width = 1;
    if (width > menu->width)
	menu->width = width;

    tmp->user_colors = FALSE;
    if (Scr->Monochrome == COLOR && fore != NULL)
    {
	int save;

	save = Scr->FirstTime;
	Scr->FirstTime = TRUE;
	GetColor(COLOR, &tmp->fore, fore);
	GetColor(COLOR, &tmp->back, back);
	Scr->FirstTime = save;
	tmp->user_colors = TRUE;
    }
    if (sub != NULL)
    {
	tmp->sub = sub;
	menu->pull = TRUE;
    }
    tmp->item_num = menu->items++;

    return (tmp);
}

MakeMenus()
{
    MenuRoot *mr;
    unsigned long gcm;
    XGCValues gcv;

    for (mr = Scr->MenuList; mr != NULL; mr = mr->next)
    {
	if (mr->real_menu == FALSE)
	    continue;

	MakeMenu(mr);
    }
}

MakeMenu(mr)
MenuRoot *mr;
{
    MenuItem *start, *end, *cur, *tmp;
    XColor f1, f2, f3;
    XColor b1, b2, b3;
    XColor save_fore, save_back;
    int num, i;
    int fred, fgreen, fblue;
    int bred, bgreen, bblue;
    int width;
    unsigned long valuemask;
    XSetWindowAttributes attributes;

    Scr->EntryHeight = Scr->MenuFont.height + 4;

    /* lets first size the window accordingly */
    if (mr->mapped == NEVER_MAPPED)
    {
	if (mr->pull == TRUE)
	{
	    mr->width += 16 + 10;
	}

	width = mr->width + 10;

	for (cur = mr->first; cur != NULL; cur = cur->next)
	{
	    if (cur->func != F_TITLE)
		cur->x = 5;
	    else
	    {
		cur->x = width - XTextWidth(Scr->MenuFont.font, cur->item,
		    cur->strlen);
		cur->x /= 2;
	    }
	}
	mr->height = mr->items * Scr->EntryHeight;
	mr->width += 10;

	if (Scr->Shadow)
	{
	    mr->shadow = XCreateSimpleWindow(dpy, Scr->Root,
		0, 0, mr->width, mr->height, 1,
		Scr->MenuShadowColor, Scr->MenuShadowColor);
	}

	mr->w = XCreateSimpleWindow(dpy, Scr->Root,
	    0, 0, mr->width, mr->height, 1,
	    Scr->MenuC.fore, Scr->MenuC.back);
	XSelectInput(dpy, mr->w, ExposureMask | EnterWindowMask);

	XSaveContext(dpy, mr->w, MenuContext, mr);
	XSaveContext(dpy, mr->w, ScreenContext, Scr);

	if (Scr->SaveUnder)
	{
	    valuemask = CWSaveUnder;
	    attributes.save_under = True;
	    if (Scr->Shadow)
		XChangeWindowAttributes(dpy, mr->shadow,valuemask,&attributes);
	    XChangeWindowAttributes(dpy, mr->w, valuemask, &attributes);
	}

	if (Scr->BackingStore)
	{
	    valuemask = CWBackingStore;
	    attributes.backing_store = Always;
	    XChangeWindowAttributes(dpy, mr->w, valuemask, &attributes);
	}

	mr->mapped = UNMAPPED;
    }

    /* get the default colors into the menus */
    for (tmp = mr->first; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->user_colors)
	    continue;

	if (tmp->func != F_TITLE)
	{
	    tmp->fore = Scr->MenuC.fore;
	    tmp->back = Scr->MenuC.back;
	}
	else
	{
	    tmp->fore = Scr->MenuTitleC.fore;
	    tmp->back = Scr->MenuTitleC.back;
	}

	if (mr->hi_fore != -1)
	{
	    tmp->hi_fore = mr->hi_fore;
	    tmp->hi_back = mr->hi_back;
	}
	else
	{
	    tmp->hi_fore = tmp->back;
	    tmp->hi_back = tmp->fore;
	}
    }

    if (Scr->Monochrome == MONOCHROME || !Scr->InterpolateMenuColors)
	return;

    start = mr->first;
    end = NULL;
    while (TRUE)
    {
	for (; start != NULL; start = start->next)
	{
	    if (start->user_colors)
		break;
	}
	if (start == NULL)
	    break;

	for (end = start->next; end != NULL; end = end->next)
	{
	    if (end->user_colors)
		break;
	}
	if (end == NULL)
	    break;

	/* we have a start and end to interpolate between */
	num = end->item_num - start->item_num;

	f1.pixel = start->fore;
	XQueryColor(dpy, Scr->CMap, &f1);
	f2.pixel = end->fore;
	XQueryColor(dpy, Scr->CMap, &f2);

	b1.pixel = start->back;
	XQueryColor(dpy, Scr->CMap, &b1);
	b2.pixel = end->back;
	XQueryColor(dpy, Scr->CMap, &b2);

	fred = ((int)f2.red - (int)f1.red) / num;
	fgreen = ((int)f2.green - (int)f1.green) / num;
	fblue = ((int)f2.blue - (int)f1.blue) / num;

	bred = ((int)b2.red - (int)b1.red) / num;
	bgreen = ((int)b2.green - (int)b1.green) / num;
	bblue = ((int)b2.blue - (int)b1.blue) / num;

	f3 = f1;
	f3.flags = DoRed | DoGreen | DoBlue;

	b3 = b1;
	b3.flags = DoRed | DoGreen | DoBlue;

	num -= 1;
	for (i = 0, cur = start->next; i < num; i++, cur = cur->next)
	{
	    f3.red += fred;
	    f3.green += fgreen;
	    f3.blue += fblue;
	    save_fore = f3;

	    b3.red += bred;
	    b3.green += bgreen;
	    b3.blue += bblue;
	    save_back = b3;

	    XAllocColor(dpy, Scr->CMap, &f3);
	    XAllocColor(dpy, Scr->CMap, &b3);
	    cur->fore = f3.pixel;
	    cur->back = b3.pixel;
	    cur->user_colors = True;

	    f3 = save_fore;
	    b3 = save_back;
	}
	start = end;
    }

    /* now redo the highlight colors
     */
    for (cur = mr->first; cur != NULL; cur = cur->next)
    {
	if (mr->hi_fore != -1)
	{
	    cur->hi_fore = mr->hi_fore;
	    cur->hi_back = mr->hi_back;
	}
	else
	{
	    cur->hi_fore = cur->back;
	    cur->hi_back = cur->fore;
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	PopUpMenu - pop up a pull down menu
 *
 *  Inputs:
 *	menu	- the root pointer of the menu to pop up
 *	x	- the x location of the mouse
 *	y	- the y location of the mouse
 *
 ***********************************************************************
 */

void
PopUpMenu(menu, x, y)
    MenuRoot *menu;
    int x, y;
{
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    MenuItem *tmp, *tmp1;
    TwmWindow *tmp_win;

    if (menu == NULL)
	return;

    if (menu == Scr->Windows)
    {
	/* this is the twm windows menu,  let's go ahead and build it */

	if (menu->w)
	{
	    XDeleteContext(dpy, menu->w, MenuContext);
	    XDeleteContext(dpy, menu->w, ScreenContext);
	    if (Scr->Shadow)
		XDestroyWindow(dpy, menu->shadow);
	    XDestroyWindow(dpy, menu->w);
	}

	for (tmp = menu->first; tmp != NULL;)
	{
	    tmp1 = tmp;
	    tmp = tmp->next;
	    free(tmp1);
	}

	menu->first = NULL;
	menu->last = NULL;
	menu->items = 0;
	menu->width = 0;
	menu->mapped = NEVER_MAPPED;

	AddToMenu(menu, "TWM Windows", NULL, NULL, F_TITLE, NULL, NULL);
	for (tmp_win = Scr->TwmRoot.next;
	     tmp_win != NULL;
	     tmp_win = tmp_win->next)
	{
	    AddToMenu(menu, tmp_win->name, tmp_win, NULL, F_POPUP, NULL, NULL);
	}
	MakeMenu(menu);
    }

    if (menu->items == 0)
	return;

    /* Prevent recursively bringing up menus. */
    if (menu->mapped == MAPPED)
	return;

    if (ActiveMenu == NULL)
    {
	StartingX = x;
	StartingY = y;
	RestorePointer = TRUE;
    }
    /*
     * Dynamically set the parent;  this allows pull-ups to also be main
     * menus, or to be brought up from more than one place.
     */
    if (ActiveMenu != NULL)
	menu->prev = ActiveMenu;
    else
	menu->prev = NULL;

    XGrabPointer(dpy, Scr->Root, True,
	ButtonPressMask | ButtonReleaseMask,
	GrabModeAsync, GrabModeAsync,
	Scr->Root, Scr->MenuCursor, CurrentTime);

    ActiveMenu = menu;
    menu->mapped = MAPPED;
    menu->entered = FALSE;

    if ((x + 25) > Scr->MyDisplayWidth)
	x = (Scr->MyDisplayWidth - 30);

    if ((y + menu->height + 25) > Scr->MyDisplayHeight)
	y = (Scr->MyDisplayHeight - menu->height) - 15;

    x -= menu->width - 10;
    if (x < 0)
	x = 0;
    y -= ((Scr->MenuFont.height + 4) / 2);

    x += 15;
    y += 15;

    XMoveWindow(dpy, menu->w, x, y);
    if (Scr->Shadow)
	XMoveWindow(dpy, menu->shadow, x+5, y+5);
    XWarpPointer(dpy, None, menu->w, 0, 0, 0, 0, 
	menu->width - 30, (Scr->MenuFont.height + 4) / 2);
	/* (menu->width - 10)/2, (Scr->MenuFont.height + 4) / 2);*/
    if (Scr->Shadow)
	XRaiseWindow(dpy, menu->shadow);
    XMapRaised(dpy, menu->w);
    if (Scr->Shadow)
	XMapWindow(dpy, menu->shadow);
    XSync(dpy, 0);
}

/***********************************************************************
 *
 *  Procedure:
 *	PopDownMenu - unhighlight the current menu selection and
 *		take down the menus
 *
 ***********************************************************************
 */

PopDownMenu()
{
    MenuRoot *tmp;

    if (ActiveMenu == NULL)
	return;

    if (ActiveItem)
    {
	ActiveItem->state = 0;
	PaintEntry(ActiveMenu, ActiveItem, False);
    }

    for (tmp = ActiveMenu; tmp != NULL; tmp = tmp->prev)
    {
	if (Scr->Shadow)
	    XUnmapWindow(dpy, tmp->shadow);
	XUnmapWindow(dpy, tmp->w);
	tmp->mapped = UNMAPPED;
    }
    XFlush(dpy);
    ActiveMenu = NULL;
    ActiveItem = NULL;
}

/***********************************************************************
 *
 *  Procedure:
 *	FindMenuRoot - look for a menu root
 *
 *  Returned Value:
 *	(MenuRoot *)  - a pointer to the menu root structure 
 *
 *  Inputs:
 *	name	- the name of the menu root 
 *
 ***********************************************************************
 */

MenuRoot *
FindMenuRoot(name)
    char *name;
{
    MenuRoot *tmp;

    for (tmp = Scr->MenuList; tmp != NULL; tmp = tmp->next)
    {
	if (strcmp(name, tmp->name) == 0)
	    return (tmp);
    }
    return NULL;
}

CheckButton(dpy, event, count)
Display *dpy;
XEvent *event;
int *count;
{
    if (event->type == ButtonRelease)
    {
	*count = TRUE;
	return TRUE;
    }
    *count -= 1;
    if (*count <= 0)
    {
	*count = FALSE;
	return TRUE;
    }
    return FALSE;
}

/***********************************************************************
 *
 *  Procedure:
 *	ExecuteFunction - execute a twm root function
 *
 *  Inputs:
 *	func	- the function to execute
 *	action	- the menu action to execute 
 *	w	- the window to execute this function on
 *	tmp_win	- the twm window structure
 *	event	- the event that caused the function
 *	context - the context in which the button was pressed
 *	pulldown- flag indicating execution from pull down menu
 *
 *  Returns:
 *	TRUE if should continue with remaining actions else FALSE to abort
 *
 ***********************************************************************
 */

int
ExecuteFunction(func, action, w, tmp_win, event, context, pulldown)
    int func;
    char *action;
    Window w;
    TwmWindow *tmp_win;
    XEvent event;
    int context;
    int pulldown;
{
    static Time last_time = 0;

    char tmp[200];
    char *ptr;
    int len;
    char buff[MAX_FILE_SIZE];
    int count, fd;
    MenuRoot *root, *tmp_root;
    MenuItem *item, *tmp_item;
    ScreenInfo *oldScr;
    int scrnum;
    Window rootw;
    int origX, origY;
    int do_next_action = TRUE;

    RootFunction = NULL;
    if (Cancel)
	return TRUE;			/* XXX should this be FALSE? */

    switch (func)
    {
    case F_UPICONMGR:
    case F_LEFTICONMGR:
    case F_RIGHTICONMGR:
    case F_DOWNICONMGR:
    case F_FORWICONMGR:
    case F_BACKICONMGR:
    case F_NEXTICONMGR:
    case F_PREVICONMGR:
    case F_NOP:
    case F_TITLE:
    case F_DELTASTOP:
    case F_RAISELOWER:
    case F_WARPTOSCREEN:
	break;
    default:
        XGrabPointer(dpy, Scr->Root, True,
            ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync,
            Scr->Root, Scr->WaitCursor, CurrentTime);
	break;
    }

    switch (func)
    {
    case F_NOP:
    case F_TITLE:
	break;

    case F_DELTASTOP:
	if (WindowMoved) do_next_action = FALSE;
	break;

    case F_RESTART:
	XSync (dpy, 0);
	Reborder ();
	execvp(*Argv, Argv, Environ);
	XSync (dpy, 0);
	fprintf(stderr, "%s:  restart failed\n", *Argv);
	break;

    case F_UPICONMGR:
    case F_DOWNICONMGR:
    case F_LEFTICONMGR:
    case F_RIGHTICONMGR:
    case F_FORWICONMGR:
    case F_BACKICONMGR:
        if (context == C_ICONMGR)
	    MoveIconManager(func);
        break;

    case F_NEXTICONMGR:
    case F_PREVICONMGR:
        if (context == C_ICONMGR)
	    JumpIconManager(func);
        break;

    case F_SHOWLIST:
	if (Scr->NoIconManagers)
	    break;
	DeIconify(Scr->iconmgr.twm_win);
	XRaiseWindow(dpy, Scr->iconmgr.twm_win->frame);
	break;

    case F_HIDELIST:
	if (Scr->NoIconManagers)
	    break;
	XUnmapWindow(dpy, Scr->iconmgr.twm_win->frame);
	if (Scr->iconmgr.twm_win->icon_w)
	    XUnmapWindow(dpy, Scr->iconmgr.twm_win->icon_w);
	Scr->iconmgr.twm_win->mapped = FALSE;
	Scr->iconmgr.twm_win->icon = TRUE;
	break;

    case F_SORTICONMGR:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	{
	    int save_sort;

	    save_sort = Scr->SortIconMgr;
	    Scr->SortIconMgr = TRUE;

	    if (context == C_ICONMGR)
		SortIconManager(NULL);
	    else if (tmp_win->iconmgr)
		SortIconManager(tmp_win->iconmgrp);
	    else
		XBell(dpy, 0);

	    Scr->SortIconMgr = save_sort;
	}
	break;

    case F_IDENTIFY:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	Identify(tmp_win);
	break;

    case F_AUTORAISE:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	tmp_win->auto_raise = !tmp_win->auto_raise;
	break;

    case F_BEEP:
	XBell(dpy, 0);
	break;

    case F_POPUP:
	tmp_win = (TwmWindow *)action;
	if (Scr->WindowFunction.func != NULL)
	{
	   ExecuteFunction(Scr->WindowFunction.func,
			   Scr->WindowFunction.item->action,
			   w, tmp_win, event, C_FRAME, FALSE);
	}
	else
	{
	    DeIconify(tmp_win);
	    XMapWindow(dpy, tmp_win->w);
	    XMapRaised(dpy, tmp_win->frame);
	}
	break;

    case F_RESIZE:
	EventHandler[EnterNotify] = HandleUnknown;
	EventHandler[LeaveNotify] = HandleUnknown;
	if (DeferExecution(context, func, Scr->MoveCursor))
	    return TRUE;

	if (pulldown)
	    XWarpPointer(dpy, None, Scr->Root, 
		0, 0, 0, 0, event.xbutton.x_root, event.xbutton.y_root);

	if (w != tmp_win->icon_w)
	{
	    StartResize(event, tmp_win);
	    return TRUE;
	}
	break;


    case F_ZOOM:
    case F_HORIZOOM:
    case F_FULLZOOM:
    case F_LEFTZOOM:
    case F_RIGHTZOOM:
    case F_TOPZOOM:
    case F_BOTTOMZOOM:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;
	fullzoom(tmp_win, func);
	break;

    case F_MOVE:
    case F_FORCEMOVE:
	if (DeferExecution(context, func, Scr->MoveCursor))
	    return TRUE;

	PopDownMenu();
	rootw = event.xbutton.root;
	MoveFunction = func;

	if (pulldown)
	    XWarpPointer(dpy, None, Scr->Root, 
		0, 0, 0, 0, event.xbutton.x_root, event.xbutton.y_root);

	EventHandler[EnterNotify] = HandleUnknown;
	EventHandler[LeaveNotify] = HandleUnknown;

#ifdef NOT_NEEDED
	/* redraw the text in the title bar or the icon window if
	 * needed, we have disabled expose event handling so we must
	 * do it here
	 */
	if (context == C_TITLE)
	{
	    FBF(tmp_win->title.fore, tmp_win->title.back,
		Scr->TitleBarFont.font->fid);
	    XDrawImageString(dpy, tmp_win->title_w,
		Scr->NormalGC,
		TitleBarX, Scr->TitleBarFont.y,
		tmp_win->name, strlen(tmp_win->name));
	}
	else if (context == C_ICON && tmp_win->icon_w)
	{
	    FBF(tmp_win->iconc.fore, tmp_win->iconc.back,
		Scr->IconFont.font->fid);
	    XDrawImageString(dpy, tmp_win->icon_w,
		Scr->NormalGC,
		tmp_win->icon_x, tmp_win->icon_y,
		tmp_win->icon_name, strlen(tmp_win->icon_name));
	}
#endif

	if (!Scr->NoGrabServer || !Scr->OpaqueMove) {
	    XGrabServer(dpy);
	}
	XGrabPointer(dpy, event.xbutton.root, True,
	    ButtonPressMask | ButtonReleaseMask,
	    GrabModeAsync, GrabModeAsync,
	    Scr->Root, Scr->MoveCursor, CurrentTime);

	if (context == C_ICON && tmp_win->icon_w)
	{
	    w = tmp_win->icon_w;
	    DragX = event.xbutton.x;
	    DragY = event.xbutton.y;
	}
	else if (w != tmp_win->icon_w)
	{
	    XTranslateCoordinates(dpy, w, tmp_win->frame,
		event.xbutton.x, 
		event.xbutton.y, 
		&DragX, &DragY, &JunkChild);

	    w = tmp_win->frame;
	}

	DragWindow = NULL;

	XGetGeometry(dpy, w, &JunkRoot, &JunkX, &JunkY,
	    &DragWidth, &DragHeight, &JunkBW,
	    &JunkDepth);

	origX = event.xbutton.x_root;
	origY = event.xbutton.y_root;
	/*
	MoveOutline((Window)event.xbutton.root,
	    event.xbutton.x_root-DragX-JunkBW,
	    event.xbutton.y_root-DragY-JunkBW,
	    DragWidth + 2 * JunkBW,
	    DragHeight + 2 * JunkBW, 
	    tmp->tmp_win->frame_bw, tmp_win->title_height);
	*/

	if ((event.xbutton.time - last_time) < 400)
	{
	    int width, height;

	    ConstMove = TRUE;
	    ConstMoveDir = MOVE_NONE;
	    ConstMoveX = event.xbutton.x_root - DragX - JunkBW;
	    ConstMoveY = event.xbutton.y_root - DragY - JunkBW;
	    width = DragWidth + 2 * JunkBW;
	    height = DragHeight + 2 * JunkBW;
	    ConstMoveXL = ConstMoveX + width/3;
	    ConstMoveXR = ConstMoveX + 2*(width/3);
	    ConstMoveYT = ConstMoveY + height/3;
	    ConstMoveYB = ConstMoveY + 2*(height/3);

	    XWarpPointer(dpy, None, w,
		0, 0, 0, 0, DragWidth/2, DragHeight/2);

	    XQueryPointer(dpy, w, &JunkRoot, &JunkChild,
		&JunkX, &JunkY, &DragX, &DragY, &JunkMask);
	}
	last_time = event.xbutton.time;

	while (TRUE)
	{
	    int done;

	    done = FALSE;
	    while (XCheckMaskEvent(dpy, ButtonPressMask | ButtonReleaseMask |
					EnterWindowMask | LeaveWindowMask,
		&Event))
	    {
		/* throw away enter and leave events until release */
		if (Event.xany.type == EnterNotify ||
		    Event.xany.type == LeaveNotify) continue; 

		if (XFindContext(dpy, Event.xany.window,
		    TwmContext,&Tmp_win) == XCNOENT)
		    Tmp_win = NULL;

		if (XFindContext(dpy, Event.xany.window,
		    ScreenContext, &Scr) == XCNOENT)
		    Scr = FindScreenInfo(Event.xany.window);

		if (Scr == NULL)
		    continue;

		if (Event.type >= 0 && Event.type < LASTEvent)
		    (*EventHandler[Event.type])();

		if (Cancel)
		{
		    WindowMoved = FALSE;
		    return TRUE;	/* XXX should this be FALSE? */
		}
		if (Event.type == ButtonRelease)
		{
		    MoveOutline(rootw, 0, 0, 0, 0, 0, 0);
		    done = TRUE;
		    break;
		}
	    }

	    if (done)
		break;

	    XQueryPointer(dpy, rootw, &(event.xmotion.root), &JunkChild,
		&(event.xmotion.x_root), &(event.xmotion.y_root),
		&JunkX, &JunkY, &JunkMask);

	    if (DragWindow == NULL &&
		abs(event.xmotion.x_root - origX) < Scr->MoveDelta &&
	        abs(event.xmotion.y_root - origY) < Scr->MoveDelta)
		continue;

	    WindowMoved = TRUE;
	    DragWindow = w;

	    if (ConstMove)
	    {
		switch (ConstMoveDir)
		{
		    case MOVE_NONE:
			if (event.xmotion.x_root < ConstMoveXL ||
			    event.xmotion.x_root > ConstMoveXR)
			    ConstMoveDir = MOVE_HORIZ;

			if (event.xmotion.y_root < ConstMoveYT ||
			    event.xmotion.y_root > ConstMoveYB)
			    ConstMoveDir = MOVE_VERT;

			XQueryPointer(dpy, DragWindow, &JunkRoot, &JunkChild,
			    &JunkX, &JunkY, &DragX, &DragY, &JunkMask);
			break;

		    case MOVE_VERT:
			ConstMoveY = event.xmotion.y_root - DragY - JunkBW;
			break;

		    case MOVE_HORIZ:
			ConstMoveX= event.xmotion.x_root - DragX - JunkBW;
			break;
		}

		if (ConstMoveDir != MOVE_NONE)
		{
		    int xl, yt, xr, yb, w, h;

		    xl = ConstMoveX;
		    yt = ConstMoveY;
		    w = DragWidth + 2 * JunkBW;
		    h = DragHeight + 2 * JunkBW;

		    if (Scr->DontMoveOff && MoveFunction != F_FORCEMOVE)
		    {
			xr = xl + w;
			yb = yt + h;

			if (xl < 0)
			    xl = 0;
			if (xr > Scr->MyDisplayWidth)
			    xl = Scr->MyDisplayWidth - w;

			if (yt < 0)
			    yt = 0;
			if (yb > Scr->MyDisplayHeight)
			    yt = Scr->MyDisplayHeight - h;
		    }
		    if (Scr->OpaqueMove)
			XMoveWindow(dpy, DragWindow, xl, yt);
		    else
			MoveOutline(event.xmotion.root, xl, yt, w, h,
				    tmp_win->frame_bw, tmp_win->title_height);
		}
	    }
	    else if (DragWindow != NULL)
	    {
		int xl, yt, xr, yb, w, h;

		xl = event.xmotion.x_root - DragX - JunkBW;
		yt = event.xmotion.y_root - DragY - JunkBW;
		w = DragWidth + 2 * JunkBW;
		h = DragHeight + 2 * JunkBW;

		if (Scr->DontMoveOff && MoveFunction != F_FORCEMOVE)
		{
		    xr = xl + w;
		    yb = yt + h;

		    if (xl < 0)
			xl = 0;
		    if (xr > Scr->MyDisplayWidth)
			xl = Scr->MyDisplayWidth - w;

		    if (yt < 0)
			yt = 0;
		    if (yb > Scr->MyDisplayHeight)
			yt = Scr->MyDisplayHeight - h;
		}

		if (Scr->OpaqueMove)
		    XMoveWindow(dpy, DragWindow, xl, yt);
		else
		    MoveOutline(event.xmotion.root, xl, yt, w, h,
				tmp_win->frame_bw, tmp_win->title_height);
	    }

	}
        break;

    case F_FUNCTION:
	{
	    MenuRoot *mroot;
	    MenuItem *mitem;

	    if ((mroot = FindMenuRoot(action)) == NULL)
	    {
		fprintf(stderr, "twm: couldn't find function \"%s\"\n", action);
		return TRUE;
	    }

	    if (NeedToDefer(mroot) && DeferExecution(context, func, Scr->SelectCursor))
		return TRUE;
	    else
	    {
		for (mitem = mroot->first; mitem != NULL; mitem = mitem->next)
		{
		    if (!ExecuteFunction (mitem->func, mitem->action, w,
					  tmp_win, event, context, pulldown)) {
			break;
		    }
		}
	    }
	}
	break;

    case F_DEICONIFY:
    case F_ICONIFY:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	if (tmp_win->icon)
	{
	    DeIconify(tmp_win);
	}
        else if (func == F_ICONIFY)
	{
	    Iconify(tmp_win, event.xbutton.x_root-5, event.xbutton.y_root-5);
	}
	SetHints(tmp_win);
	break;

    case F_RAISELOWER:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	if (!WindowMoved)
	{
	    int vis;
	    int bw;

	    if (w == tmp_win->icon_w)
	    {
		vis = tmp_win->icon_vis;
		bw = BW2;
	    }
	    else
	    {
		w = tmp_win->frame;
		vis = tmp_win->frame_vis;
		bw = (tmp_win->frame_bw + tmp_win->bw) * 2;
	    }

	    if (vis == VisibilityUnobscured)
		XLowerWindow(dpy, w);
	    else
	    {
		int myX, myY, myWidth, myHeight;
		int x, y, width, height;
		Window *children;
		int nchildren;
		int i;

		/*
		 * If the window is above all of its siblings, but partially
		 * offscreen, its visibility is VisibilityPartiallyObscured,
		 * but we'd want to lower it.
		 */
		XGetGeometry(dpy, w, &JunkRoot, 
			     &myX, &myY, &myWidth, &myHeight,
			     &JunkBW, &JunkDepth);
		
		/* If it's completely onscreen, it must be obscured. */
		if (myX > 0 && (myX + myWidth+bw) <= Scr->MyDisplayWidth &&
		    myY > 0 && (myY + myHeight+bw) <= Scr->MyDisplayHeight)
		{
		    XRaiseWindow(dpy, w);
		}
		else
		{
		    XQueryTree(dpy, Scr->Root, &JunkRoot, &JunkParent,
			       &children, &nchildren);
		    /*
		     * Start at the upper-most window and work down.  Look
		     * for an obscuring sibling above w.
		     */
		    for (i = nchildren - 1; i >= 0; i--)
		    {
			if (children[i] == w)
			{
			    XLowerWindow(dpy, w);
			    break;
			}
			else
			{
			    XGetGeometry(dpy, children[i], &JunkRoot,
					 &x, &y, &width, &height,
					 &JunkBW, &JunkDepth);
			    if (x + width >= myX && x < myX + myWidth &&
				y + height >= myY && y < myY + myHeight)
			    {
				XRaiseWindow(dpy, w);
				break;
			    }
			}
		    } /* for */
		    XFree(children);
		}
	    }
	}
	break;
	
    case F_RAISE:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	if (w == tmp_win->icon_w)
	    XRaiseWindow(dpy, tmp_win->icon_w);
	else
	    XRaiseWindow(dpy, tmp_win->frame);

	break;

    case F_LOWER:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	if (w == tmp_win->icon_w)
	    XLowerWindow(dpy, tmp_win->icon_w);
	else
	    XLowerWindow(dpy, tmp_win->frame);

	break;

    case F_FOCUS:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	if (tmp_win->icon == FALSE)
	{
	    XWindowAttributes attr;

	    if (!Scr->FocusRoot && Scr->Focus == tmp_win)
	    {
		FocusOnRoot();
	    }
	    else
	    {
		if (Scr->Focus != NULL)
		{
		    if (Scr->Focus->highlight)
		    {
			XSetWindowBorderPixmap(dpy, Scr->Focus->frame, 
			    Scr->Focus->gray);
			if (Scr->Focus->title_w)
			    XSetWindowBorderPixmap(dpy, Scr->Focus->title_w, 
				Scr->Focus->gray);
		    }
		    if (Scr->Focus->hilite_w)
			XUnmapWindow(dpy, Scr->Focus->hilite_w);
		}

		XGetWindowAttributes(dpy, tmp_win->w, &attr);
		tmp_win->attr.colormap = attr.colormap;
		XInstallColormap(dpy, tmp_win->attr.colormap);
		if (tmp_win->hilite_w)
		    XMapWindow(dpy, tmp_win->hilite_w);
		XSetWindowBorder(dpy, tmp_win->frame, tmp_win->border);
		if (tmp_win->title_w)
		    XSetWindowBorder(dpy, tmp_win->title_w, tmp_win->border);
		XSetInputFocus(dpy, tmp_win->w, RevertToPointerRoot,
			CurrentTime);
		Scr->FocusRoot = FALSE;
		Scr->Focus = tmp_win;
	    }
	}
	break;

    case F_DESTROY:
	if (DeferExecution(context, func, Scr->DestroyCursor))
	    return TRUE;

	if (tmp_win->iconmgr)
	    XBell(dpy, 0);
	else
	    XKillClient(dpy, tmp_win->w);
	break;

    case F_CIRCLEUP:
	XCirculateSubwindowsUp(dpy, Scr->Root);
	break;

    case F_CIRCLEDOWN:
	XCirculateSubwindowsDown(dpy, Scr->Root);
	break;

    case F_VERSION:
	XMapRaised(dpy, Scr->VersionWindow);
	break;

    case F_EXEC:
	Execute(action);
	break;

    case F_UNFOCUS:
	FocusOnRoot();
	break;

    case F_CUT:
	strcpy(tmp, action);
	strcat(tmp, "\n");
	XStoreBytes(dpy, tmp, strlen(tmp));
	break;

    case F_CUTFILE:
	ptr = XFetchBytes(dpy, &count);
	if (count != 0)
	{
	    if (sscanf(ptr, "%s", tmp) == 1)
	    {
		ptr = ExpandFilename(tmp);
		fd = open(ptr, 0);
		if (fd >= 0)
		{
		    count = read(fd, buff, MAX_FILE_SIZE - 1);
		    if (count > 0)
			XStoreBytes(dpy, buff, count);

		    close(fd);
		}
		else
		{
		    fprintf(stderr, "twm: couldn't open \"%s\"\n", tmp);
		}
	    }
	    XFree(ptr);
	}
	else
	{
	    fprintf(stderr, "twm: nothing in the cut buffer\n");
	}
	break;

    case F_WARPTOSCREEN:
	{
	    if (strcmp (action, WARPSCREEN_NEXT) == 0) {
		WarpToScreen (Scr->screen + 1, 1);
	    } else if (strcmp (action, WARPSCREEN_PREV) == 0) {
		WarpToScreen (Scr->screen - 1, -1);
	    } else if (strcmp (action, WARPSCREEN_BACK) == 0) {
		WarpToScreen (PreviousScreen, 0);
	    } else {
		WarpToScreen (atoi (action), 0);
	    }
	}
	break;

    case F_WARPTO:
	{
	    TwmWindow *t;
	    int len;

	    len = strlen(action);

	    for (t = Scr->TwmRoot.next; t != NULL; t = t->next)
	    {
		/* match only the first portion of WINDOW the name */
		if (!strncmp(action, t->name, len))
		{
		    if (t->mapped)
		    {
			XRaiseWindow(dpy, t->frame);
			XWarpPointer(dpy, None, t->w, 0, 0, 0, 0, 10, 10);
			break;
		    }
		}
	    }
	}
	break;

    case F_WARPTOICONMGRE:
	{
	    TwmWindow *t;
	    int len;

	    len = strlen(action);

	    for (t = Scr->TwmRoot.next; t != NULL; t = t->next)
	    {
		/* match only the first portion of ICON the name */
		if (!strncmp(action, t->icon_name, len))
		{
		    if (t->list && t->list->iconmgr->twm_win->mapped)
		    {
			XRaiseWindow(dpy, t->list->iconmgr->twm_win->frame);
			XWarpPointer(dpy, None, t->list->icon, 0,0,0,0, 5, 5);
			break;
		    }
		}
	    }
	}
	break;
	
    case F_WARPTOICONMGR:
	if (tmp_win && !tmp_win->iconmgr)
	{
	    if (tmp_win->list && tmp_win->list->iconmgr->twm_win->mapped)
	    {
		XRaiseWindow(dpy, tmp_win->list->iconmgr->twm_win->frame);
		XWarpPointer(dpy, None, tmp_win->list->w, 0, 0, 0, 0, 5, 5);
	    }
	}
	break;

    case F_FILE:
	action = ExpandFilename(action);
	fd = open(action, 0);
	if (fd >= 0)
	{
	    count = read(fd, buff, MAX_FILE_SIZE - 1);
	    if (count > 0)
		XStoreBytes(dpy, buff, count);

	    close(fd);
	}
	else
	{
	    fprintf(stderr, "twm: couldn't open \"%s\"\n", action);
	}
	break;

    case F_TWMRC:
	len = strlen(action);
	if (len == 0)
	    ptr = InitFile;
	else
	{
	    ptr = (char *)malloc(len+1);
	    if (ptr == NULL)
	    {
		fprintf(stderr, "twm: out of memory\n");
		exit(1);
	    }
	    strcpy(ptr, action);
	    ptr = ExpandFilename(ptr);
	}

	oldScr = Scr;
	FirstScreen = TRUE;
	for (scrnum = 0; scrnum < NumScreens; scrnum++)
	{
	    if ((Scr = ScreenList[scrnum]) == NULL)
		continue;

	    /* first get rid of the existing menu structure and destroy all
	     * windows */
	    for (root = Scr->MenuList; root != NULL;)
	    {
		for (item = root->last; item != NULL;)
		{
		    tmp_item = item;
		    item = item->prev;
		    free(tmp_item);
		}

		if (root->w)
		{
		    XDeleteContext(dpy, root->w, MenuContext);
		    XDeleteContext(dpy, root->w, ScreenContext);
		    if (Scr->Shadow)
			XDestroyWindow(dpy, root->shadow);
		    XDestroyWindow(dpy, root->w);
		}

		tmp_root = root;
		root = root->next;
		free(tmp_root);
	    }
	    Scr->MenuList = NULL;
	    Scr->LastMenu = NULL;
	    ActiveMenu = NULL;
	    ActiveItem = NULL;

	    UngrabAllButtons();
	    UngrabAllKeys();
	    FreeIconRegions();

	    ParseTwmrc(ptr);
	    MakeMenus();

#ifdef TOM
	    /* I started to write code to redo colors and stuff, but I 
	     * didn't get it all done, that's why it is ifdef'ed out
	     */
	    for (tmp_win = Scr->TwmRoot.next;
		 tmp_win != NULL;
		 tmp_win = tmp_win->next)
	    {
		if (tmp_win->title_w)
		{
		    XUnmapWindow(dpy, tmp_win->title_w);
		    XSetWindowBackground(dpy, tmp_win->title_w,
			Scr->TitleC.back);
		}
		XDestroyWindow(dpy, tmp_win->iconify_w, Scr->TitleC.back);
#ifndef NOFOCUS
		XDestroyWindow(dpy, tmp_win->focus_w, Scr->TitleC.back);
#endif
		XDestroyWindow(dpy, tmp_win->resize_w, Scr->TitleC.back);
		XDestroyWindow(dpy, tmp_win->hilite_w, Scr->TitleC.back);
		CreateTitleButtons(tmp_win);
		XMapWindow(dpy, tmp_win->title_w);
		SetupWindow(tmp_win, tmp_win->frame_x, tmp_win->frame_y,
			    tmp_win->frame_width, tmp_win->frame_height);
		if (tmp_win == Scr->Focus || !tmp_win->highlight)
		{
		    XSetWindowBorder(dpy, tmp_win->frame, tmp_win->border);
		    XSetWindowBorder(dpy, tmp_win->title_w, tmp_win->border);
		}
		if (tmp_win == Scr->Focus)
		    XMapWindow(tmp_win->hilite_w);
	    }
#endif /* TOM */
	    GrabAllButtons();
	    GrabAllKeys();
	    FirstScreen = FALSE;
	}

	Scr = oldScr;
	break;

    case F_REFRESH:
	w = XCreateSimpleWindow(dpy, Scr->Root,
	    0, 0, 9999, 9999, 0, Scr->Black, Scr->Black);
	XMapWindow(dpy, w);
	XDestroyWindow(dpy, w);
	XFlush(dpy);
	break;

    case F_WINREFRESH:
	if (DeferExecution(context, func, Scr->SelectCursor))
	    return TRUE;

	if (context == C_ICON && tmp_win->icon_w)
	    w = XCreateSimpleWindow(dpy, tmp_win->icon_w,
		0, 0, 9999, 9999, 0, Scr->Black, Scr->Black);
	else
	    w = XCreateSimpleWindow(dpy, tmp_win->frame,
		0, 0, 9999, 9999, 0, Scr->Black, Scr->Black);

	XMapWindow(dpy, w);
	XDestroyWindow(dpy, w);
	XFlush(dpy);
	break;

    case F_QUIT:
	Done();
	break;
    }

    if (ButtonPressed == -1)
	XUngrabPointer(dpy, CurrentTime);
    return do_next_action;
}

/***********************************************************************
 *
 *  Procedure:
 *	DeferExecution - defer the execution of a function to the
 *	    next button press if the context is C_ROOT
 *
 *  Inputs:
 *	context	- the context in which the mouse button was pressed
 *	func	- the function to defer
 *	cursor	- the cursor to display while waiting
 *
 ***********************************************************************
 */

int
DeferExecution(context, func, cursor)
int context, func;
Cursor cursor;
{
    if (context == C_ROOT)
    {
	LastCursor = cursor;
	XGrabPointer(dpy, Scr->Root, True,
	    ButtonPressMask | ButtonReleaseMask,
	    GrabModeAsync, GrabModeAsync,
	    Scr->Root, cursor, CurrentTime);

	RootFunction = func;

	return (TRUE);
    }
    
    return (FALSE);
}


/***********************************************************************
 *
 *  Procedure:
 *	ReGrab - regrab the pointer with the LastCursor;
 *
 ***********************************************************************
 */

ReGrab()
{
    XGrabPointer(dpy, Scr->Root, True,
	ButtonPressMask | ButtonReleaseMask,
	GrabModeAsync, GrabModeAsync,
	Scr->Root, LastCursor, CurrentTime);
}

/***********************************************************************
 *
 *  Procedure:
 *	NeedToDefer - checks each function in the list to see if it
 *		is one that needs to be defered.
 *
 *  Inputs:
 *	root	- the menu root to check
 *
 ***********************************************************************
 */

NeedToDefer(root)
MenuRoot *root;
{
    MenuItem *mitem;

    for (mitem = root->first; mitem != NULL; mitem = mitem->next)
    {
	switch (mitem->func)
	{
	case F_IDENTIFY:
	case F_RESIZE:
	case F_MOVE:
	case F_FORCEMOVE:
	case F_DEICONIFY:
	case F_ICONIFY:
	case F_RAISELOWER:
	case F_RAISE:
	case F_LOWER:
	case F_FOCUS:
	case F_DESTROY:
	case F_WINREFRESH:
	case F_ZOOM:
	case F_FULLZOOM:
	case F_HORIZOOM:
        case F_RIGHTZOOM:
        case F_LEFTZOOM:
        case F_TOPZOOM:
        case F_BOTTOMZOOM:
	case F_AUTORAISE:
	    return TRUE;
	}
    }
    return FALSE;
}

/***********************************************************************
 *
 *  Procedure:
 *	Execute - execute the string by /bin/sh
 *
 *  Inputs:
 *	s	- the string containing the command
 *
 ***********************************************************************
 */

void
Execute(s)
    char *s;
{
    int status, pid, w;
    SigProc istat, qstat;
    static char buf[256];
    char *ds = DisplayString (dpy);
    char *colon, *dot1;
    char oldDisplay[256];
    char *doisplay;

    oldDisplay[0] = '\0';
    doisplay=getenv("DISPLAY");
    if (doisplay)
	strcpy (oldDisplay, doisplay);

    /*
     * Build a display string using the current screen number, so that
     * X programs which get fired up from a menu come up on the screen
     * that they were invoked from, unless specifically overridden on
     * their command line.
     */
    colon = rindex (ds, ':');
    if (colon) {			/* if host[:]:dpy */
	strcpy (buf, "DISPLAY=");
	strcat (buf, ds);
	colon = buf + 8 + (colon - ds);	/* use version in buf */
	dot1 = index (colon, '.');	/* first period after colon */
	if (!dot1) dot1 = colon + strlen (colon);  /* if not there, append */
	sprintf (dot1, ".%d", Scr->screen);
	putenv (buf);
    }

    if ((pid = vfork()) == 0)
    {
	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGHUP, SIG_DFL);
#ifdef macII
	setpgrp();
#endif
	execl("/bin/sh", "sh", "-c", s, 0);
	_exit(127);
    }
    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);
    while ((w = wait(&status)) != pid && w != -1);
    if (w == -1)
	status = -1;
    signal(SIGINT, istat);
    signal(SIGQUIT, qstat);

    sprintf (buf, "DISPLAY=%s", oldDisplay);
    putenv (buf);
}

/***********************************************************************
 *
 *  Procedure:
 *	FocusOnRoot - put input focus on the root window
 *
 ***********************************************************************
 */

void
FocusOnRoot()
{
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    if (Scr->Focus != NULL)
    {
	if (Scr->Focus->highlight)
	{
	    XSetWindowBorderPixmap(dpy, Scr->Focus->frame, Scr->Focus->gray);
	    if (Scr->Focus->title_w)
		XSetWindowBorderPixmap(dpy, Scr->Focus->title_w,
		    Scr->Focus->gray);
	}
	if (Scr->Focus->hilite_w)
	    XUnmapWindow(dpy, Scr->Focus->hilite_w);
    }
    XInstallColormap(dpy, Scr->CMap);
    Scr->Focus = NULL;
    Scr->FocusRoot = TRUE;
}

DeIconify(tmp_win)
TwmWindow *tmp_win;
{
    TwmWindow *t;

    /* de-iconify group members (if any) */
    if (tmp_win->group == tmp_win->w)
    {
	for (t = Scr->TwmRoot.next; t != NULL; t = t->next)
	{
	    if (tmp_win->group == t->group &&
		tmp_win->group != t->w && t->icon)
	    {
		if (t->icon_on)
		    Zoom(t->icon_w, t->frame);
		else
		    Zoom(tmp_win->icon_w, t->frame);

		XMapWindow(dpy, t->w);
		t->mapped = TRUE;
		if (Scr->NoRaiseDeicon)
		    XMapWindow(dpy, t->frame);
		else
		    XMapRaised(dpy, t->frame);
		SetMapStateProp(t, NormalState);

		if (t->icon_w) {
		    XUnmapWindow(dpy, t->icon_w);
		    IconDown (t);
		}
		XUnmapWindow(dpy, t->list->icon);
		t->icon = FALSE;
		t->icon_on = FALSE;
	    }
	}
    }

    /* now de-iconify the main window */
    if (tmp_win->icon)
    {
	if (tmp_win->icon_on)
	    Zoom(tmp_win->icon_w, tmp_win->frame);
	else if (tmp_win->group != NULL)
	{
	    for (t = Scr->TwmRoot.next; t != NULL; t = t->next)
	    {
		if (tmp_win->group == t->w && t->icon_on)
		{
		    Zoom(t->icon_w, tmp_win->frame);
		    break;
		}
	    }
	}
    }


    XMapWindow(dpy, tmp_win->w);
    tmp_win->mapped = TRUE;
    if (Scr->NoRaiseDeicon)
	XMapWindow(dpy, tmp_win->frame);
    else
	XMapRaised(dpy, tmp_win->frame);
    SetMapStateProp(tmp_win, NormalState);

    if (tmp_win->icon_w) {
	XUnmapWindow(dpy, tmp_win->icon_w);
	IconDown (tmp_win);
    }
    if (tmp_win->list)
	XUnmapWindow(dpy, tmp_win->list->icon);
    if (Scr->WarpCursor && tmp_win->icon)
    {
	XWarpPointer(dpy, None, tmp_win->w,
	    0, 0, 0, 0, 5, 5);
    }
    tmp_win->icon = FALSE;
    tmp_win->icon_on = FALSE;
}

Iconify(tmp_win, def_x, def_y)
TwmWindow *tmp_win;
int def_x, def_y;
{
    TwmWindow *t;
    int iconify;
    XWindowAttributes winattrs;
    unsigned long eventMask;

    iconify = !tmp_win->iconify_by_unmapping;
    if (iconify)
    {
	if (tmp_win->icon_w == NULL)
	    CreateIconWindow(tmp_win, def_x, def_y);
	else
	    IconUp(tmp_win);
	XMapRaised(dpy, tmp_win->icon_w);
    }
    if (tmp_win->list)
	XMapWindow(dpy, tmp_win->list->icon);

    XGetWindowAttributes(dpy, tmp_win->w, &winattrs);
    eventMask = winattrs.your_event_mask;

    /* iconify group members first */
    if (tmp_win->group == tmp_win->w)
    {
	for (t = Scr->TwmRoot.next; t != NULL; t = t->next)
	{
	    if (tmp_win->group == t->group && tmp_win->group != t->w)
	    {
		if (iconify)
		{
		    if (t->icon_on)
			Zoom(t->icon_w, tmp_win->icon_w);
		    else
			Zoom(t->frame, tmp_win->icon_w);
		}

		/*
		 * Prevent the receipt of an UnmapNotify, since that would
		 * cause a transition to the Withdrawn state.
		 */
		t->mapped = FALSE;
		XSelectInput(dpy, t->w, eventMask & ~StructureNotifyMask);
		XUnmapWindow(dpy, t->w);
		XSelectInput(dpy, t->w, eventMask);
		XUnmapWindow(dpy, t->frame);
		if (t->icon_w)
		    XUnmapWindow(dpy, t->icon_w);
		SetMapStateProp(t, IconicState);
		if (tmp_win->highlight)
		{
		    XSetWindowBorderPixmap(dpy, tmp_win->frame, tmp_win->gray);
		    if (tmp_win->title_w)
			XSetWindowBorderPixmap(dpy,tmp_win->title_w,
			    tmp_win->gray);
		}
		if (t == Scr->Focus)
		{
		    XSetInputFocus(dpy, PointerRoot,
			RevertToPointerRoot, CurrentTime);
		    Scr->Focus = NULL;
		    Scr->FocusRoot = TRUE;
		}
		XMapWindow(dpy, t->list->icon);
		t->icon = TRUE;
		t->icon_on = FALSE;
	    }
	}
    }

    if (iconify)
	Zoom(tmp_win->frame, tmp_win->icon_w);

    /*
     * Prevent the receipt of an UnmapNotify, since that would
     * cause a transition to the Withdrawn state.
     */
    tmp_win->mapped = FALSE;
    XSelectInput(dpy, tmp_win->w, eventMask & ~StructureNotifyMask);
    XUnmapWindow(dpy, tmp_win->w);
    XSelectInput(dpy, tmp_win->w, eventMask);
    XUnmapWindow(dpy, tmp_win->frame);
    SetMapStateProp(tmp_win, IconicState);

    if (tmp_win->highlight)
    {
	XSetWindowBorderPixmap(dpy, tmp_win->frame, tmp_win->gray);
	if (tmp_win->title_w)
	    XSetWindowBorderPixmap(dpy, tmp_win->title_w, tmp_win->gray);
    }

    if (tmp_win == Scr->Focus)
    {
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
	    CurrentTime);
	Scr->Focus = NULL;
	Scr->FocusRoot = TRUE;
    }
    tmp_win->icon = TRUE;
    if (iconify)
	tmp_win->icon_on = TRUE;
    else
	tmp_win->icon_on = FALSE;
}

Identify(t)
TwmWindow *t;
{
    int i, n, twidth, width, height;
    int x, y, wwidth, wheight, bw, depth;
    Window junk;

    XGetGeometry(dpy, t->w, &JunkRoot, &JunkX, &JunkY,
	&wwidth, &wheight, &bw, &depth);
    XTranslateCoordinates(dpy, t->w, Scr->Root, JunkX, JunkY, &x, &y, &junk);

    n = 0;
    strcpy(Info[n++], "Window information");
    Info[n++][0] = '\0';
    sprintf(Info[n++], "Name             = \"%s\"", t->full_name);
    sprintf(Info[n++], "Class.res_name   = \"%s\"", t->class.res_name);
    sprintf(Info[n++], "Class.res_class  = \"%s\"", t->class.res_class);
    Info[n++][0] = '\0';
    sprintf(Info[n++], "Geometry/root    = %dx%d+%d+%d", wwidth, wheight,x,y);
    sprintf(Info[n++], "Border width     = %d", bw);
    sprintf(Info[n++], "Depth            = %d", depth);
    InfoLines = n;

    /* figure out the width and height of the info window */
    height = n * (Scr->DefaultFont.height+2);
    width = 1;
    for (i = 0; i < n; i++)
    {
	twidth = XTextWidth(Scr->DefaultFont.font, Info[i],
	    strlen(Info[i]));
	if (twidth > width)
	    width = twidth;
    }
    XUnmapWindow(dpy, Scr->InfoWindow);
    XFlush(dpy);
    XResizeWindow(dpy, Scr->InfoWindow, width+10, height);
    XMapRaised(dpy, Scr->InfoWindow);
/*
         ==> Upper left X: 577
         ==> Upper left Y: 162
         ==> Width: 501
         ==> Height: 803
         ==> Depth: 8
         ==> Border width: 2
         ==> Window class: InputOutput
         ==> Window Map State: IsViewable
*/

}

static Atom wmStateAtom = None;

SetMapStateProp(tmp_win, state)
TwmWindow *tmp_win;
int state;
{
    unsigned long data[2];		/* "suggested" by ICCCM version 1 */
  
    if (tmp_win->iconmgr) return;

    data[0] = (unsigned long) state;
    data[1] = (unsigned long) (tmp_win->iconify_by_unmapping ? None : 
			   tmp_win->icon_w);

    if (wmStateAtom == None) 
      wmStateAtom = XInternAtom (dpy, "WM_STATE", False);

    XChangeProperty (dpy, tmp_win->w, wmStateAtom, wmStateAtom, 32, 
		 PropModeReplace, (unsigned char *) data, 2);
}

Bool GetWMState (w, statep, iwp)
    int *statep;
    Window *iwp;
{
    Atom actual_type;
    int actual_format;
    long nitems, bytesafter;
    unsigned long *datap = NULL;
    Bool retval = False;

    if (wmStateAtom == None &&
        (wmStateAtom = XInternAtom (dpy, "WM_STATE", False)) == None)
      return False;

    if (XGetWindowProperty (dpy, w, wmStateAtom, 0, 2, False, wmStateAtom,
			    &actual_type, &actual_format, &nitems, &bytesafter,
			    (unsigned char **) &datap) != Success || !datap)
      return False;

    if (nitems <= 2) {			/* "suggested" by ICCCM version 1 */
	*statep = (int) datap[0];
	*iwp = (Window) datap[1];
	retval = True;
    }

    XFree ((char *) datap);
    return retval;
}


WarpToScreen (n, inc)
    int n, inc;
{
    Window dumwin;
    int x, y, dumint;
    unsigned int dummask;
    ScreenInfo *newscr = NULL;

    while (!newscr) {
					/* wrap around */
	if (n < 0) 
	  n = NumScreens - 1;
	else if (n >= NumScreens)
	  n = 0;

	newscr = ScreenList[n];
	if (!newscr) {			/* make sure screen is managed */
	    if (inc) {			/* walk around the list */
		n += inc;
		continue;
	    }
	    fprintf (stderr, "twm:  can't warp to unmanaged screen %d\n", n);
	    XBell (dpy, 0);
	    return;
	}
    }

    if (Scr->screen == n) return;	/* already on that screen */

    PreviousScreen = Scr->screen;
    XQueryPointer (dpy, Scr->Root, &dumwin, &dumwin, &x, &y,
		   &dumint, &dumint, &dummask);

    XWarpPointer (dpy, None, newscr->Root, 0, 0, 0, 0, x, y);
    return;
}
