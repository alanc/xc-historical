/*
 * We need to keep a list of open displays since the Xlib display list isn't
 * public.  We also have to per-display info in a separate block since it isn't
 * stored directly in the Display structure.
 */
typedef struct _XExtDisplayInfo {
    struct _XExtDisplayInfo *next;	/* keep a linked list */
    Display *display;			/* which display this is */
    XExtCodes *codes;			/* the extension protocol codes */
    caddr_t data;			/* extra data for extension to use */
} XExtDisplayInfo;

typedef struct _XExtensionInfo {
    XExtDisplayInfo *head;		/* start of list */
    XExtDisplayInfo *cur;		/* most recently used */
    int ndisplays;			/* number of displays */
} XExtensionInfo;

extern XExtensionInfo *XextCreateExtension();
extern void XextDestroyExtension();
extern XExtDisplayInfo *XextAddDisplay();
extern int XextRemoveDisplay();
extern XExtDisplayInfo *XextFindDisplay();

#define XextHasExtension(i) ((i) && ((i)->codes))
#define XextCheckExtension(dpy,i,name,val) \
  if (!XextHasExtension(i)) { XMissingExtension (dpy, name); return val; }


/*
 * helper macros to generate code that is common to all extensions; caller
 * should prefix it with static if extension source is in one file; this
 * could be a utility function, but have to stack 6 unused arguments for 
 * something that is called many, many times would be bad.
 */
#define XEXT_GENERATE_FIND_DISPLAY(proc,extinfo,extname,cd,w2e,e2w,nev,data) \
XExtDisplayInfo *proc (dpy) \
    register Display *dpy; \
{ \
    XExtDisplayInfo *dpyinfo; \
    if (!extinfo) { if (!(extinfo = XextCreateExtension())) return NULL; } \
    if (!(dpyinfo = XextFindDisplay (extinfo, dpy))) \
      dpyinfo = XextAddDisplay (extinfo,dpy,extname,cd,w2e,e2w,nev,data); \
    return dpyinfo; \
}

#define XEXT_GENERATE_CLOSE_DISPLAY(proc,extinfo) \
int proc (dpy, codes) \
    Display *dpy; \
    XExtCodes *codes; \
{ \
    return XextRemoveDisplay (extinfo, dpy); \
}
