#include <stdio.h>
#include <signal.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/SimpleMenu.h>
#include "ptyx.h"
#include <setjmp.h>			/* for data.h */
#include "data.h"
#include "menu.h"

Arg menuArgs[2] = {{ XtNleftBitmap, (XtArgVal) 0 },
		   { XtNsensitive, (XtArgVal) 0 }};

static void do_securekbd(), do_allowsends(), do_visualbell(), do_logging(),
  do_redraw(), do_suspend(), do_continue(), do_interrupt(), do_hangup(),
  do_terminate(), do_kill(), do_quit(), do_scrollbar(), do_jumpscroll(),
  do_reversevideo(), do_autowrap(), do_reversewrap(), do_autolinefeed(),
  do_appcursor(), do_appkeypad(), do_scrollkey(), do_scrollinput(),
  do_allow132(), do_cursesemul(), do_marginbell(), do_tekshow(), 
  do_altscreen(), do_softreset(), do_hardreset(), do_tekmode(), do_vthide(), 
  do_tektextlarge(), do_tektext2(), do_tektext3(), do_tektextsmall(), 
  do_tekpage(), do_tekreset(), do_tekcopy(), do_vtshow(), do_vtmode(), 
  do_tekhide();


/*
 * The order entries MUST match the values given in menu.h
 */
MenuEntry mainMenuEntries[] = {
    { "securekbd",	do_securekbd },		/*  0 */
    { "allowsends",	do_allowsends },	/*  1 */
    { "logging",	do_logging },		/*  2 */
    { "redraw",		do_redraw },		/*  3 */
    { "line1",		NULL },			/*  4 */
    { "suspend",	do_suspend },		/*  5 */
    { "continue",	do_continue },		/*  6 */
    { "interrupt",	do_interrupt },		/*  7 */
    { "hangup",		do_hangup },		/*  8 */
    { "terminate",	do_terminate },		/*  9 */
    { "kill",		do_kill },		/* 10 */
    { "line2",		NULL },			/* 11 */
    { "quit",		do_quit }};		/* 12 */

MenuEntry vtMenuEntries[] = {
    { "scrollbar",	do_scrollbar },		/*  0 */
    { "jumpscroll",	do_jumpscroll },	/*  1 */
    { "reversevideo",	do_reversevideo },	/*  2 */
    { "autowrap",	do_autowrap },		/*  3 */
    { "reversewrap",	do_reversewrap },	/*  4 */
    { "autolinefeed",	do_autolinefeed },	/*  5 */
    { "appcursor",	do_appcursor },		/*  6 */
    { "appkeypad",	do_appkeypad },		/*  7 */
    { "scrollkey",	do_scrollkey },		/*  8 */
    { "scrollinput",	do_scrollinput },	/*  9 */
    { "allow132",	do_allow132 },		/* 10 */
    { "cursesemul",	do_cursesemul },	/* 11 */
    { "visualbell",	do_visualbell },	/* 12 */
    { "marginbell",	do_marginbell },	/* 13 */
    { "altscreen",	do_altscreen },		/* 14 */
    { "line1",		NULL },			/* 15 */
    { "softreset",	do_softreset },		/* 16 */
    { "hardreset",	do_hardreset },		/* 17 */
    { "line2",		NULL },			/* 18 */
    { "tekshow",	do_tekshow },		/* 19 */
    { "tekmode",	do_tekmode },		/* 20 */
    { "vthide",		do_vthide }};		/* 21 */

MenuEntry tekMenuEntries[] = {
    { "tektextlarge",	do_tektextlarge },	/*  0 */
    { "tektext2",	do_tektext2 },		/*  1 */
    { "tektext3",	do_tektext3 },		/*  2 */
    { "tektextsmall",	do_tektextsmall },	/*  3 */
    { "line1",		NULL },			/*  4 */
    { "tekpage",	do_tekpage },		/*  5 */
    { "tekreset",	do_tekreset },		/*  6 */
    { "tekcopy",	do_tekcopy },		/*  7 */
    { "line2",		NULL },			/*  8 */
    { "vtshow",		do_vtshow },		/*  9 */
    { "vtmode",		do_vtmode },		/* 10 */
    { "tekhide",	do_tekhide }};		/* 11 */

static Widget create_menu();
extern Widget toplevel;


/*
 * we really want to do these dynamically
 */
#define check_width 9
#define check_height 8
static char check_bits[] = {
   0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x00,
   0x31, 0x00, 0x1b, 0x00, 0x0e, 0x00, 0x04, 0x00
};


/*
 * public interfaces
 */

void create_dummy_menu_hack ()
{
    Widget w = XtCreateWidget ("dummyMenu", simpleMenuWidgetClass, 
			       toplevel, NULL, 0);
    XtDestroyWidget (w);
}

#ifdef notdef
Widget CreateMainMenu (xtw, toplevel)
    XtermWidget xtw;
    Widget toplevel;
{
    return create_menu (xtw, toplevel, "mainMenu",
			mainMenuEntries, XtNumber(mainMenuEntries));
}

