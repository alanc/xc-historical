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

extern XExtDisplayInfo *XextInitDisplay();
extern XExtDisplayInfo *XextFindDisplay();
extern int XextCloseDisplay();

