/* Copyright 1987, Massachusetts Institute of Technology */

/*
 * xwininfo.c	- MIT Project Athena, X Window system window
 *		  information utility.
 *
 *	This program will report all relavent information
 *	about a specific window.
 *
 *  Author:	Mark Lillibridge, MIT Project Athena
 *		16-Jun-87
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <strings.h>

/* Include routines to handle parsing defaults */
#include "jdisplay.h"
#include "ostuff.h"

#define TRUE 1
#define FALSE 0

#define FAILURE 0

Window window;
extern char *window_id_format;

/*
 * Report the syntax for calling xwininfo:
 */
usage()
{
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: %s [-help] [-id <id>] [-int] [-root] ",
	    program_name);
    fprintf(stderr, "[host:vs] [-tree] [-stats] [-bits] [-events] [-size] [-wm]\n\n");
    exit(0);
}


main(argc, argv)
     int argc;
     char **argv;
{
  register int i;
  char *id = NULL;
  int root_switch = FALSE;
  int tree, stats, bits, events, wm, size  = 0;

  INIT_NAME;

  /* Open display, handle command line arguments */
  Setup_Display_And_Screen(&argc, argv);

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-help"))
      usage();
    if (!strcmp(argv[i], "-id")) {
      if (++i >= argc) usage();
      id = argv[i];
      continue;
    }
    if (!strcmp(argv[i], "-int")) {
      window_id_format = " %d";
      continue;
    }
    if (!strcmp(argv[i], "-root")) {
      root_switch = TRUE;
      continue;
    }
    if (!strcmp(argv[i], "-tree")) {
      tree = 1;
      continue;
    }
    if (!strcmp(argv[i], "-stats")) {
      stats = 1;
      continue;
    }
    if (!strcmp(argv[i], "-bits")) {
      bits = 1;
      continue;
    }
    if (!strcmp(argv[i], "-events")) {
      events = 1;
      continue;
    }
    if (!strcmp(argv[i], "-wm")) {
      wm = 1;
      continue;
    }
    if (!strcmp(argv[i], "-size")) {
      size = 1;
      continue;
    }
    usage();
  }
    
  /* Get target window # */
  if (root_switch)
    window = RootWindow(dpy, screen);
  else {
    if (id) {
      window = 0;
      (void) sscanf(id, "0x%x", &window);
      if (!window) {
	(void) sscanf(id, "%d", &window);
	if (!window)
	  Fatal_Error("Invalid window id format.");
      }
    }
    else {
	    printf("\nxwininfo ==> Please select the window you wish\n");
	    printf("         ==> information on by clicking the\n");
	    printf("         ==> mouse in that window.\n");

	    window= Select_Window(dpy);
    }
  }
    

  /*
   * Do the actual displaying as per parameters
   */
  if (!(tree || bits || events || wm || size))
    stats = 1;

  printf("\nxwininfo ==> Window id:");
  Display_Window_Id(window);
  if (tree)
    Display_Tree_Info(window);
  if (stats)
    Display_Stats_Info(window);
  if (bits)
    Display_Bits_Info(window);
  if (events)
    Display_Events_Info(window);
  if (wm)
    Display_WM_Info(window);
  if (size)
    Display_Size_Hints(window);
  printf("\n");
}


/*
 * Routine to display a window id in dec/hex with name if window has one
 */

static char *window_id_format = " 0x%x";

Display_Window_Id(window)
     Window window;
{
  char *win_name;

  printf(window_id_format, window);         /* print id # in hex/dec */
  if (!window) {
	  printf(" (none)\n");
	  return;
  }
  if (!XFetchName(dpy, window, &win_name)) { /* Get window name if any */
	  printf(" (XFetchName failed!)\n");
	  return;
  }
  if (win_name) {
    printf(" (%s)\n", win_name);
    free(win_name);
  } else if (window == RootWindow(dpy, screen))
    printf(" (the root window)\n");
  else
    printf(" (has no name)\n");
}


/*
 * Display Stats on window
 */
Display_Stats_Info(window)
     Window window;
{
  XWindowAttributes win_attributes;
  char buffer[100];

  if (!XGetWindowAttributes(dpy, window, &win_attributes))
    Fatal_Error("Can't get window attributes.");

  printf("\n         ==> Upper left X: %d\n", win_attributes.x);
  printf("         ==> Upper left Y: %d\n", win_attributes.y);
  printf("         ==> Width: %d\n", win_attributes.width);
  printf("         ==> Height: %d\n", win_attributes.height);
  printf("         ==> Depth: %d\n", win_attributes.depth);
  printf("         ==> Border width: %d\n", win_attributes.border_width);

  sprintf(buffer, "unknown (code = %d)", win_attributes.class);
  if (win_attributes.class == InputOutput)
    strcpy(buffer, "InputOutput");
  if (win_attributes.class == InputOnly)
    strcpy(buffer, "InputOnly");
  printf("         ==> Window class: %s\n", buffer);

  sprintf(buffer, "unknown (code = %d)", win_attributes.map_state);
  if (win_attributes.map_state == IsUnmapped)
    strcpy(buffer, "IsUnmapped");
  if (win_attributes.map_state == IsUnviewable)
    strcpy(buffer, "IsUnviewable");
  if (win_attributes.map_state == IsViewable)
    strcpy(buffer, "IsViewable");
  printf("         ==> Window Map State: %s\n", buffer);
}