Widget CreateVTMenu (xtw, toplevel)
    Widget toplevel;
{
    return create_menu (xtw, toplevel, "vtMenu",
			vtMenuEntries, XtNumber(vtMenuEntries));
}

Widget CreateTekMenu (xtw, toplevel)
    XtermWidget xtw;
    Widget toplevel;
{
    return create_menu (xtw, toplevel, "tekMenu",
			tekMenuEntries, XtNumber(tekMenuEntries));
}
#endif


void HandleCreateMenu (w, event, params, param_count)
    Widget w;
    XEvent *event;              /* unused */
    String *params;             /* mainMenu, vtMenu, or tekMenu */
    Cardinal *param_count;      /* 0 or 1 */
{
    TScreen *screen = &term->screen;
    int gotmenus = 0;

    if (*param_count != 1) {
	XBell (XtDisplay(w), 0);
    } else if (gotmenus < 3) {
	switch (params[0][0]) {
	  case 'm':
	    if (!screen->mainMenu) {
		screen->mainMenu = create_menu (term, toplevel, "mainMenu",
						mainMenuEntries,
						XtNumber(mainMenuEntries));
		update_securekbd();
		update_allowsends();
		update_visualbell();
		update_logging();
		gotmenus++;
	    }
	    break;
	  case 'v':
	    if (!screen->vtMenu) {
		screen->vtMenu = create_menu (term, toplevel, "vtMenu",
					      vtMenuEntries,
					      XtNumber(vtMenuEntries));
		/* and turn off the alternate screen entry */
		set_sensitivity (screen->vtMenu,
				 vtMenuEntries[vtMenu_altscreen].name, FALSE);
		update_scrollbar();
		update_jumpscroll();
		update_reversevideo();
		update_autowrap();
		update_reversewrap();
		update_autolinefeed();
		update_appcursor();
		update_appkeypad();
		update_scrollkey();
		update_scrollinput();
		update_allow132();
		update_cursesemul();
		update_marginbell();
		gotmenus++;
	    }
	    break;
	  case 't':
	    if (!screen->tekMenu) {
		screen->tekMenu = create_menu (term, toplevel, "tekMenu",
					       tekMenuEntries,
					       XtNumber(tekMenuEntries));
		set_tekfont_menu_item (screen->cur.fontsize, TRUE);
		gotmenus++;
	    }
	    break;
	  default:
	    XBell (XtDisplay(w), 0);
	    break;
	}
    }
    return;
}


/*
 * private interfaces - keep out!
 */

/*
 * create_menu - create a popup shell and stuff the menu into it.
 */

static Widget create_menu (xtw, toplevel, name, entries, nentries)
    XtermWidget xtw;
    Widget toplevel;
    char *name;
    struct _MenuEntry *entries;
    int nentries;
{
    Widget m;
    TScreen *screen = &xtw->screen;
    static XtCallbackRec cb[2] = { { NULL, NULL }, { NULL, NULL }};
    static Arg arg = { XtNcallback, (XtArgVal) cb };

    if (screen->menu_item_bitmap == None) {
	screen->menu_item_bitmap =
	  XCreateBitmapFromData (XtDisplay(xtw),
				 RootWindowOfScreen(XtScreen(xtw)),
				 check_bits, check_width, check_height);
    }

    m = XtCreatePopupShell (name, simpleMenuWidgetClass, toplevel, NULL, 0);

    for (; nentries > 0; nentries--, entries++) {
	cb[0].callback = (XtCallbackProc) entries->function;
	XawSimpleMenuAddEntry (m, entries->name, &arg, (Cardinal) 1);
    }

    /* do not realize at this point */
    return m;
}



/*
 * action routines
 */

void DoSecureKeyboard (time)
    Time time;
{
    do_securekbd (term->screen.mainMenu, NULL, NULL);
}

static void do_securekbd (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;
    Time time = CurrentTime;		/* XXX - wrong */

    if (screen->grabbedKbd) {
	XUngrabKeyboard (screen->display, time);
	ReverseVideo (term);
	screen->grabbedKbd = FALSE;
    } else {
	if (XGrabKeyboard (screen->display, term->core.parent->core.window,
			   True, GrabModeAsync, GrabModeAsync, time)
	    != GrabSuccess) {
	    XBell (screen->display, 100);
	} else {
	    ReverseVideo (term);
	    screen->grabbedKbd = TRUE;
	}
    }
    update_securekbd();
}


static void do_allowsends (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    screen->allowSendEvents = !screen->allowSendEvents;
    update_allowsends ();
}


static void do_visualbell (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    screen->visualbell = !screen->visualbell;
    update_visualbell();
}


static void do_logging (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (screen->logging) {
	CloseLog (screen);
    } else {
	StartLog (screen);
    }
    /* update_logging done by CloseLog and StartLog */
}


static void do_redraw (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    Redraw ();
}


/*
 * The following cases use the pid instead of the process group so that we
 * don't get hosed by programs that change their process group
 */


