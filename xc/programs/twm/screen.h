/***********************************************************************
 *
 * $XConsortium: screen.h,v 1.22 89/04/11 08:05:14 toml Exp $
 *
 * twm per-screen data include file
 *
 * 11-3-88 Dave Payne, Apple Computer			File created
 *
 ***********************************************************************/

#ifndef _SCREEN_
#define _SCREEN_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include "list.h"
#include "menus.h"
#include "iconmgr.h"

typedef struct ScreenInfo
{
    int screen;			/* the default screen */
    int d_depth;		/* copy of DefaultDepth(dpy, screen) */
    Visual *d_visual;		/* copy of DefaultVisual(dpy, screen) */
    int Monochrome;		/* is the display monochrome ? */
    int MyDisplayWidth;		/* my copy of DisplayWidth(dpy, screen) */
    int MyDisplayHeight;	/* my copy of DisplayHeight(dpy, screen) */

    TwmWindow TwmRoot;		/* the head of the twm window list */

    Window Root;		/* the root window */
    Window VersionWindow;	/* the twm version window */
    Window SizeWindow;		/* the resize dimensions window */
    Window InitialWindow;	/* the window name we are creating */
    Window InfoWindow;		/* the information window */

    name_list *Icons;		/* list of icon pixmaps */
    Pixmap UnknownPm;		/* the unknown icon pixmap */
    Pixmap iconifyPm;		/* the titlebar iconify pixmap */
    Pixmap focusPm;		/* the titlebar focus pixmap */
    Pixmap resizePm;		/* the titlebar resize pixmap */
    Pixmap siconifyPm;		/* the icon manager iconify pixmap */
    Pixmap gray;		/* pixmap for 3D top */

    MenuRoot *MenuList;		/* head of the menu list */
    MenuRoot *LastMenu;		/* the last menu (mostly unused?) */
    MenuRoot *Windows;		/* the TwmWindows menu */

    MouseButton Mouse[MAX_BUTTONS+1][NUM_CONTEXTS][MOD_SIZE];
    MouseButton DefaultFunction;
    MouseButton WindowFunction;

    Colormap CMap;		/* default color map */
    ColorPair BorderTileC;	/* border tile colors */
    ColorPair TitleC;		/* titlebar colors */
    ColorPair MenuC;		/* menu colors */
    ColorPair MenuTitleC;	/* menu title colors */
    ColorPair IconC;		/* icon colors */
    ColorPair IconManagerC;	/* icon manager colors */
    ColorPair DefaultC;		/* default colors */
    int BorderColor;		/* color of window borders */
    int MenuShadowColor;	/* menu shadow color */
    int IconBorderColor;	/* icon border color */
    int IconManagerHighlight;	/* icon manager highlight */

    Cursor TitleCursor;		/* title bar cursor */
    Cursor FrameCursor;		/* frame cursor */
    Cursor IconCursor;		/* icon cursor */
    Cursor IconMgrCursor;	/* icon manager cursor */
    Cursor ButtonCursor;	/* title bar button cursor */
    Cursor MoveCursor;		/* move cursor */
    Cursor ResizeCursor;	/* resize cursor */
    Cursor WaitCursor;		/* wait a while cursor */
    Cursor MenuCursor;		/* menu cursor */
    Cursor SelectCursor;	/* dot cursor for f.move, etc. from menus */
    Cursor DestroyCursor;		/* skull and cross bones, f.destroy */

    name_list *BorderColorL;
    name_list *IconBorderColorL;
    name_list *BorderTileForegroundL;
    name_list *BorderTileBackgroundL;
    name_list *TitleForegroundL;
    name_list *TitleBackgroundL;
    name_list *IconForegroundL;
    name_list *IconBackgroundL;
    name_list *IconManagerFL;
    name_list *IconManagerBL;
    name_list *IconMgrs;
    name_list *NoTitle;		/* list of window names with no title bar */
    name_list *MakeTitle;	/* list of window names with title bar */
    name_list *AutoRaise;	/* list of window names to auto-raise */
    name_list *IconNames;	/* list of window names and icon names */
    name_list *NoHighlight;	/* list of windows to not highlight */
    name_list *NoTitleHighlight;/* list of windows to not highlight the TB*/
    name_list *DontIconify;	/* don't iconify by unmapping */
    name_list *IconMgrNoShow;	/* don't show in the icon manager */
    name_list *IconMgrShow;	/* show in the icon manager */
    name_list *IconifyByUn;	/* windows to iconify by unmapping */
    name_list *StartIconified;	/* windows to start iconic */
    name_list *IconManagerHighlightL;	/* icon manager highlight colors */

    GC NormalGC;		/* normal GC for everything */
    GC DrawGC;			/* GC to draw lines for move and resize */

    unsigned long Black;
    unsigned long White;
    unsigned long TopShadow;
    unsigned long BottomShadow;
    MyFont TitleBarFont;	/* title bar font structure */
    MyFont MenuFont;		/* menu font structure */
    MyFont IconFont;		/* icon font structure */
    MyFont SizeFont;		/* resize font structure */
    MyFont VersionFont;		/* version font structure */
    MyFont InitialFont;		/* window creation font structure */
    MyFont IconManagerFont;	/* window list font structure */
    MyFont DefaultFont;
    IconMgr iconmgr;		/* default icon manager */
    struct IconRegion *FirstRegion;	/* pointer to icon regions */
    struct IconRegion *LastRegion;	/* pointer to the last icon region */
    char *IconDirectory;	/* icon directory to search */
    int BorderWidth;		/* border width of twm windows */
    int IconBorderWidth;	/* border width of icon windows */
    int UnknownWidth;		/* width of the unknown icon */
    int UnknownHeight;		/* height of the unknown icon */
    int TitleHeight;		/* height of the title bar window */
    TwmWindow *Focus;		/* the twm window that has focus */
    int EntryHeight;		/* menu entry height */
    short ReverseVideo;		/* flag to do reverse video */
    short FocusRoot;		/* is the input focus on the root ? */
    short WarpCursor;		/* warp cursor on de-iconify ? */
    short ForceIcon;		/* force the icon to the user specified */
    short NoRaiseMove;		/* don't raise window following move */
    short NoRaiseResize;	/* don't raise window following resize */
    short NoRaiseDeicon;	/* don't raise window on deiconify */
    short DontMoveOff;		/* don't allow windows to be moved off */
    short DoZoom;		/* zoom in and out of icons */
    short TitleFocus;		/* focus on window in title bar ? */
    short NoTitlebar;		/* put title bars on windows */
    short DecorateTransients;	/* put title bars on transients */
    short IconifyByUnmapping;	/* simply unmap windows when iconifying */
    short ShowIconManager;	/* display the window list */
    short IconManagerDontShow;	/* show nothing in the icon manager */
    short BackingStore;		/* use backing store for menus */
    short SaveUnder;		/* use save under's for menus */
    short RandomPlacement;	/* randomly place windows that no give hints */
    short OpaqueMove;		/* move the window rather than outline */
    short Highlight;		/* should we highlight the window borders */
    short TitleHighlight;	/* should we highlight the titlebar */
    short MoveDelta;		/* number of pixels before f.move starts */
    short ZoomCount;		/* zoom outline count */
    short SortIconMgr;		/* sort entries in the icon manager */
    short Shadow;		/* show the menu shadow */
    short ShowVersion;		/*  show the version window on startup */
    short InterpolateMenuColors;/* make pretty menus */
    short ThreeD;		/* make stuff look 3D */
    short ThreeDMenus;		/* make stuff look 3D */
    short FlatMenus;		/* flat 3D menus */
    short ShadowsSpecified;	/* TopShadow or BottomShadow was parsed */
    short NoIconManagers;	/* Don't create any icon managers */
    short FirstTime;		/* first time we've read .twmrc */

    FuncKey FuncKeyRoot;
} ScreenInfo;

extern int MultiScreen;
extern int NumScreens;
extern ScreenInfo **ScreenList;
extern ScreenInfo *Scr;
extern int FirstScreen;

#endif _SCREEN_
