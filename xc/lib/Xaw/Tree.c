/*
 * $XConsortium: Tree.c,v 1.6 90/02/02 11:36:44 jim Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 * Copyright 1989 Prentice Hall
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation.
 * 
 * M.I.T., Prentice Hall and the authors disclaim all warranties with regard
 * to this software, including all implied warranties of merchantability and
 * fitness.  In no event shall M.I.T., Prentice Hall or the authors be liable
 * for any special, indirect or cosequential damages or any damages whatsoever
 * resulting from loss of use, data or profits, whether in an action of
 * contract, negligence or other tortious action, arising out of or in
 * connection with the use or performance of this software.
 * 
 * Authors:  Jim Fulton, MIT X Consortium,
 *           based on a version by Douglas Young, Prentice Hall
 * 
 * This widget is based on the Tree widget described on pages 397-419 of
 * Douglas Young's book "The X Window System, Programming and Applications 
 * with Xt OSF/Motif Edition."  The layout code has been rewritten to use
 * additional blank space to make the structure of the graph easier to see
 * as well as to support vertical trees.
 */

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>
#include <X11/Xmu/Converters.h>
#include "TreeP.h"

#define MAXIMUM(a,b) ((a) > (b) ? (a) : (b))
#define IsHorizontal(tw) ((tw)->tree.orientation == XtorientHorizontal)

static void             Initialize();
static void             ConstraintInitialize();
static void             ConstraintDestroy();
static Boolean          ConstraintSetValues();
static void             Resize();
static Boolean          SetValues();
static XtGeometryResult GeometryManager();
static void             ChangeManaged();
static void             insert_new_node();
static void             delete_node();
static void             new_layout();
static void             Redisplay();
static void             set_positions();
static void             initialize_dimensions();

/*
 * resources of the tree itself
 */
static XtResource resources[] = {
    { XtNhSpace, XtCHSpace, XtRDimension, sizeof (Dimension),
	XtOffset(TreeWidget, tree.h_min_space), XtRString, "20" },
    { XtNvSpace, XtCVSpace, XtRDimension, sizeof (Dimension),
	XtOffset(TreeWidget, tree.v_min_space), XtRString, "6" },
    { XtNforeground, XtCForeground, XtRPixel, sizeof (Pixel),
	XtOffset(TreeWidget, tree.foreground), XtRString, "Black"},
    { XtNlineWidth, XtCLineWidth, XtRDimension, sizeof (Dimension),
	XtOffset(TreeWidget, tree.line_width), XtRString, "0" },
    { XtNorientation, XtCOrientation, XtROrientation, sizeof (XtOrientation),
	XtOffset(TreeWidget, tree.orientation), XtRImmediate,
	(caddr_t) XtorientHorizontal },
};


/*
 * resources that are attached to all children of the tree
 */
static XtResource treeConstraintResources[] = {
    { XtNparent, XtCParent, XtRPointer, sizeof (Widget),
	XtOffset(TreeConstraints, tree.parent), XtRPointer, NULL},
};


TreeClassRec treeClassRec = {
  {
    /* core_class fields  */
    (WidgetClass) &constraintClassRec,/* superclass         */
    "Tree",                           /* class_name         */
    sizeof(TreeRec),                  /* widget_size        */
    NULL,                             /* class_init         */
    NULL,                             /* class_part_init    */
    FALSE,                            /* class_inited       */	
    Initialize,                       /* initialize         */
    NULL,                             /* initialize_hook    */	
    XtInheritRealize,                 /* realize            */
    NULL,                             /* actions            */
    0,                                /* num_actions        */	
    resources,                        /* resources          */
    XtNumber(resources),              /* num_resources      */
    NULLQUARK,                        /* xrm_class          */
    TRUE,                             /* compress_motion    */	
    TRUE,                             /* compress_exposure  */	
    TRUE,                             /* compress_enterleave*/	
    TRUE,                             /* visible_interest   */
    NULL,                             /* destroy            */
    NULL,                             /* resize             */
    Redisplay,                        /* expose             */
    SetValues,                        /* set_values         */
    NULL,                             /* set_values_hook    */	
    XtInheritSetValuesAlmost,         /* set_values_almost  */
    NULL,                             /* get_values_hook    */	
    NULL,                             /* accept_focus       */
    XtVersion,                        /* version            */	
    NULL,                             /* callback_private   */
    NULL,                             /* tm_table           */
    NULL,                             /* query_geometry     */	
    NULL,                             /* display_accelerator*/
    NULL,                             /* extension          */
  },
  {
    /* composite_class fields */
    GeometryManager,                 /* geometry_manager    */
    ChangeManaged,                   /* change_managed      */
    XtInheritInsertChild,            /* insert_child        */	
    XtInheritDeleteChild,            /* delete_child        */	
    NULL,                            /* extension           */
  },
  { 
    /* constraint_class fields */
   treeConstraintResources,          /* subresources        */
   XtNumber(treeConstraintResources),/* subresource_count   */
   sizeof(TreeConstraintsRec),       /* constraint_size     */
   ConstraintInitialize,             /* initialize          */
   ConstraintDestroy,                /* destroy             */
   ConstraintSetValues,              /* set_values          */
   NULL,                             /* extension           */
   },
  {
    /* Tree class fields */
    0,                               /* ignore              */	
  }
};