static void do_suspend (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
#if !defined(SYSV) || defined(JOBCONTROL)
    register TScreen *screen = &term->screen;

    if (screen->pid > 1) killpg (screen->pid, SIGTSTP);
#endif	/* !defined(SYSV) || defined(JOBCONTROL) */
}


static void do_continue (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
#if !defined(SYSV) || defined(JOBCONTROL)
    register TScreen *screen = &term->screen;

    if (screen->pid > 1) killpg (screen->pid, SIGCONT);
#endif	/* !defined(SYSV) || defined(JOBCONTROL) */
}


static void do_interrupt (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (screen->pid > 1) killpg (screen->pid, SIGINT);
}


static void do_hangup (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (screen->pid > 1) killpg (screen->pid, SIGHUP);
}


static void do_terminate (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (screen->pid > 1) killpg (screen->pid, SIGTERM);
}


static void do_kill (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (screen->pid > 1) killpg (screen->pid, SIGKILL);
}


static void do_quit (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    Cleanup (0);
}



/*
 * vt menu callbacks
 */

static void do_scrollbar (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (screen->scrollbar) {
	ScrollBarOff (screen);
    } else {
	ScrollBarOn (term, FALSE, FALSE);
    }
    update_scrollbar();
}


static void do_jumpscroll (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    term->flags ^= SMOOTHSCROLL;
    if (term->flags & SMOOTHSCROLL) {
	screen->jumpscroll = FALSE;
	if (screen->scroll_amt) FlushScroll(screen);
    } else {
	screen->jumpscroll = TRUE;
    }
    update_jumpscroll();
}


static void do_reversevideo (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    term->flags ^= REVERSE_VIDEO;
    ReverseVideo (term);
    /* update_reversevideo done in ReverseVideo */
}


static void do_autowrap (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    term->flags ^= WRAPAROUND;
    update_autowrap();
}


static void do_reversewrap (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    term->flags ^= REVERSEWRAP;
    update_reversewrap();
}


static void do_autolinefeed (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    term->flags ^= LINEFEED;
    update_autolinefeed();
}


static void do_appcursor (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    term->keyboard.flags ^= CURSOR_APL;
    update_appcursor();
}


static void do_appkeypad (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    term->keyboard.flags ^= KYPD_APL;
    update_appkeypad();
}


static void do_scrollkey (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    screen->scrollkey = !screen->scrollkey;
    update_scrollkey();
}


static void do_scrollinput (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    screen->scrollinput = !screen->scrollinput;
    update_scrollinput();
}


static void do_allow132 (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    screen->c132 = !screen->c132;
    update_allow132();
}


static void do_cursesemul (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    screen->curses = !screen->curses;
    update_cursesemul();
}


static void do_marginbell (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (!(screen->marginbell = !screen->marginbell)) screen->bellarmed = -1;
    update_marginbell();
}


static void do_tekshow (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (!screen->Tshow) {
	set_tek_visibility (TRUE);
    } else {
	set_tek_visibility (FALSE);
	end_tek_mode ();		/* WARNING: this does a longjmp */
    }
}


static void do_altscreen (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    /* do nothing for now; eventually, will want to flip screen */
}


static void do_softreset (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    VTReset (FALSE);
}


static void do_hardreset (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    VTReset (TRUE);
}


static void switch_modes (tovt)
    Bool tovt;				/* if true, then become vt mode */
{
    if (tovt) {
	if (TekRefresh) dorefresh();
	end_tek_mode ();		/* WARNING: this does a longjmp... */
    } else {
	end_vt_mode ();			/* WARNING: this does a longjmp... */
    }
}


static void do_tekmode (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    switch_modes (screen->TekEmu);	/* switch to tek mode */
}

static void do_vthide (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    set_vt_visibility (FALSE);
    if (!screen->TekEmu) switch_modes (False);	/* switch to tek mode */
}


/*
 * tek menu
 */

static void do_tektextlarge (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    TekSetFontSize (gw, tekMenu_tektextlarge);
}


static void do_tektext2 (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    TekSetFontSize (gw, tekMenu_tektext2);
}


static void do_tektext3 (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    TekSetFontSize (gw, tekMenu_tektext3);
}


static void do_tektextsmall (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{

    TekSetFontSize (gw, tekMenu_tektextsmall);
}


static void do_tekpage (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    TekSimulatePageButton (False);
}


static void do_tekreset (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    TekSimulatePageButton (True);
}


static void do_tekcopy (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    TekCopy ();
}


static void do_vtshow (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    if (!screen->Vshow) {
	set_vt_visibility (TRUE);
    } else {
	set_vt_visibility (FALSE);
	if (!screen->TekEmu && TekRefresh) dorefresh ();
	end_vt_mode ();
    }
}


static void do_vtmode (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    switch_modes (screen->TekEmu);	/* switch to vt, or from */
}


static void do_tekhide (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    set_tek_visibility (FALSE);
    TekRefresh = (TekLink *)0;
    if (screen->TekEmu) switch_modes (True);	/* does longjmp to vt mode */
}
