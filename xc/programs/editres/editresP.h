
#include <X11/Xaw/EditRes.h>

#define DEBUG

#ifdef DEBUG
#  define CLIENT_TIME_OUT 60000	/* wait sixty seconds for the client. */
#else
#  define CLIENT_TIME_OUT 5000	/* wait five seconds for the client. */
#endif /* DEBUG */

#define FLASH_TIME  1000	/* Default flash time in microseconds */
#define NUM_FLASHES 3		/* Default number of flashes. */

#define NO_IDENT 0		/* an ident that will match nothing. */

#define NUM_INC 10		/* amount to increment allocators. */

extern void exit();

/*
 * Contexts to use with the X Context Manager.
 */

#define NODE_INFO ((XContext) 42)

/*
 * Error codes for X Server errors. 
 */

#define NO_ERROR 0
#define NO_WINDOW 1

typedef enum {SendWidgetTree, SetValues, FindChild, FlashWidget,
	      GetGeometry} ResCommand;

typedef enum {ClassLabel, NameLabel, IDLabel, WindowLabel} LabelTypes;
typedef enum {SelectAll, SelectNone, SelectInvert, SelectParent, 
	      SelectChildren,  SelectDescendants, SelectAncestors} SelectTypes;

typedef struct _WNode {
    char * name;
    char * class;
    unsigned long id, window;
    struct _WNode * parent;
    struct _WNode ** children;
    struct _TreeInfo * tree_info;
    Cardinal num_children, alloc_children;
    Widget widget;
} WNode;

/*
 * Information about the client we are currently working with.
 */

typedef struct _CurrentClient {
    ResIdent ident;		/* The ident number being used. */
    ResCommand command;		/* Command we are currently executing. */
    char *value;		/* Command's value. */
    XtIntervalId timeout;	/* timeout set in case he doesn't answer. */
    Window window;		/* window to communicate with. */
    Atom atom;			/* Atom used to communicate with this client.*/
} CurrentClient;

/*
 * Information about a tree we can display.
 */

typedef struct _TreeInfo {
    Widget tree_widget;		/* The Tree widget that contains all nodes */
    WNode * top_node;		/* The top node in the tree. */
    WNode ** active_nodes;	/* The currently active nodes. */
    Cardinal num_nodes, alloc_nodes; /* number of active nodes, and space */
    Widget * flash_widgets;	/* list of widgets to flash on and off. */
    Cardinal num_flash_widgets, alloc_flash_widgets; /* number of flash wids.*/
} TreeInfo;

/*
 * Information specific to a give APPLICATION screen.
 */

typedef struct _ScreenData {
    Widget set_values_popup;	/* The SetValues popup. */
    Widget res_text;		/* SetValues resource text widget. */
    Widget val_text;		/* SetValues value text widget. */
    Widget info_label;	        /* The information label. */
} ScreenData;

typedef struct _AppResources {
    Boolean debug;		/* Is debugging on? */
    int num_flashes, flash_time; /* Number and duration of flashes. */
    Pixel flash_color;		/* Color of flash window. */
} AppResources;

/*
 * Global variables. 
 */

#ifndef THIS_IS_MAIN
    extern int global_error_code;
    extern unsigned long global_serial_num;
    extern int (*global_old_error_handler)();

    extern TreeInfo *global_tree_info;
    extern CurrentClient global_client;
    extern ScreenData global_screen_data;
    extern Widget global_tree_parent;
    extern AppResources global_resources;
#endif

/*
 * Macros.
 */

#define streq(a, b)        ( strcmp((a), (b)) == 0 )
 