WidgetClass treeWidgetClass = (WidgetClass) &treeClassRec;

static void Initialize(request, new)
    TreeWidget request, new;
{
  Arg       wargs[2];
  XGCValues values;
  XtGCMask  valueMask;
  /*
   * Make sure the widget's width and height are 
   * greater than zero.
   */
  if (request->core.width <= 0)
    new->core.width = 5;
  if (request->core.height <= 0)
    new->core.height = 5;
  /*
   * Create a graphics context for the connecting lines.
   */
  valueMask = GCForeground | GCBackground;
  values.foreground = new->tree.foreground;
  values.background = new->core.background_pixel;
  if (new->tree.line_width != 0) {
      valueMask |= GCLineWidth;
      values.line_width = new->tree.line_width;
  }
  new->tree.gc = XtGetGC (new, valueMask, &values);  
  /*
   * Create the hidden root widget.
   */
  new->tree.tree_root = (Widget) NULL;
  XtSetArg(wargs[0], XtNwidth, 1);
  XtSetArg(wargs[1], XtNheight, 1);
  new->tree.tree_root = 
          XtCreateWidget("root", widgetClass, new, wargs, 2);
  /*
   * Allocate the tables used by the layout
   * algorithm.
   */
  new->tree.n_largest = 0;
  initialize_dimensions (&new->tree.largest, &new->tree.n_largest, 
			 INITIAL_TREE_DEPTH);
} 

static void ConstraintInitialize(request, new)
     Widget request, new;
{
  TreeConstraints tree_const = TREE_CONSTRAINT(new);
  TreeWidget tw = (TreeWidget) new->core.parent;
  /*
   * Initialize the widget to have no sub-nodes.
   */
  tree_const->tree.n_children = 0;
  tree_const->tree.max_children = 0;
  tree_const->tree.children = (WidgetList) NULL;
  tree_const->tree.x = tree_const->tree.y = 0; 
  /*
   * If this widget has a super-node, add it to that 
   * widget' sub-nodes list. Otherwise make it a sub-node of 
   * the tree_root widget.
   */
  if(tree_const->tree.parent)
    insert_new_node(tree_const->tree.parent, new);
  else
    if(tw->tree.tree_root)
      insert_new_node(tw->tree.tree_root, new);
} 

static Boolean SetValues(current, request, new)
    TreeWidget current, request, new;
{
 int       redraw = FALSE;
 XGCValues values;
 XtGCMask  valueMask;
 /*
  * If the foreground color has changed, redo the GC's
  * and indicate a redraw.
  */
 if (new->tree.foreground != current->tree.foreground ||
     new->core.background_pixel !=
                           current->core.background_pixel){
   valueMask         = GCForeground | GCBackground;
   values.foreground = new->tree.foreground;
   values.background = new->core.background_pixel;
   XtReleaseGC(new, new->tree.gc);
   new->tree.gc    = XtGetGC (new, valueMask, &values);   
   redraw = TRUE;     
 }
 /*
  * If the minimum spacing has changed, recalculate the
  * tree layout. new_layout() does a redraw, so we don't
  * need SetValues to do another one.
  */
 if (new->tree.v_min_space != current->tree.v_min_space ||
     new->tree.h_min_space != current->tree.h_min_space){ 
   new_layout(new);
   redraw = FALSE;
 }
 return (redraw);
}

