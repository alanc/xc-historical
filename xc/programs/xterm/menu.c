/* Copyright 1989 Massachusetts Institute of Technology */

#include <stdio.h>
#include <signal.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
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
  do_appcursor(), do_appkeypad(), do_scrollkey(), do_scrollttyoutput(),
  do_allow132(), do_cursesemul(), do_marginbell(), do_tekshow(), 
  do_altscreen(), do_softreset(), do_hardreset(), do_tekmode(), do_vthide(), 
  do_tektextlarge(), do_tektext2(), do_tektext3(), do_tektextsmall(), 
  do_tekpage(), do_tekreset(), do_tekcopy(), do_vtshow(), do_vtmode(), 
  do_tekhide(), do_vtfont();


/*
 * The order entries MUST match the values given in menu.h
 */
MenuEntry mainMenuEntries[] = {
    { "securekbd",	do_securekbd, NULL },		/*  0 */
    { "allowsends",	do_allowsends, NULL },		/*  1 */
    { "logging",	do_logging, NULL },		/*  2 */
    { "redraw",		do_redraw, NULL },		/*  3 */
    { "line1",		NULL, NULL },			/*  4 */
    { "suspend",	do_suspend, NULL },		/*  5 */
    { "continue",	do_continue, NULL },		/*  6 */
    { "interrupt",	do_interrupt, NULL },		/*  7 */
    { "hangup",		do_hangup, NULL },		/*  8 */
    { "terminate",	do_terminate, NULL },		/*  9 */
    { "kill",		do_kill, NULL },		/* 10 */
    { "line2",		NULL, NULL },			/* 11 */
    { "quit",		do_quit, NULL }};		/* 12 */

MenuEntry vtMenuEntries[] = {
    { "scrollbar",	do_scrollbar, NULL },		/*  0 */
    { "jumpscroll",	do_jumpscroll, NULL },		/*  1 */
    { "reversevideo",	do_reversevideo, NULL },	/*  2 */
    { "autowrap",	do_autowrap, NULL },		/*  3 */
    { "reversewrap",	do_reversewrap, NULL },		/*  4 */
    { "autolinefeed",	do_autolinefeed, NULL },	/*  5 */
    { "appcursor",	do_appcursor, NULL },		/*  6 */
    { "appkeypad",	do_appkeypad, NULL },		/*  7 */
    { "scrollkey",	do_scrollkey, NULL },		/*  8 */
    { "scrollttyoutput",	do_scrollttyoutput, NULL },	/*  9 */
    { "allow132",	do_allow132, NULL },		/* 10 */
    { "cursesemul",	do_cursesemul, NULL },		/* 11 */
    { "visualbell",	do_visualbell, NULL },		/* 12 */
    { "marginbell",	do_marginbell, NULL },		/* 13 */
    { "altscreen",	do_altscreen, NULL },		/* 14 */
    { "line1",		NULL, NULL },			/* 15 */
    { "softreset",	do_softreset, NULL },		/* 16 */
    { "hardreset",	do_hardreset, NULL },		/* 17 */
    { "line2",		NULL, NULL },			/* 18 */
    { "tekshow",	do_tekshow, NULL },		/* 19 */
    { "tekmode",	do_tekmode, NULL },		/* 20 */
    { "vthide",		do_vthide, NULL }};		/* 21 */

MenuEntry fontMenuEntries[] = {
    { "fontdefault",	do_vtfont, NULL },		/*  0 */
    { "font1",		do_vtfont, NULL },		/*  1 */
    { "font2",		do_vtfont, NULL },		/*  2 */
    { "font3",		do_vtfont, NULL },		/*  3 */
    { "font4",		do_vtfont, NULL },		/*  4 */
    { "fontescape",	do_vtfont, NULL },		/*  5 */
    { "fontsel",	do_vtfont, NULL }};		/*  6 */
    /* this should match NMENUFONTS in ptyx.h */

