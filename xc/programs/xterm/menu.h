Widget CreateMainMenu(), CreateVTMenu(), CreateTekMenu();

typedef struct _MenuEntry {
    char *name;
    void (*function)();
} MenuEntry;

extern MenuEntry mainMenuEntries[], vtMenuEntries[], tekMenuEntries[];
extern Arg menuArgs[];


/*
 * The following definitions MUST match the order of entries given in 
 * the mainMenuEntries, vtMenuEntries, and tekMenuEntries arrays in menu.c.
 */

/*
 * items in primary menu
 */
#define mainMenu_line0 0
#define mainMenu_securekbd 1
#define mainMenu_logging 2
#define mainMenu_redraw 3
#define mainMenu_line1 4
#define mainMenu_suspend 5
#define mainMenu_continue 6
#define mainMenu_interrupt 7
#define mainMenu_hangup 8
#define mainMenu_terminate 9
#define mainMenu_kill 10
#define mainMenu_line2 11
#define mainMenu_quit 12

/*
 * items in vt100 mode menu
 */
#define vtMenu_line0 0
#define vtMenu_scrollbar 1
#define vtMenu_jumpscroll 2
#define vtMenu_reversevideo 3
#define vtMenu_autowrap 4
#define vtMenu_reversewrap 5
#define vtMenu_autolinefeed 6
#define vtMenu_appcursor 7
#define vtMenu_appkeypad 8
#define vtMenu_scrollkey 9
#define vtMenu_scrollinput 10
#define vtMenu_allow132 11
#define vtMenu_cursesemul 12
#define vtMenu_visualbell 13
#define vtMenu_marginbell 14
#define vtMenu_altscreen 15
#define vtMenu_line1 16
#define vtMenu_softreset 17
#define vtMenu_hardreset 18
#define vtMenu_line2 19
#define vtMenu_tekshow 20
#define vtMenu_tekmode 21
#define vtMenu_vthide 22

/*
 * items in tek4014 mode menu
 */
#define tekMenu_line0 0
#define tekMenu_tektextlarge 1
#define tekMenu_tektext2 2
#define tekMenu_tektext3 3
#define tekMenu_tektextsmall 4
#define tekMenu_line1 5
#define tekMenu_tekpage 6
#define tekMenu_tekreset 7
#define tekMenu_tekcopy 8
#define tekMenu_line2 9
#define tekMenu_vtshow 10
#define tekMenu_vtmode 11
#define tekMenu_tekhide 12


/*
 * macros for updating menus
 */

#define update_menu_item(w,itemname,val) { if (w) { \
    menuArgs[0].value = (XtArgVal) ((val) ? term->screen.menu_item_bitmap \
				          : None); \
    XawSimpleMenuSetEntryValues (w, itemname, menuArgs, (Cardinal) 1); }}


#define set_sensitivity(w,itemname,val) { if (w) { \
    menuArgs[1].value = (XtArgVal) (val); \
    XawSimpleMenuSetEntryValues (w, itemname, menuArgs+1, (Cardinal) 1); }}



/*
 * there should be one of each of the following for each checkable item
 */


#define update_securekbd() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_securekbd].name, \
		    term->screen.grabbedKbd)

#define update_logging() \
  update_menu_item (term->screen.mainMenu, \
		    mainMenuEntries[mainMenu_logging].name, \
		    term->screen.logging)


#define update_scrollbar() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_scrollbar].name, \
		    term->screen.scrollbar)

#define update_jumpscroll() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_jumpscroll].name, \
		    term->screen.jumpscroll)

#define update_reversevideo() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_reversevideo].name, \
		    (term->flags & REVERSE_VIDEO))

#define update_autowrap() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_autowrap].name, \
		    (term->flags & WRAPAROUND))

#define update_reversewrap() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_reversewrap].name, \
		    (term->flags & REVERSEWRAP))

#define update_autolinefeed() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_autolinefeed].name, \
		    (term->flags & LINEFEED))

#define update_appcursor() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_appcursor].name, \
		    (term->keyboard.flags & CURSOR_APL))

#define update_appkeypad() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_appkeypad].name, \
		    (term->keyboard.flags & KYPD_APL))

#define update_scrollkey() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_scrollkey].name,  \
		    term->screen.scrollkey)

#define update_scrollinput() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_scrollinput].name, \
		    term->screen.scrollinput)

#define update_allow132() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_allow132].name, \
		    term->screen.c132)
  
#define update_cursesemul() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_cursesemul].name, \
		    term->screen.curses)

#define update_visualbell() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_visualbell].name, \
		    term->screen.visualbell)

#define update_marginbell() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_marginbell].name, \
		    term->screen.marginbell)

#define update_altscreen() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_altscreen].name, \
		    term->screen.alternate)

#define update_tekshow() \
  update_menu_item (term->screen.vtMenu, \
		    vtMenuEntries[vtMenu_tekshow].name, \
		    term->screen.Tshow)

#define update_vttekmode() { \
    update_menu_item (term->screen.vtMenu, \
		      vtMenuEntries[vtMenu_tekmode].name, \
		      term->screen.TekEmu) \
    update_menu_item (term->screen.tekMenu, \
		      tekMenuEntries[tekMenu_vtmode].name, \
		      !term->screen.TekEmu) }

#define update_vtshow() \
  update_menu_item (term->screen.tekMenu, \
		    tekMenuEntries[tekMenu_vtshow].name, \
		    term->screen.Vshow)


#define set_vthide_sensitivity() \
  set_sensitivity (term->screen.vtMenu, \
		   vtMenuEntries[vtMenu_vthide].name, \
		   term->screen.Tshow)

#define set_tekhide_sensitivity() \
  set_sensitivity (term->screen.tekMenu, \
		   tekMenuEntries[tekMenu_tekhide].name, \
		   term->screen.Vshow)

/*
 * macros for mapping font size to tekMenu placement
 */
#define FS2MI(n) (n)			/* font_size_to_menu_item */
#define MI2FS(n) (n)			/* menu_item_to_font_size */

#define set_tekfont_menu_item(n,val) \
  update_menu_item (term->screen.tekMenu, \
		    tekMenuEntries[FS2MI(n)].name, \
		    (val))