static Boolean ConstraintSetValues(current, request, new)
    Widget current, request, new;
{
 TreeConstraints newconst = TREE_CONSTRAINT(new);
 TreeConstraints current_const = TREE_CONSTRAINT(current);
 TreeWidget tw = (TreeWidget) new->core.parent;
 /*
  * If the parent field has changed, remove the widget
  * from the old widget's children list and add it to the
  * new one.
  */
 if(current_const->tree.parent !=
                                  newconst->tree.parent){
   if(current_const->tree.parent)
     delete_node(current_const->tree.parent, new);
   if(newconst->tree.parent)
     insert_new_node(newconst->tree.parent, new);
   /*
    * If the Tree widget has been realized, 
    * compute new layout.
    */
   if(XtIsRealized(tw))
     new_layout(tw);
  }               
  return (False);
}

static void insert_new_node(parent, node)
     Widget parent, node;
{
  TreeConstraints super_const = TREE_CONSTRAINT(parent);
  TreeConstraints node_const = TREE_CONSTRAINT(node);
  int nindex = super_const->tree.n_children;
  
  node_const->tree.parent = parent;
  /*
   * If there isn't more room in the children array, 
   * allocate additional space.
   */  
  if(super_const->tree.n_children ==
                             super_const->tree.max_children){
    super_const->tree.max_children += 
                    (super_const->tree.max_children / 2) + 2;
    super_const->tree.children = 
     (WidgetList) XtRealloc(super_const->tree.children, 
                           (super_const->tree.max_children) *
                            sizeof(Widget));
  } 
  /*
   * Add the sub_node in the next available slot and 
   * increment the counter.
   */
  super_const->tree.children[nindex] = node;
  super_const->tree.n_children++;
}

static void delete_node(parent, node)
    Widget  parent, node;
{
  TreeConstraints node_const = TREE_CONSTRAINT(node);
  TreeConstraints super_const;
  int             pos, i;
  /*
   * Make sure the parent exists.
   */
  if(!parent) return;  
  
  super_const = TREE_CONSTRAINT(parent);
  /*
   * Find the sub_node on its parent's list.
   */
  for (pos = 0; pos < super_const->tree.n_children; pos++)
    if (super_const->tree.children[pos] == node)
      break;
  if (pos == super_const->tree.n_children) return;
  /*
   * Decrement the number of children
   */  
  super_const->tree.n_children--;
  /*
   * Fill in the gap left by the sub_node.
   * Zero the last slot for good luck.
   */
  for (i = pos; i < super_const->tree.n_children; i++) 
    super_const->tree.children[i] = 
                            super_const->tree.children[i+1];
 super_const->tree.children[super_const->tree.n_children]=0;
}

static void ConstraintDestroy(w) 
     TreeWidget w;
{ 
  TreeConstraints tree_const = TREE_CONSTRAINT(w);
  int i;
 /* 
  * Remove the widget from its parent's sub-nodes list and
  * make all this widget's sub-nodes sub-nodes of the parent.
  */
  if(tree_const->tree.parent) { 
    delete_node(tree_const->tree.parent, w);
    for(i=0;i< tree_const->tree.n_children; i++)
      insert_new_node(tree_const->tree.parent, 
                      tree_const->tree.children[i]);
  }
  new_layout(w->core.parent);
}

static XtGeometryResult GeometryManager(w, request, reply)
    Widget               w;
    XtWidgetGeometry    *request;
    XtWidgetGeometry    *reply;
{

 TreeWidget tw = (TreeWidget) w->core.parent;
 /*
  * No position changes allowed!.
  */
 if ((request->request_mode & CWX && request->x!=w->core.x)
     ||(request->request_mode & CWY && request->y!=w->core.y))
  return (XtGeometryNo);
 /*
  * Allow all resize requests.
  */
 if (request->request_mode & CWWidth)
   w->core.width = request->width;
 if (request->request_mode & CWHeight)
   w->core.height = request->height;
 if (request->request_mode & CWBorderWidth)
   w->core.border_width = request->border_width;
 /*
  *  Compute the new layout based on the new widget sizes;
  */
 new_layout(tw);
 return (XtGeometryYes);
}

static void ChangeManaged(tw)
    TreeWidget tw;
{
  new_layout(tw);
}