MenuEntry tekMenuEntries[] = {
    { "tektextlarge",	do_tektextlarge, NULL },	/*  0 */
    { "tektext2",	do_tektext2, NULL },		/*  1 */
    { "tektext3",	do_tektext3, NULL },		/*  2 */
    { "tektextsmall",	do_tektextsmall, NULL },	/*  3 */
    { "line1",		NULL, NULL },			/*  4 */
    { "tekpage",	do_tekpage, NULL },		/*  5 */
    { "tekreset",	do_tekreset, NULL },		/*  6 */
    { "tekcopy",	do_tekcopy, NULL },		/*  7 */
    { "line2",		NULL, NULL },			/*  8 */
    { "vtshow",		do_vtshow, NULL },		/*  9 */
    { "vtmode",		do_vtmode, NULL },		/* 10 */
    { "tekhide",	do_tekhide, NULL }};		/* 11 */

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

void HandleCreateMenu (w, event, params, param_count)
    Widget w;
    XEvent *event;              /* unused */
    String *params;             /* mainMenu, vtMenu, or tekMenu */
    Cardinal *param_count;      /* 0 or 1 */
{
    TScreen *screen = &term->screen;

    if (*param_count != 1) {
	XBell (XtDisplay(w), 0);
    } else {
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
	    }
	    break;
	  case 'v':
	    if (!screen->vtMenu) {
		screen->vtMenu = create_menu (term, toplevel, "vtMenu",
					      vtMenuEntries,
					      XtNumber(vtMenuEntries));
		/* and turn off the alternate screen entry */
		set_altscreen_sensitivity (FALSE);
		update_scrollbar();
		update_jumpscroll();
		update_reversevideo();
		update_autowrap();
		update_reversewrap();
		update_autolinefeed();
		update_appcursor();
		update_appkeypad();
		update_scrollkey();
		update_scrollttyoutput();
		update_allow132();
		update_cursesemul();
		update_marginbell();
	    }
	    break;
	  case 'f':
	    if (!screen->fontMenu) {
		screen->fontMenu = create_menu (term, toplevel, "fontMenu",
						fontMenuEntries,
						NMENUFONTS);  
		set_menu_font (True);
		set_sensitivity (screen->fontMenu,
				 fontMenuEntries[fontMenu_fontescape].widget,
				 (screen->menu_font_names[fontMenu_fontescape]
				  ? TRUE : FALSE));
	    }
	    get_selected_font ();
	    set_sensitivity (screen->fontMenu,
			     fontMenuEntries[fontMenu_fontsel].widget,
			     (screen->menu_font_names[fontMenu_fontsel]
			      ? TRUE : FALSE));
	    break;
	  case 't':
	    if (!screen->tekMenu) {
		screen->tekMenu = create_menu (term, toplevel, "tekMenu",
					       tekMenuEntries,
					       XtNumber(tekMenuEntries));
		set_tekfont_menu_item (screen->cur.fontsize, TRUE);
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
	cb[0].closure = (caddr_t) entries->name;
	entries->widget = XtCreateManagedWidget (entries->name, 
						 (entries->function ?
						  smeBSBObjectClass :
						  smeLineObjectClass), m,
						 &arg, (Cardinal) 1);
    }

    /* do not realize at this point */
    return m;
}

static void handle_send_signal (gw, sig)
    Widget gw;
    int sig;
{
    register TScreen *screen = &term->screen;

    if (screen->pid > 1) killpg (screen->pid, sig);
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
    handle_send_signal (gw, SIGTSTP);
#endif	/* !defined(SYSV) || defined(JOBCONTROL) */
}


static void do_continue (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
#if !defined(SYSV) || defined(JOBCONTROL)
    handle_send_signal (gw, SIGCONT);
#endif	/* !defined(SYSV) || defined(JOBCONTROL) */
}


static void do_interrupt (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    handle_send_signal (gw, SIGINT);
}


static void do_hangup (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    handle_send_signal (gw, SIGHUP);
}


static void do_terminate (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    handle_send_signal (gw, SIGTERM);
}