/*
 * Routine to deterimine what name of gravity code is
 */

static char *gravity_name[] = { "ForgetGravity", "NorthWestGravity",
			       "NorthGravity", "NorthEastGravity",
			       "WestGravity", "CenterGravity",
			       "EastGravity", "SouthWestGravity",
			       "SouthGravity", "SouthEastGravity",
			       "StaticGravity" };

Get_Gravity(buffer, gravity)
     char *buffer;
{
  sprintf(buffer, "unknown (code = %d)", gravity);
  if (gravity>=0 && gravity < 11)
    strcpy(buffer, gravity_name[gravity]);
}


/*
 * Display bits info
 */
Display_Bits_Info(window)
     Window window;
{
  XWindowAttributes win_attributes;
  char buffer[100];

  if (!XGetWindowAttributes(dpy, window, &win_attributes))
    Fatal_Error("Can't get window attributes.");

  Get_Gravity(buffer, win_attributes.bit_gravity);
  printf("\n         ==> Bit gravity: %s\n", buffer);

  Get_Gravity(buffer, win_attributes.win_gravity);
  if(!win_attributes.win_gravity)
    strcpy(buffer, "UnmapGravity");
  printf("         ==> Window gravity: %s\n", buffer);

  sprintf(buffer, "unknown (code = %d)", win_attributes.backing_store);
  if (win_attributes.backing_store == NotUseful)
    strcpy(buffer, "NotUseful");
  if (win_attributes.backing_store == WhenMapped)
    strcpy(buffer, "WhenMapped");
  if (win_attributes.backing_store == Always)
    strcpy(buffer, "Always");
  printf("         ==> Backing-store hint: %s\n", buffer);

  printf("         ==> Backing-planes to be preserved: 0x%x\n",
	 win_attributes.backing_planes);
  printf("         ==> Backing pixel: %d\n", win_attributes.backing_pixel);

  strcpy(buffer, "yes");
  if (!win_attributes.save_under)
    strcpy(buffer, "no");
  printf("         ==> Save-under?: %s\n", buffer);
}


/*
 * Routine to display all events in an event mask
 */

static char *event_names[] = { "KeyPress", "KeyRelease", "ButtonPress",
			      "ButtonRelease", "EnterWindow",
			      "LeaveWindow", "PointerMotion",
			      "PointerMotionHint", "Button1Motion",
			      "Button2Motion", "Button3Motion",
			      "Button4Motion", "Button5Motion",
			      "ButtonMotion", "KeymapState",
			      "Exposure", "VisibilityChange",
			      "StructureNotify", "ResizeRedirect",
			      "SubstructureNotify", "FocusChange",
			      "PropertyChange", "ColormapChange",
			      "OwnerGrabButton" };
Display_Event_Mask(mask)
     long mask;
{
  long bit;
  long bit_mask;
  char buffer[100];

  for (bit=0, bit_mask=1; bit<sizeof(long)*8; bit++, bit_mask <<= 1)
    if (mask & bit_mask) {
	    sprintf(buffer, "unknown event mask bit (bit # = %ld)", bit);
	    if (bit<24)
	      strcpy(buffer, event_names[bit]);
	    printf("             ==> %s\n", buffer);
    }
}


/*
 * Display info on events
 */
Display_Events_Info(window)
     Window window;
{
  XWindowAttributes win_attributes;
  char buffer[100];

  if (!XGetWindowAttributes(dpy, window, &win_attributes))
    Fatal_Error("Can't get window attributes.");

  printf("\n         ==> Someone wants these events:\n");
  Display_Event_Mask(win_attributes.all_event_masks);

  printf("         ==> Do not prograte these events:\n");
  Display_Event_Mask(win_attributes.do_not_propagate_mask);

  strcpy(buffer, "yes");
  if (!win_attributes.override_redirect)
    strcpy(buffer, "no");
  printf("         ==> Overide redirection?: %s\n", buffer);
}


  /* left out visual stuff */
  /* left out colormap */
  /* left out map_installed */


/*
 * Display root, parent, and children window id's of window
 */