static void Redisplay (w, event, region)
     TreeWidget   w;
     XEvent        *event;
     Region         region;
{
  int              i, j;
  TreeConstraints tree_const;
  Widget          child;
  /*
   * If the Tree widget is visible, visit each managed child.
   */
  if(w->core.visible)
   for (i = 0; i < w -> composite.num_children; i++){
     child = w -> composite.children[i];
     tree_const = TREE_CONSTRAINT(child);
     /*
      * Draw a line between the right edge of each widget
      * and the left edge of each of its children. Don't
      * draw lines from the fake tree_root.
      */
     if(child != w->tree.tree_root && 
        tree_const->tree.n_children)
       for (j = 0; j < tree_const->tree.n_children; j++)
         XDrawLine(XtDisplay(w), XtWindow(w), 
                   w->tree.gc,
                   child->core.x + child->core.width, 
                   child->core.y + child->core.height / 2,
                   tree_const->tree.children[j]->core.x,
                   tree_const->tree.children[j]->core.y + 
                tree_const->tree.children[j]->core.height/2);
    }
}


static void set_positions(tw, w, level)
     TreeWidget tw;
     Widget       w;
     int          level;
{
 int               i;
 Dimension         replyWidth = 0, replyHeight = 0;
 XtGeometryResult  result;
  
 if(w){
  TreeConstraints tree_const = TREE_CONSTRAINT(w);
 /*
  * Move the widget into position.
  */
  XtMoveWidget (w, tree_const->tree.x, tree_const->tree.y);
 /*
  * If the widget position plus its width or height doesn't
  * fit in the tree, ask if the tree can be resized.
  */
  if(tw->core.width < tree_const->tree.x + w->core.width ||
     tw->core.height < tree_const->tree.y + w->core.height){
    result = 
      XtMakeResizeRequest(tw, MAXIMUM(tw->core.width, 
                                  tree_const->tree.x + 
                                  w->core.width),
                              MAXIMUM(tw->core.height, 
                                  tree_const->tree.y + 
                                  w->core.height),
                          &replyWidth, &replyHeight);
    /*
     * Accept any compromise.
     */
     if (result == XtGeometryAlmost)
       XtMakeResizeRequest (tw, replyWidth, replyHeight, 
                             NULL, NULL);
  }
 /*
  * Set the positions of all children.
  */
  for(i=0; i< tree_const->tree.n_children;i++)
    set_positions(tw, tree_const->tree.children[i], level+1);
  }
}


static void initialize_dimensions (listp, sizep, n)
    Dimension **listp;
    int *sizep;
    int n;
{
    register int i;
    register Dimension *l;

    if (!*listp) {
	*listp = (Dimension *) XtCalloc (n, sizeof(Dimension));
	*sizep = ((*listp) ? n : 0);
	return;
    }
    if (n > *sizep) {
	*listp = (Dimension *) XtRealloc (*listp, n * sizeof(Dimension));
	*sizep = ((*listp) ? n : 0);
	if (!*listp) return;
    }
    for (i = *sizep, l = (*listp) + i; i < n; i++, l++) *l = 0;
    return;
}

static void compute_bounding_box_subtree (tree, w, depth)
    TreeWidget tree;
    Widget w;
    int depth;
{
    TreeConstraints tc = TREE_CONSTRAINT(w);  /* info attached to all kids */
    register int i;
    Bool horiz = IsHorizontal (tree);
    Dimension newwidth, newheight;

    /*
     * Set the max-size per level.
     */
    if (depth >= tree->tree.n_largest) {
	initialize_dimensions (&tree->tree.largest,
			       &tree->tree.n_largest, depth);
    }
    newwidth = (horiz ? w->core.width : w->core.height);
    if (tree->tree.largest[depth] < newwidth)
      tree->tree.largest[depth] = newwidth;


    /*
     * initialize
     */
    tc->tree.bbwidth = w->core.width;
    tc->tree.bbheight = w->core.height;

    if (tc->tree.n_children == 0) return;

    /*
     * Figure the size of the opposite dimension (vertical if tree is 
     * horizontal, else vice versa).  The other dimension will be set 
     * in the second pass once we know the maximum dimensions.
     */
    newwidth = 0;
    newheight = 0;
    for (i = 0; i < tc->tree.n_children; i++) {
	Widget child = tc->tree.children[i];
	TreeConstraints cc = TREE_CONSTRAINT(child);
	    
	compute_bounding_box_subtree (tree, child, depth + 1);

	if (horiz) {
	    if (newwidth < cc->tree.bbwidth) newwidth = cc->tree.bbwidth;
	    newheight += tree->tree.v_min_space + cc->tree.bbheight;
	} else {
	    if (newheight < cc->tree.bbheight) newheight = cc->tree.bbheight;
	    newwidth += tree->tree.h_min_space + cc->tree.bbwidth;
	}
    }


    /*
     * Add a little bit of padding to make parallel subtrees stand out a
     * little bit.  Make sure to do similar adjustments in second pass.
     */
    if (tc->tree.n_children > 1) {
	if (horiz) {
	    newheight += tree->tree.v_min_space;
	} else {
	    newwidth += tree->tree.h_min_space;
	}
    }


    /*
     * Now fit parent onto side (or top) of bounding box and correct for
     * extra padding.  Be careful of unsigned arithmetic.
     */
    if (horiz) {
	tc->tree.bbwidth += tree->tree.h_min_space + newwidth;
	newheight -= tree->tree.v_min_space;
	if (newheight > tc->tree.bbheight) tc->tree.bbheight = newheight;
    } else {
	tc->tree.bbheight += tree->tree.v_min_space + newheight;
	newwidth -= tree->tree.h_min_space;
	if (newwidth > tc->tree.bbwidth) tc->tree.bbwidth = newwidth;
    }
}