static void do_kill (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    handle_send_signal (gw, SIGKILL);
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


static void do_scrollttyoutput (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;

    screen->scrollttyoutput = !screen->scrollttyoutput;
    update_scrollttyoutput();
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


static void handle_tekshow (gw, allowswitch)
    Widget gw;
    Bool allowswitch;
{
    register TScreen *screen = &term->screen;

    if (!screen->Tshow) {		/* not showing, turn on */
	set_tek_visibility (TRUE);
    } else if (screen->Vshow || allowswitch) {  /* is showing, turn off */
	set_tek_visibility (FALSE);
	end_tek_mode ();		/* WARNING: this does a longjmp */
    } else
      Bell();
}


static void do_tekshow (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    handle_tekshow (gw, True);
}


static void do_tekonoff (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    handle_tekshow (gw, False);
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
 * vtfont menu
 */

static void do_vtfont (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    register TScreen *screen = &term->screen;
    char *entryname = (char *) closure;
    int i;

    for (i = 0; i < NMENUFONTS; i++) {
	if (strcmp (entryname, fontMenuEntries[i].name) == 0) {
	    set_vt_font (i, True);
	    return;
	}
    }
    Bell();
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


static void handle_vtshow (gw, allowswitch)
    Widget gw;
    Bool allowswitch;
{
    register TScreen *screen = &term->screen;

    if (!screen->Vshow) {		/* not showing, turn on */
	set_vt_visibility (TRUE);
    } else if (screen->Tshow || allowswitch) {  /* is showing, turn off */
	set_vt_visibility (FALSE);
	if (!screen->TekEmu && TekRefresh) dorefresh ();
	end_vt_mode ();
    } else 
      Bell();
}

static void do_vtshow (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    handle_vtshow (gw, True);
}

static void do_vtonoff (gw, closure, data)
    Widget gw;
    caddr_t closure, data;
{
    handle_vtshow (gw, False);
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



/*
 * public handler routines
 */

static void handle_toggle (proc, var, params, nparams, w, closure, data)
    void (*proc)();
    int var;
    String *params;
    Cardinal nparams;
    Widget w;
    caddr_t closure, data;
{
    int dir = -2;

    switch (nparams) {
      case 0:
	dir = -1;
      case 1:
	if (XmuCompareISOLatin1 (params[0], "on") == 0) dir = 1;
	else if (XmuCompareISOLatin1 (params[0], "off") == 0) dir = 0;
	else if (XmuCompareISOLatin1 (params[0], "toggle") == 0) dir = -1;
    }

    switch (dir) {
      case -2:
	Bell();
	break;

      case -1:
	(*proc) (w, closure, data);
	break;

      case 0:
	if (var) (*proc) (w, closure, data);
	else Bell();
	break;

      case 1:
	if (!var) (*proc) (w, closure, data);
	else Bell();
	break;
    }
    return;
}

void HandleAllowSends(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_allowsends, (int) term->screen.allowSendEvents,
		   params, *param_count, w, NULL, NULL);
}

void HandleVisualBell(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_visualbell, (int) term->screen.visualbell,
		   params, *param_count, w, NULL, NULL);
}

void HandleLogging(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_logging, (int) term->screen.logging,
		   params, *param_count, w, NULL, NULL);
}

void HandleRedraw(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    do_redraw(w, NULL, NULL);
}

void HandleSendSignal(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    static struct sigtab {
	char *name;
	int sig;
    } signals[] = {
	{ "suspend",	SIGTSTP },
	{ "tstp",	SIGTSTP },
	{ "cont",	SIGCONT },
	{ "int",	SIGINT },
	{ "hup",	SIGHUP },
	{ "term",	SIGTERM },
	{ "kill",	SIGKILL },
	{ NULL, 0 },
    };

    if (*param_count == 1) {
	struct sigtab *st;

	for (st = signals; st->name; st++) {
	    if (XmuCompareISOLatin1 (st->name, params[0]) == 0) {
		handle_send_signal (w, st->sig);
		return;
	    }
	}
	/* one could allow numeric values, but that would be a security hole */
    }

    Bell();
}

void HandleQuit(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    do_quit(w, NULL, NULL);
}

void HandleScrollbar(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_scrollbar, (int) term->screen.scrollbar,
		   params, *param_count, w, NULL, NULL);
}

void HandleJumpscroll(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_jumpscroll, (int) term->screen.jumpscroll,
		   params, *param_count, w, NULL, NULL);
}

