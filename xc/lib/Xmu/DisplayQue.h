/* Coopyright 1989 Massachusetts Institute of Technology */

#include <X11/Xmu/CloseHook.h>

/*
 *			      Public Entry Points
 * 
 * 
 * DisplayQueue *XmuDQCreate (closefunc, data)
 *     int (*closefunc)();
 *     caddr_t data;
 * 
 *         Creates and returns a queue into which displays may be placed.  When
 *         the display is closed, the closefunc (if non-NULL) is upcalled with
 *         as follows:
 *
 *                 (*closefunc) (queue, entry)
 *
 *
 * Bool XmuDQDestroy (q, docallbacks)
 *     DisplayQueue *q;
 *     Bool docallbacks;
 * 
 *         Releases all memory for the indicated display queue.  If docallbacks
 *         is true, then the closefunc (if non-NULL) is called for each 
 *         display.
 * 
 * 
 * DisplayQueueEntry *XmuDQLookupDisplay (q, dpy)
 *     DisplayQueue *q;
 *     Display *dpy;
 *
 *         Returns the queue entry for the specified display or NULL if the
 *         display is not in the queue.
 *
 * 
 * DisplayQueueEntry *XmuDQAddDisplay (q, dpy, data)
 *     DisplayQueue *q;
 *     Display *dpy;
 *     caddr_t data;
 *
 *         Adds the indicated display to the end of the queue or NULL if it
 *         is unable to allocate memory.  The data field may be used by the
 *         caller to attach arbitrary data to this display in this queue.  The
 *         caller should use XmuDQLookupDisplay to make sure that the display
 *         hasn't already been added.
 * 
 * 
 * Bool XmuDQRemoveDisplay (q, dpy)
 *     DisplayQueue *q;
 *     Display *dpy;
 *
 *         Removes the specified display from the given queue.  If the 
 *         indicated display is not found on this queue, False is returned,
 *         otherwise True is returned.
 */

typedef struct _DisplayQueueEntry {
    struct _DisplayQueueEntry *prev, *next;
    Display *display;
    CloseHook closehook;
    caddr_t data;
} DisplayQueueEntry;

typedef struct _DisplayQueue {
    int nentries;
    DisplayQueueEntry *head, *tail;
    int (*closefunc)();
    caddr_t data;
} DisplayQueue;


extern DisplayQueue *XmuDQCreate ();
extern Bool XmuDQDestroy ();
extern DisplayQueueEntry *XmuDQLookupDisplay ();
extern DisplayQueueEntry *XmuDQAddDisplay ();
extern Bool XmuDQRemoveDisplay ();


#define XmuDQNDisplays(q) ((q)->nentries)
