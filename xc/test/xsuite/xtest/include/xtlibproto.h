/*
 * Copyright 1990, 1991, 1992 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: xtlibproto.h,v 1.1 92/06/11 15:32:22 rws Exp $
 */

/*
 * Do not edit this file.
 * It is generated automaticaly from the source files.
 */
#if __STDC__ == 1

void startup(void);
void cleanup(void);
void fontstartup(void);
void setxtfontpath(void);
void fontcleanup(void);
void focusstartup(void);
void focuscleanup(void);
void fontfocusstartup(void);
void fontfocuscleanup(void);
void rmstartup(void);
void aborttest(void);
void reset_delay(void);
void exec_startup(void);
void exec_cleanup(void);
char *boolname(int  val);
char *eventmaskname(unsigned long val);
char *eventname(int  val);
char *keymaskname(unsigned long val);
char *modifiername(int  val);
char *buttonmaskname(unsigned long val);
char *buttonname(int  val);
char *notifymodename(int  val);
char *notifydetailname(int  val);
char *visibilityname(int  val);
char *grabreplyname(int  val);
char *alloweventmodename(int  val);
char *reverttoname(int  val);
char *errorname(int  val);
char *classname(int  val);
char *bitgravityname(int  val);
char *wingravityname(int  val);
char *backingstorename(int  val);
char *gcfunctionname(int  val);
char *linestylename(int  val);
char *capstylename(int  val);
char *joinstylename(int  val);
char *fillstylename(int  val);
char *displayclassname(int  val);
char *protoname(int  val);
char *atomname(int  val);
char *mapstatename(int  val);
char *visualmaskname(unsigned long val);
char *contexterrorname(int  val);
int error_status(Display *disp, XErrorEvent *errevent);
int unexp_err(Display *disp, XErrorEvent *errevent);
int io_err(Display *disp);
int geterr(void);
int getbadvalue(void);
void reseterr(void);
int getevent(Display *disp, XEvent *event);
int stackorder(Display *disp, Window win);
void report(char *fmt, ...);
void trace(char *fmt, ...);
void check(char *fmt, ...);
void debug(int lev, char *fmt, ...);
void tccabort(char *fmt, ...);
void setdblev(int  n);
int getdblev(void);
Window iponlywin(Display *disp);
Window badwin(Display *disp);
void _startcall(Display *disp);
void startcall(Display *disp);
void _endcall(Display *disp);
void endcall(Display *disp);
Window makewin(Display *disp, XVisualInfo *vp);
void winpair(Display *disp, XVisualInfo *vp, Window *w1p, Window *w2p);
Drawable makewinpos(Display *disp, XVisualInfo *vp, int  x, int  y);
int checkevent(XEvent *good, XEvent *ev);
int initvclass(void);
void resetvclass(void);
int nextvclass(int  *vp, int  *dp);
int nvclass(void);
GC badgc(Display *disp);
Pixmap badpixm(Display *disp);
Font badfont(Display *disp);
void badvis(Visual *v);
void linkstart(void);
void linkclean(void);
int issuppvis(Display *disp, int  vis);
unsigned long visualsupported(Display *disp, unsigned long mask);
void resetsupvis(unsigned long vismask);
int nextsupvis(XVisualInfo **vi);
int nsupvis(void);
Status checkarea(Display *disp, Drawable d, register struct area *ap, unsigned long  inpix, unsigned long  outpix, int  flags);
Status checkclear(Display *disp, Drawable d);
void getsize(Display *disp, Drawable d, unsigned int  *widthp, unsigned int  *heightp);
unsigned int getdepth(Display *disp, Drawable d);
void pattern(Display *disp, Drawable d);
Status checkpattern(Display *disp, Drawable d, struct area *ap);
Window crechild(Display *disp, Window w, struct area *ap);
Window creunmapchild(Display *disp, Window w, struct area *ap);
Status checktile(Display *disp, Drawable d, struct area *ap, int  xorig, int  yorig, Pixmap pm);
Pixmap maketile(Display *disp, Drawable d);
Pixmap makepixm(Display *disp, XVisualInfo *vp);
void resetvinf(int  flags);
int nextvinf(XVisualInfo  **visp);
int nvinf(void);
Drawable defdraw(Display *disp, int  type);
Window defwin(Display *disp);
GC makegc(Display *disp, Drawable d);
void setgcfont(Font font);
void dumpimage(XImage *imp, char *name, struct area *ap);
Status verifyimage(Display  *disp, Drawable d, struct area *ap);
void dclear(Display *disp, Drawable d);
void dset(Display *disp, Drawable d, unsigned long pix);
void delete(char *mess, ... );
int isdeleted(void);
void resetdelete(void);
void cancelrest(char *reason);
void unsupported(char *mess, ... );
void notinuse(char *mess, ... );
void untested(char *mess, ... );
XImage *savimage(Display *disp, Drawable d);
Status compsavimage(Display *disp, Drawable d, XImage *im);
Status diffsavimage(Display *disp, Drawable d, XImage *im);
int checkpixel(Display *display, Drawable drawable, int x, int y, unsigned long value);
unsigned long getpixel(Display *display, Drawable drawable, int  x, int  y);
Status checkpixels(Display *display, Drawable drawable, int x, int y, int dx, int dy, int len, unsigned long value);
int notmember(int  *list, int  len, long *ret);
int notmaskmember(unsigned long *list, int  len, unsigned long *ret);
void setwidth(Display *disp, GC  gc, unsigned int  width);
void setcapstyle(Display *disp, GC  gc, int  capstyle);
void setlinestyle(Display *disp, GC  gc, int  linestyle);
void setjoinstyle(Display *disp, GC  gc, int  joinstyle);
int checkgccomponent(Display *disp, GC gc, unsigned int valuemask, XGCValues *values);
void gcflush(Display *dpy, GC gc);
void tpstartup(void);
void tpcleanup(void);
Pixmap nondepth1pixmap( Display *disp,  Drawable d);
void setfuncpixel(Display *disp, Drawable drw, int  *xp, int  *yp);
void regid(Display *disp, union regtypes *id, int  type);
void freereg(void);
int regenable(void);
int regdisable(void);
void openfonts(Font fonts[], int  nfonts);
int checkfsp(XFontStruct *fsp, XFontStruct *good, char *str);
void txtextents(XFontStruct *fsp, unsigned char *str, int  n, int  *dir, int  *ascent, int  *descent, XCharStruct *overall);
void txtextents16(XFontStruct *fsp, XChar2b *str, int  n, int  *dir, int  *ascent, int  *descent, XCharStruct *overall);
int txtwidth(XFontStruct *fsp, unsigned char *str, int  n);
int txtwidth16(XFontStruct *fsp, XChar2b *str, int  n);
Colormap badcolormap( Display *disp,  Window  win);
Colormap makecolmap(Display *disp, Visual *visual, int alloc);
int lg( unsigned long x);
char *bitstr( unsigned long val,  char spare[]);
Cursor makecurnum(Display *disp, int inc);
Cursor makecur(Display *disp);
Cursor makecur2(Display *disp);
XImage *makeimg(Display *disp, XVisualInfo *vp, int format);
void dsetimg(XImage *ximage, unsigned long pixel);
void patternimg(XImage *ximage, unsigned long pixel);
Status checkimg(XImage *im, struct area *ap, long inpix, long outpix, int flags);
Status checkimgstruct(XImage *im, unsigned int depth, unsigned int width, unsigned int height, int format);
Display *opendisplay(void);
unsigned int bitcount( register unsigned long n);
unsigned long getpix(unsigned long m, unsigned long p);
int atov(char *str);
void initconfig(void);
Window mkwinchild(Display *disp, XVisualInfo *vp, struct area *ap, int  mapflag, Window  parent, int  border_width);
Window mkwin(Display *disp, XVisualInfo *vp, struct area *ap, int  mapflag);
Status block(Display *display, XEvent *event, Block_Info *info);
Winh *winh_adopt(Display *display, Winh *parent, unsigned long valuemask, XSetWindowAttributes *attributes, Winhg *winhg, long winhmask);
int winh_create(Display *display, Winh *winh, long winhmask);
Winh *winh_find(Winh *winh, Window window);
int winh_plant(Winh *source, XEvent *event, long event_mask, long winhmask);
int winh_pending(int expected);
int winh_harvest(Display *display, Winh *winh);
int winh_weed(Winh *winh, int event_type, long winhmask);
int winh_ignore_event(Winh *winh, int event_type, long winhmask);
int winh_selectinput(Display *display, Winh *winh, long event_mask);
int winh_changewindowattributes(Display *display, Winh *winh, unsigned long valuemask, XSetWindowAttributes *attributes);
void winh_free(Winh *winh);
int winh_walk(Winh *winh, int depthfirst, int (*procedure)());
int winh_climb(Winh *start, Winh *stop, int (*procedure)());
int winh_eventindex(int event_type);
int winh_ordercheck(int before, int after);
int winh(Display *display, int depth, long winhmask);
PointerPlace *warppointer(Display *display, Window dest_w, int dest_x, int dest_y);
Bool pointermoved(Display *display, PointerPlace *ptr);
void unwarppointer(Display *display, PointerPlace *ptr);
void pointerrootpos(Display *display, PointerPlace *ptr);
void rootcoordset(Display *display, Window src_w, Window dest_w, int src_x, int src_y, int *dest_x_return, int *dest_y_return);
void serialset(Display *display, XEvent *event);
Bool serialtest(XEvent *good, XEvent *ev);
Time gettime( Display *disp);
int maxsize( XVisualInfo *vp);
char *xt_strdup(char *str);
struct	buildtree *buildtree(Display *disp, Window parent, char **list, int  nlist);
struct	buildtree *btntobtp(struct buildtree *list, char *name);
struct	buildtree *btwtobtp(struct buildtree *list, Window w);
Window btntow(struct buildtree *list, char *name);
char *btwton(struct buildtree *list, Window w);
void setforexpose(Display *disp, Window w);
Status exposefill(Display *disp, Window w);
Status exposecheck(Display *disp, Window w);
Status expose_test_restored(Display *disp, Window w);
void settimeout(int  to);
unsigned long cleartimeout(void);
Region makeregion(void);
int samehost(XHostAddress *h1, XHostAddress *h2);
int pfcount(int  pass, int  fail);
int rptcounts(void);
Status SimulateKeyPressEvent(Display *dpy, KeyCode keycode);
Status SimulateKeyReleaseEvent(Display *dpy, KeyCode keycode);
Status SimulateButtonPressEvent(Display  *dpy, unsigned int button);
Status SimulateButtonReleaseEvent(Display  *dpy, unsigned int button);
Status CompareCursorWithWindow(Display *dpy, Window window, Cursor cursor);
Status CompareCurrentWithWindow(Display *dpy, Window window);
Status SimulateMotionEvent(Display *dpy, int screen, int x, int y);
Status XTestDiscard(    Display *dpy);
void XTestSetGContextOfGC( GC gc,  GContext gid);
void XTestSetVisualIDOfVisual( Visual *v,  VisualID vid);
void buttonpress(Display *disp, unsigned int     button);
void buttonrel(Display *disp, unsigned int     button);
void keypress(Display *disp, int     key);
void keyrel(Display *disp, int     key);
void relbuttons(void);
void relkeys(void);
void relalldev(void);
unsigned int wantmods(Display *disp, int  want);
int modpress(Display *disp, unsigned int  mask);
int modrel(Display *disp, unsigned int  mask);
int ismodkey(unsigned int  mask, int kc);
int restoredevstate(void);
int noext(int  needbutton);
int nbuttons(void);
int getkeycode(Display *display);
Status IsExtTestAvailable(void);
int xtest_putenv(char *envstr);
int curofwin(Display *display, Cursor cursor, Window window);
int defcur(Display *display, Window window);
int spriteiswin(Display *display, Window window);

