/***********************************************************************
 *
 * $XConsortium: iconmgr.h,v 1.8 89/04/12 18:55:45 jim Exp $
 *
 * Icon Manager includes
 *
 * 09-Mar-89 Tom LaStrange		File Created
 *
 ***********************************************************************/

#ifndef _ICONMGR_
#define _ICONMGR_

typedef struct WList
{
    struct WList *next;
    struct WList *prev;
    struct TwmWindow *twm;
    struct IconMgr *iconmgr;
    Window w;
    Window icon;
    int x, y, width, height;
    int row, col;
    int me;
    unsigned fore, back, highlight;
    unsigned top, bottom;
    short active;
    short down;
} WList;

typedef struct IconMgr
{
    struct IconMgr *next;		/* pointer to the next icon manager */
    struct IconMgr *prev;		/* pointer to the previous icon mgr */
    struct IconMgr *lasti;		/* pointer to the last icon mgr */
    struct WList *first;		/* first window in the list */
    struct WList *last;			/* last window in the list */
    struct WList *active;		/* the active entry */
    TwmWindow *twm_win;			/* back pointer to the new parent */
    struct ScreenInfo *scr;		/* the screen this thing is on */
    Window w;				/* this icon manager window */
    char *geometry;			/* geometry string */
    char *name;
    char *icon_name;
    int x, y, width, height;
    int columns, cur_rows, cur_columns;
    int count;
} IconMgr;

extern int iconmgr_textx;
extern WList *DownIconManager;

#endif /* _ICONMGR_ */
