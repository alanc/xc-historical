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

static void do_securekbd(), do_visualbell(), do_logging(), do_redraw(),
  do_suspend(), do_continue(), do_interrupt(), do_hangup(), do_terminate(),
  do_kill(), do_quit(), do_scrollbar(), do_jumpscroll(), do_reversevideo(),
  do_autowrap(), do_reversewrap(), do_autolinefeed(), do_appcursor(), 
  do_appkeypad(), do_scrollkey(), do_scrollinput(), do_allow132(),
  do_cursesemul(), do_marginbell(), do_tekshow(), do_altscreen(),
  do_softreset(), do_hardreset(), do_tekmode(), do_vthide(),
  do_tektextlarge(), do_tektext2(), do_tektext3(), do_tektextsmall(),
  do_tekpage(), do_tekreset(), do_tekcopy(), do_vtshow(), do_vtmode(),
  do_tekhide();

/*
 * The order entries MUST match the values given in menu.h
 */
MenuEntry mainMenuEntries[] = {
    { "line0",		NULL },			/*  0 */
    { "securekbd",	do_securekbd },		/*  1 */
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
    { "line0",		NULL },			/*  0 */
    { "scrollbar",	do_scrollbar },		/*  1 */
    { "jumpscroll",	do_jumpscroll },	/*  2 */
    { "reversevideo",	do_reversevideo },	/*  3 */
    { "autowrap",	do_autowrap },		/*  4 */
    { "reversewrap",	do_reversewrap },	/*  5 */
    { "autolinefeed",	do_autolinefeed },	/*  6 */
    { "appcursor",	do_appcursor },		/*  7 */
    { "appkeypad",	do_appkeypad },		/*  8 */
    { "scrollkey",	do_scrollkey },		/*  9 */
    { "scrollinput",	do_scrollinput },	/* 10 */
    { "allow132",	do_allow132 },		/* 11 */
    { "cursesemul",	do_cursesemul },	/* 12 */
    { "visualbell",	do_visualbell },	/* 13 */
    { "marginbell",	do_marginbell },	/* 14 */
    { "altscreen",	do_altscreen },		/* 15 */
    { "line1",		NULL },			/* 16 */
    { "softreset",	do_softreset },		/* 17 */
    { "hardreset",	do_hardreset },		/* 18 */
    { "line2",		NULL },			/* 19 */
    { "tekshow",	do_tekshow },		/* 20 */
    { "tekmode",	do_tekmode },		/* 21 */
    { "vthide",		do_vthide }};		/* 22 */

MenuEntry tekMenuEntries[] = {
    { "line0",		NULL },			/*  0 */
    { "tektextlarge",	do_tektextlarge },	/*  1 */
    { "tektext2",	do_tektext2 },		/*  2 */
    { "tektext3",	do_tektext3 },		/*  3 */
    { "tektextsmall",	do_tektextsmall },	/*  4 */
    { "line1",		NULL },			/*  5 */
    { "tekpage",	do_tekpage },		/*  6 */
    { "tekreset",	do_tekreset },		/*  7 */
    { "tekcopy",	do_tekcopy },		/*  8 */
    { "line2",		NULL },			/*  9 */
    { "vtshow",		do_vtshow },		/* 10 */
    { "vtmode",		do_vtmode },		/* 11 */
    { "tekhide",	do_tekhide }};		/* 12 */

static Widget create_menu();


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

    if (screen->menu_item_bitmap == None) {
	screen->menu_item_bitmap =
	  XCreateBitmapFromData (XtDisplay(xtw),
				 RootWindowOfScreen(XtScreen(xtw)),
				 check_bits, check_width, check_height);
    }

    m = XtCreatePopupShell (name, simpleMenuWidgetClass, toplevel, NULL, 0);

   
    /*
     * XXX - this is temporary until this crocky interface is rewritten
     */
    for (; nentries > 0; nentries--, entries++) {
	XawSimpleMenuAddEntry (m, entries->name, NULL, (Cardinal) 0);
	if (entries->function)
	  XawSimpleMenuAddEntryCallback (m, entries->name, entries->function,
					 entries->name);
    }

    XtRealizeWidget (m);
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
    do_tekmode (gw, closure, data);
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
    do_vtmode (gw, closure, data);
}