#else

void startup();
void cleanup();
void fontstartup();
void setxtfontpath();
void fontcleanup();
void focusstartup();
void focuscleanup();
void fontfocusstartup();
void fontfocuscleanup();
void rmstartup();
void aborttest();
void reset_delay();
void exec_startup();
void exec_cleanup();
char *boolname();
char *eventmaskname();
char *eventname();
char *keymaskname();
char *modifiername();
char *buttonmaskname();
char *buttonname();
char *notifymodename();
char *notifydetailname();
char *visibilityname();
char *grabreplyname();
char *alloweventmodename();
char *reverttoname();
char *errorname();
char *classname();
char *bitgravityname();
char *wingravityname();
char *backingstorename();
char *gcfunctionname();
char *linestylename();
char *capstylename();
char *joinstylename();
char *fillstylename();
char *displayclassname();
char *protoname();
char *atomname();
char *mapstatename();
char *visualmaskname();
char *contexterrorname();
int error_status();
int unexp_err();
int io_err();
int geterr();
int getbadvalue();
void reseterr();
int getevent();
int stackorder();
void report();
void trace();
void check();
void debug();
void tccabort();
void setdblev();
int getdblev();
Window iponlywin();
Window badwin();
void _startcall();
void startcall();
void _endcall();
void endcall();
Window makewin();
void winpair();
Drawable makewinpos();
int checkevent();
int initvclass();
void resetvclass();
int nextvclass();
int nvclass();
GC badgc();
Pixmap badpixm();
Font badfont();
void badvis();
void linkstart();
void linkclean();
int issuppvis();
unsigned long visualsupported();
void resetsupvis();
int nextsupvis();
int nsupvis();
Status checkarea();
Status checkclear();
void getsize();
unsigned int getdepth();
void pattern();
Status checkpattern();
Window crechild();
Window creunmapchild();
Status checktile();
Pixmap maketile();
Pixmap makepixm();
void resetvinf();
int nextvinf();
int nvinf();
Drawable defdraw();
Window defwin();
GC makegc();
void setgcfont();
void dumpimage();
Status verifyimage();
void dclear();
void dset();
void delete();
int isdeleted();
void resetdelete();
void cancelrest();
void unsupported();
void notinuse();
void untested();
XImage *savimage();
Status compsavimage();
Status diffsavimage();
int checkpixel();
unsigned long getpixel();
Status checkpixels();
int notmember();
int notmaskmember();
void setwidth();
void setcapstyle();
void setlinestyle();
void setjoinstyle();
int checkgccomponent();
void gcflush();
void tpstartup();
void tpcleanup();
Pixmap nondepth1pixmap();
void setfuncpixel();
void regid();
void freereg();
int regenable();
int regdisable();
void openfonts();
int checkfsp();
void txtextents();
void txtextents16();
int txtwidth();
int txtwidth16();
Colormap badcolormap();
Colormap makecolmap();
int lg();
char *bitstr();
Cursor makecurnum();
Cursor makecur();
Cursor makecur2();
XImage *makeimg();
void dsetimg();
void patternimg();
Status checkimg();
Status checkimgstruct();
Display *opendisplay();
unsigned int bitcount();
unsigned long getpix();
int atov();
void initconfig();
Window mkwinchild();
Window mkwin();
Status block();
Winh *winh_adopt();
int winh_create();
Winh *winh_find();
int winh_plant();
int winh_pending();
int winh_harvest();
int winh_weed();
int winh_ignore_event();
int winh_selectinput();
int winh_changewindowattributes();
void winh_free();
int winh_walk();
int winh_climb();
int winh_eventindex();
int winh_ordercheck();
int winh();
PointerPlace *warppointer();
Bool pointermoved();
void unwarppointer();
void pointerrootpos();
void rootcoordset();
void serialset();
Bool serialtest();
Time gettime();
int maxsize();
char *xt_strdup();
struct	buildtree *buildtree();
struct	buildtree *btntobtp();
struct	buildtree *btwtobtp();
Window btntow();
char *btwton();
void setforexpose();
Status exposefill();
Status exposecheck();
Status expose_test_restored();
void settimeout();
unsigned long cleartimeout();
Region makeregion();
int samehost();
int pfcount();
int rptcounts();
Status SimulateKeyPressEvent();
Status SimulateKeyReleaseEvent();
Status SimulateButtonPressEvent();
Status SimulateButtonReleaseEvent();
Status CompareCursorWithWindow();
Status CompareCurrentWithWindow();
Status SimulateMotionEvent();
Status XTestDiscard();
void XTestSetGContextOfGC();
void XTestSetVisualIDOfVisual();
void buttonpress();
void buttonrel();
void keypress();
void keyrel();
void relbuttons();
void relkeys();
void relalldev();
unsigned int wantmods();
int modpress();
int modrel();
int ismodkey();
int restoredevstate();
int noext();
int nbuttons();
int getkeycode();
Status IsExtTestAvailable();
int xtest_putenv();
int curofwin();
int defcur();
int spriteiswin();

#endif