static void arrange_subtree (tree, w, depth, x, y)
    TreeWidget tree;
    Widget w;
    int depth;
    Position x, y;
{
    TreeConstraints tc = TREE_CONSTRAINT(w);  /* info attached to all kids */
    TreeConstraints firstcc, lastcc;
    register int i;
    int newx, newy;
    Bool horiz = IsHorizontal (tree);

    if (tc->tree.n_children > 1) {
	if (horiz) {
	    y += tree->tree.v_min_space / 2;
	} else {
	    x += tree->tree.h_min_space / 2;
	}
    }

    tc->tree.x = x;
    tc->tree.y = y;
    /*
     * If no children, then just lay out where requested.
     */
    if (tc->tree.n_children == 0) return;


    /*
     * Have children, so walk down tree laying out children, then laying
     * out parents.
     */
    if (horiz) {
	newx = x + tree->tree.largest[depth];
	if (depth > 0) newx += tree->tree.h_min_space;
	newy = y;
    } else {
	newx = x;
	newy = y + tree->tree.largest[depth];
	if (depth > 0) newy += tree->tree.v_min_space;
    }

    for (i = 0; i < tc->tree.n_children; i++) {
	Widget child = tc->tree.children[i];
	TreeConstraints cc = TREE_CONSTRAINT(child);

	arrange_subtree (tree, child, depth + 1, newx, newy);
	if (horiz) {
	    newy += tree->tree.v_min_space + cc->tree.bbheight;
	} else {
	    newx += tree->tree.h_min_space + cc->tree.bbwidth;
	}
    }

    /*
     * now layout parent between first and last children
     */
    firstcc = TREE_CONSTRAINT (tc->tree.children[0]);
    lastcc = TREE_CONSTRAINT (tc->tree.children[tc->tree.n_children-1]);
	
    if (horiz) {
	tc->tree.x = x;
	tc->tree.y = (firstcc->tree.y +
		      ((lastcc->tree.y - firstcc->tree.y) / 2));
    } else {
	tc->tree.x = (firstcc->tree.x +
		      ((lastcc->tree.x - firstcc->tree.x) / 2));
	tc->tree.y = y;
    }
}

static void new_layout (tw)
    TreeWidget tw;
{
    /*
     * Do a depth-first search computing the width and height of the bounding
     * box for the tree at that position (and below).  Then, walk again using
     * this information to layout the children at each level.
     */

    initialize_dimensions (&tw->tree.largest, &tw->tree.n_largest, 
			   tw->tree.n_largest);
    compute_bounding_box_subtree (tw, tw->tree.tree_root, 0);

   /*
    * Second pass to do final layout.  Each child's bounding box is stacked
    * on top of (if horizontal, else next to) on top of its siblings.  The
    * parent is centered between the first and last children.
    */
    arrange_subtree (tw, tw->tree.tree_root, 0, 0, 0);

    /*
     * Move each widget into place.
     */
    set_positions (tw, tw->tree.tree_root, 0, 0);

    /*
     * And redisplay.
     */
    if (XtIsRealized (tw))
      XClearArea (XtDisplay(tw), XtWindow(tw), 0, 0, 0, 0, TRUE);

}