Display_Tree_Info(window)
     Window window;
{
  int i;
  Window root_win, parent_win;
  int num_children;
  Window *child_list;
  
  if (!XQueryTree(dpy, window, &root_win, &parent_win, &child_list,
		  &num_children))
    Fatal_Error("Can't query window tree.");

  printf("\n         ==> Root window id:");
  Display_Window_Id(root_win);
  printf("         ==> Parent window id:");
  Display_Window_Id(parent_win);

  printf("         ==> Number of children: %d\n", num_children);

  for (i = num_children - 1; i >= 0; i--) {
    printf("             ==> Child window id:"); 
    Display_Window_Id(child_list[i]);
  }

  free(child_list);
}


/*
 * Display a set of size hints
 */
Display_Hints(hints)
     XSizeHints hints;
{
	long flags;

	flags = hints.flags;
	
	if (flags & USPosition)
	  printf("             ==> User supplied location: %d, %d\n",
		 hints.x, hints.y);

	if (flags & PPosition)
	  printf("             ==> Program supplied location: %d, %d\n",
		 hints.x, hints.y);

	if (flags & USSize)
	  printf("             ==> User supplied size: %d by %d\n",
		 hints.width, hints.height);

	if (flags & PSize)
	  printf("             ==> Program supplied size: %d by %d\n",
		 hints.width, hints.height);

	if (flags & PMinSize)
	  printf("             ==> Program supplied minimum size: %d by %d\n",
		 hints.min_width, hints.min_height);

	if (flags & PMaxSize)
	  printf("             ==> Program supplied maximum size: %d by %d\n",
		 hints.max_width, hints.max_height);

	if (flags & PResizeInc) {
	  printf("             ==> Program supplied x resize increment: %d\n",
		 hints.width_inc);
	  printf("             ==> Program supplied y resize increment: %d\n",
		 hints.height_inc);
        }

	if (flags & PAspect) {
	  printf("             ==> Program supplied min aspect ratio: %d/%d\n",
		 hints.min_aspect.x, hints.min_aspect.y);
	  printf("             ==> Program supplied max aspect ratio: %d/%d\n",
		 hints.max_aspect.x, hints.max_aspect.y);
        }
}


/*
 * Display Size Hints info
 */
Display_Size_Hints(window)
     Window window;
{
	XSizeHints hints;

	if (!XGetNormalHints(dpy, window, &hints))
	  printf("\n         ==> No normal window size hints defined\n");
	else {
		printf("\n         ==> Normal window size hints:\n\n");
		Display_Hints(hints);
	}

	if (!XGetZoomHints(dpy, window, &hints))
	  printf("\n         ==> No zoom window size hints defined\n");
	else {
		printf("\n         ==> Zoom window size hints:\n\n");
		Display_Hints(hints);
	}

/*
  int w0, h0, w_inc, h_inc;

  XGetResizeHint(dpy, window, &w0, &h0, &w_inc, &h_inc);

  printf("\n         ==> Resize base width: %d\n", w0);
  printf("         ==> Resize base height: %d\n", h0);
  printf("         ==> Resize width increment: %d\n", w_inc);
  printf("         ==> Resize height increment: %d\n", h_inc);
*/

}


/*
 * Display Window Manager Info
 */
Display_WM_Info(window)
     Window window;
{
        XWMHints *wmhints;
	long flags;
	char *bool;

	wmhints = XGetWMHints(dpy, window);
	if (!wmhints) {
		printf("\n         ==> No window manager hints defined\n");
		return;
	}
	flags = wmhints->flags;

	printf("\n         ==> Window manager hints:\n\n");

	if (flags & InputHint) {
		bool = "no";
		if (wmhints->input == True)
		  bool = "yes";
		printf("             ==> Application accepts input?  %s\n",
		       bool);
	}

	if (flags & IconWindowHint) {
		printf("             ==> Icon window id:");
		Display_Window_Id(wmhints->icon_window);
	}

	if (flags & IconPositionHint)
	  printf("             ==> Initial icon position: %d, %d\n",
		 wmhints->icon_x, wmhints->icon_y);

	if (flags & StateHint) {
		if (wmhints->initial_state == DontCareState)
		  printf("             ==> Initial state is Don't care\n");
		else if (wmhints->initial_state == NormalState)
		  printf("             ==> Initial state is Normal state\n");
		else if (wmhints->initial_state == ZoomState)
		  printf("             ==> Initial state is Zoomed state\n");
		else if (wmhints->initial_state == IconicState)
		  printf("             ==> Initial state is Iconic state\n");
		else if (wmhints->initial_state == InactiveState)
		  printf("             ==> Initial state is Inactive state\n");
	}
}


/*
 * Tempary replace for not implemented Xlib routine.
 */
char *XGetDefault(dpy, name, option_name)
Display *dpy;
char *name;
char *option_name;
{
	return(NULL);
}