void HandleReverseVideo(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_reversevideo, (int) (term->flags & REVERSE_VIDEO),
		   params, *param_count, w, NULL, NULL);
}

void HandleAutoWrap(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_autowrap, (int) (term->flags & WRAPAROUND),
		   params, *param_count, w, NULL, NULL);
}

void HandleReverseWrap(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_reversewrap, (int) (term->flags & REVERSEWRAP),
		   params, *param_count, w, NULL, NULL);
}

void HandleAutoLineFeed(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_autolinefeed, (int) (term->flags & LINEFEED),
		   params, *param_count, w, NULL, NULL);
}

void HandleAppCursor(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_appcursor, (int) (term->keyboard.flags & CURSOR_APL),
		   params, *param_count, w, NULL, NULL);
}

void HandleAppKeypad(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_appkeypad, (int) (term->keyboard.flags & KYPD_APL),
		   params, *param_count, w, NULL, NULL);
}

void HandleScrollKey(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_scrollkey, (int) term->screen.scrollkey,
		   params, *param_count, w, NULL, NULL);
}

void HandleScrollTtyOutput(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_scrollttyoutput, (int) term->screen.scrollttyoutput,
		   params, *param_count, w, NULL, NULL);
}

void HandleAllow132(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_allow132, (int) term->screen.c132,
		   params, *param_count, w, NULL, NULL);
}

void HandleCursesEmul(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_cursesemul, (int) term->screen.curses,
		   params, *param_count, w, NULL, NULL);
}

void HandleMarginBell(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    handle_toggle (do_marginbell, (int) term->screen.marginbell,
		   params, *param_count, w, NULL, NULL);
}

void HandleAltScreen(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    /* eventually want to see if sensitive or not */
    handle_toggle (do_altscreen, (int) term->screen.alternate,
		   params, *param_count, w, NULL, NULL);
}

void HandleSoftReset(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    do_softreset(w, NULL, NULL);
}

void HandleHardReset(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    do_hardreset(w, NULL, NULL);
}

void HandleSetTerminalType(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    if (*param_count == 1) {
	switch (params[0][0]) {
	  case 'v': case 'V':
	    if (term->screen.TekEmu) do_vtmode (w, NULL, NULL);
	    break;
	  case 't': case 'T':
	    if (!term->screen.TekEmu) do_tekmode (w, NULL, NULL);
	    break;
	  default:
	    Bell();
	}
    } else {
	Bell();
    }
}

void HandleVisibility(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    if (*param_count == 2) {
	switch (params[0][0]) {
	  case 'v': case 'V':
	    handle_toggle (do_vtonoff, (int) term->screen.Vshow,
			   params+1, (*param_count) - 1, w, NULL, NULL);
	    break;
	  case 't': case 'T':
	    handle_toggle (do_tekonoff, (int) term->screen.Tshow,
			   params+1, (*param_count) - 1, w, NULL, NULL);
	    break;
	  default:
	    Bell();
	}
    } else {
	Bell();
    }
}

void HandleSetTekText(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    void (*proc)() = NULL;

    switch (*param_count) {
      case 0:
	proc = do_tektextlarge;
	break;
      case 1:
	switch (params[0][0]) {
	  case 'l': case 'L': proc = do_tektextlarge; break;
	  case '2': proc = do_tektext2; break;
	  case '3': proc = do_tektext3; break;
	  case 's': case 'S': proc = do_tektextsmall; break;
	}
	break;
    }
    if (proc) (*proc) (w, NULL, NULL);
    else Bell();
}

void HandleTekPage(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    do_tekpage(w, NULL, NULL);
}

void HandleTekReset(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    do_tekreset(w, NULL, NULL);
}

void HandleTekCopy(w, event, params, param_count)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *param_count;
{
    do_tekcopy(w, NULL, NULL);
}


