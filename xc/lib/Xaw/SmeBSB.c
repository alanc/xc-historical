#if ( !defined(lint) && !defined(SABER) )
static char Xrcsid[] = "$XConsortium: BSBMenuEnt.c,v 1.2 89/09/29 19:03:56 kit Exp $";
#endif 

/***********************************************************
Copyright 1989 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 * BSBMenuEntry.c - Source code file for BSB Menu Entry object.
 *
 * Date:    September 26, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Xmu/Drawing.h>

#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/BSBMenuEnP.h>
#include <X11/Xaw/Cardinals.h>

#define ONE_HUNDRED 100

#define offset(field) XtOffset(BSBMenuEntryObject, bsb_entry.field)

static XtResource resources[] = {
  {XtNlabel,  XtCLabel, XtRString, sizeof(String),
     offset(label), XtRString, NULL},
  {XtNvertSpace,  XtCVertSpace, XtRInt, sizeof(int),
     offset(vert_space), XtRImmediate, (caddr_t) 25},
  {XtNleftBitmap, XtCLeftBitmap, XtRPixmap, sizeof(Pixmap),
     offset(left_bitmap), XtRImmediate, (caddr_t)None},
  {XtNrightBitmap, XtCRightBitmap, XtRPixmap, sizeof(Pixmap),
     offset(right_bitmap), XtRImmediate, (caddr_t)None},
  {XtNleftMargin,  XtCHorizontalMargins, XtRDimension, sizeof(Dimension),
     offset(left_margin), XtRImmediate, (caddr_t) 4},
  {XtNrightMargin,  XtCHorizontalMargins, XtRDimension, sizeof(Dimension),
     offset(right_margin), XtRImmediate, (caddr_t) 4},
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
     offset(foreground), XtRString, "XtDefaultForeground"},
  {XtNfont,  XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     offset(font), XtRString, "XtDefaultFont"},
};   
#undef offset

/*
 * Semi Public function definitions. 
 */

static void Redisplay(), Destroy(), Initialize(), FlipColors();
static Boolean SetValues();
static XtGeometryResult QueryGeometry();

/* 
 * Private Function Definitions.
 */

static void GetDefaultSize(), DrawBitmaps(), GetBitmapInfo();
static void CreateGCs(), DestroyGCs();
    
#define superclass (&menuEntryClassRec)
BSBMenuEntryClassRec bSBMenuEntryClassRec = {
  {
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    "BSBMenuEntry",
    /* size               */    sizeof(BSBMenuEntryRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed      */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    NULL,
    /* actions            */    NULL,
    /* num_actions        */    ZERO,
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE, 
    /* compress_exposure  */    FALSE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    NULL,
    /* expose             */    Redisplay,
    /* set_values         */    SetValues,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */    NULL,
    /* query_geometry	  */    QueryGeometry,
    /* display_accelerator*/    NULL,
    /* extension	  */    NULL
  },{
    /* Menu Entry Fields */
      
    /* highlight */             FlipColors,
    /* unhighlight */           FlipColors,
    /* notify */		XtInheritNotify,		
    /* extension	  */    NULL
  }, {
    /* BSB Menu entry Fields */  

    /* extension	  */    NULL
  }
};

WidgetClass bSBMenuEntryObjectClass = (WidgetClass) &bSBMenuEntryClassRec;

/************************************************************
 *
 * Semi-Public Functions.
 *
 ************************************************************/

/*      Function Name: Initialize
 *      Description: Initializes the simple menu widget
 *      Arguments: request - the widget requested by the argument list.
 *                 new     - the new widget with both resource and non
 *                           resource values.
 *      Returns: none.
 */

/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
    BSBMenuEntryObject entry = (BSBMenuEntryObject) new;

    if (entry->bsb_entry.label == NULL) 
	entry->bsb_entry.label = XtName(new);
    else
	entry->bsb_entry.label = XtNewString( entry->bsb_entry.label );

    GetDefaultSize(new, &(entry->rectangle.width), &(entry->rectangle.height));
    CreateGCs(new);
}

/*      Function Name: Destroy
 *      Description: Called at destroy time, cleans up.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
Destroy(w)
Widget w;
{
  DestroyGCs(w);
}

/*      Function Name: Redisplay
 *      Description: Redisplays the contents of the widget.
 *      Arguments: w - the simple menu widget.
 *                 event - the X event that caused this redisplay.
 *                 region - the region the needs to be repainted. 
 *      Returns: none.
 */

/* ARGSUSED */
static void
Redisplay(w, event, region)
Widget w;
XEvent * event;
Region region;
{
    GC gc;
    BSBMenuEntryObject entry = (BSBMenuEntryObject) w;
    int	font_ascent, font_descent, y_loc;
    
    font_ascent = entry->bsb_entry.font->max_bounds.ascent;
    font_descent = entry->bsb_entry.font->max_bounds.descent;

    y_loc = entry->rectangle.y;
    
    if (XtIsSensitive(w) && XtIsSensitive( XtParent(w) ) ) {
	if ( w == XawSimpleMenuGetActiveEntry(XtParent(w)) ) {
	    XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w), 
			   entry->bsb_entry.norm_gc, 0, y_loc,
			   (unsigned int) entry->rectangle.width,
			   (unsigned int) entry->rectangle.height);
	    gc = entry->bsb_entry.rev_gc;
	}
	else
	    gc = entry->bsb_entry.norm_gc;
    }
    else
	gc = entry->bsb_entry.norm_gray_gc;
    
    if (entry->bsb_entry.label != NULL) {
	y_loc += (entry->rectangle.height - 
		  (font_ascent + font_descent)) / 2 + font_ascent;
	
	XDrawString(XtDisplayOfObject(w), XtWindowOfObject(w), gc,
		    entry->bsb_entry.left_margin, y_loc,
		    entry->bsb_entry.label, strlen(entry->bsb_entry.label));
    }

    DrawBitmaps(w, gc);
}


/*      Function Name: SetValues
 *      Description: Relayout the menu when one of the resources is changed.
 *      Arguments: current - current state of the widget.
 *                 request - what was requested.
 *                 new - what the widget will become.
 *      Returns: none
 */

/* ARGSUSED */
static Boolean
SetValues(current, request, new)
Widget current, request, new;
{
    BSBMenuEntryObject entry = (BSBMenuEntryObject) new;
    BSBMenuEntryObject old_entry = (BSBMenuEntryObject) current;
    Boolean ret_val = FALSE;

    if (old_entry->bsb_entry.label != entry->bsb_entry.label) {
        if (old_entry->bsb_entry.label != XtName( new ) )
	    XtFree( (char *) old_entry->bsb_entry.label );

	if (entry->bsb_entry.label != XtName(new) ) 
	    entry->bsb_entry.label = XtNewString( entry->bsb_entry.label );

	ret_val = True;
    }

    if (entry->rectangle.sensitive != old_entry->rectangle.sensitive )
	ret_val = TRUE;

    if (entry->bsb_entry.left_bitmap != old_entry->bsb_entry.left_bitmap) {
	GetBitmapInfo(new, TRUE);
	ret_val = TRUE;
    }

    if (entry->bsb_entry.right_bitmap != old_entry->bsb_entry.right_bitmap) {
	GetBitmapInfo(new, FALSE);
	ret_val = TRUE;
    }

    if ( (old_entry->bsb_entry.font != entry->bsb_entry.font) ||
	 (old_entry->bsb_entry.foreground != entry->bsb_entry.foreground) ) {
	DestroyGCs(current);
	CreateGCs(new);
	ret_val = TRUE;
    }

    if (ret_val && XtIsRealized(new) ) {
	Dimension width, height;
	
	GetDefaultSize(new, &width, &height);
	switch (XtMakeResizeRequest(new, width, height, &width, &height)) {

	case XtGeometryAlmost:	/* Fall through. */
	    (void) XtMakeResizeRequest(new, width, height, &width, &height);
	case XtGeometryYes:	/* Fall through. */
	    XClearArea(XtDisplayOfObject(new), XtWindowOfObject(new),
		       (int) entry->rectangle.x, (int) entry->rectangle.y,
		       (unsigned int) entry->rectangle.width,
		       (unsigned int) entry->rectangle.height, FALSE);
	    Redisplay(new, (XEvent *) NULL, (Region) NULL);
	case XtGeometryNo:	/* Fall through. */
	default:
	    break;
	}
    }
    return(FALSE);
}

/*	Function Name: QueryGeometry.
 *	Description: Returns the preferred geometry for this widget.
 *	Arguments: w - the menu entry object.
 *                 itended, return_val - the intended and return geometry info.
 *	Returns: A Geometry Result.
 *
 * See the Intrinsics manual for details on what this function is for.
 * 
 * I just return the height and width of the label plus the margins.
 */

static XtGeometryResult
QueryGeometry(w, intended, return_val) 
Widget w;
XtWidgetGeometry *intended, *return_val;
{
    BSBMenuEntryObject entry = (BSBMenuEntryObject) w;
    Dimension width, height;
    XtGeometryResult ret_val = XtGeometryYes;
    XtGeometryMask mode = intended->request_mode;

    GetDefaultSize(w, &width, &height );    

    if ( ((mode & CWWidth) && (intended->width != width)) ||
	 !(mode & CWWidth) ) {
	return_val->request_mode |= CWWidth;
	return_val->width = width;
	ret_val = XtGeometryAlmost;
    }

    if ( ((mode & CWHeight) && (intended->height != height)) ||
	 !(mode & CWHeight) ) {
	return_val->request_mode |= CWHeight;
	return_val->height = height;
	ret_val = XtGeometryAlmost;
    }

    if (ret_val == XtGeometryAlmost) {
	mode = return_val->request_mode;
	
	if ( ((mode & CWWidth) && (width == entry->rectangle.width)) &&
	     ((mode & CWHeight) && (height == entry->rectangle.height)) )
	    return(XtGeometryNo);
    }

    return(ret_val);
}
    
/*      Function Name: FlipColors
 *      Description: Invert the colors of the current entry.
 *      Arguments: w - the bsb menu entry widget.
 *      Returns: none.
 */

static void 
FlipColors(w)
Widget w;
{
    BSBMenuEntryObject entry = (BSBMenuEntryObject) w;

    XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w),
		   entry->bsb_entry.invert_gc, 0, (int) entry->rectangle.y,
		   (unsigned int) entry->rectangle.width, 
		   (unsigned int) entry->rectangle.height);
}

/************************************************************
 *
 * Private Functions.
 *
 ************************************************************/

/*	Function Name: GetDefaultSize
 *	Description: Calculates the Default (preferred) size of
 *                   this menu entry.
 *	Arguments: w - the menu entry widget.
 *                 width, height - default sizes (RETURNED).
 *	Returns: none.
 */

static void
GetDefaultSize(w, width, height) 
Widget w;
Dimension * width, * height;
{
    BSBMenuEntryObject entry = (BSBMenuEntryObject) w;

    if (entry->bsb_entry.label == NULL) 
	*width = 0;
    else
	*width = XTextWidth(entry->bsb_entry.font, entry->bsb_entry.label,
			    strlen(entry->bsb_entry.label));

    *width += entry->bsb_entry.left_margin + entry->bsb_entry.right_margin;
    
    *height = (entry->bsb_entry.font->max_bounds.ascent +
	       entry->bsb_entry.font->max_bounds.descent);

    *height = (*height * ( ONE_HUNDRED + 
			  entry->bsb_entry.vert_space )) / ONE_HUNDRED;
}

/*      Function Name: DrawBitmaps
 *      Description: Draws left and right bitmaps.
 *      Arguments: w - the simple menu widget.
 *                 gc - graphics context to use for drawing.
 *      Returns: none
 */

static void
DrawBitmaps(w, gc)
Widget w;
GC gc;
{
    int x_loc, y_loc;
    BSBMenuEntryObject entry = (BSBMenuEntryObject) w;
    
    if ( (entry->bsb_entry.left_bitmap == None) && 
	 (entry->bsb_entry.right_bitmap == None) ) return;

    y_loc = entry->rectangle.y + (entry->rectangle.height -
				  entry->bsb_entry.left_bitmap_height) / 2;

/*
 * Draw Left Bitmap.
 */

  if (entry->bsb_entry.left_bitmap != None) {
    x_loc = (entry->bsb_entry.left_margin - 
	     entry->bsb_entry.left_bitmap_width) / 2;
    XCopyPlane(XtDisplayOfObject(w), entry->bsb_entry.left_bitmap,
	       XtWindowOfObject(w), gc, 0, 0, 
	       entry->bsb_entry.left_bitmap_width,
	       entry->bsb_entry.left_bitmap_height, x_loc, y_loc, 1);
  }

/*
 * Draw Right Bitmap.
 */

  if (entry->bsb_entry.right_bitmap != None) {
    x_loc = entry->rectangle.width - (entry->bsb_entry.right_margin - 
				      entry->bsb_entry.right_bitmap_width) / 2;
    XCopyPlane(XtDisplayOfObject(w), entry->bsb_entry.right_bitmap,
	       XtWindowOfObject(w), gc, 0, 0, 
	       entry->bsb_entry.right_bitmap_width,
	       entry->bsb_entry.right_bitmap_height, x_loc, y_loc, 1);
  }
}

/*      Function Name: GetBitmapInfo
 *      Description: Gets the bitmap information from either of the bitmaps.
 *      Arguments: w - the bsb menu entry widget.
 *                 is_left - TRUE if we are testing left bitmap,
 *                           FALSE if we are testing the right bitmap.
 *      Returns: none
 */

static void
GetBitmapInfo(w, is_left)
Widget w;
Boolean is_left;
{
    BSBMenuEntryObject entry = (BSBMenuEntryObject) w;    
    unsigned int depth, bw;
    Window root;
    int x, y;
    unsigned int width, height;
    char buf[BUFSIZ];
    
    if (is_left) {
	if (entry->bsb_entry.left_bitmap != None) {
	    if (!XGetGeometry(XtDisplayOfObject(w), 
			      entry->bsb_entry.left_bitmap, &root, 
			      &x, &y, &width, &height, &bw, &depth)) {
		sprintf(buf, "BSBMenuEntry Object: %s %s \"%s\".", "Could not",
			"get Left Bitmap geometry information for menu entry ",
			XtName(w));
		XtAppError(XtWidgetToApplicationContext(w), buf);
	    }
	    if (depth != 1) {
		sprintf(buf, "BSBMenuEntry Object: %s \"%s\"%s.", 
			"Left Bitmap of entry ", 
			XtName(w), " is not one bit deep.");
		XtAppError(XtWidgetToApplicationContext(w), buf);
	    }
	    entry->bsb_entry.left_bitmap_width = (Dimension) width; 
	    entry->bsb_entry.left_bitmap_height = (Dimension) height;
	}
    }
    else if (entry->bsb_entry.right_bitmap != None) {
	if (!XGetGeometry(XtDisplayOfObject(w),
			  entry->bsb_entry.right_bitmap, &root,
			  &x, &y, &width, &height, &bw, &depth)) {
	    sprintf(buf, "BSBMenuEntry Object: %s %s \"%s\".", "Could not",
		    "get Right Bitmap geometry information for menu entry ",
		    XtName(w));
	    XtAppError(XtWidgetToApplicationContext(w), buf);
	}
	if (depth != 1) {
	    sprintf(buf, "BSBMenuEntry Object: %s \"%s\"%s.", 
		    "Right Bitmap of entry ", XtName(w),
		    " is not one bit deep.");
	    XtAppError(XtWidgetToApplicationContext(w), buf);
	}
	entry->bsb_entry.right_bitmap_width = (Dimension) width; 
	entry->bsb_entry.right_bitmap_height = (Dimension) height;
    }
}      

/*      Function Name: CreateGCs
 *      Description: Creates all gc's for the simple menu widget.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
CreateGCs(w)
Widget w;
{
    BSBMenuEntryObject entry = (BSBMenuEntryObject) w;    
    XGCValues values;
    XtGCMask mask;
    
    values.foreground = XtParent(w)->core.background_pixel;
    values.background = entry->bsb_entry.foreground;
    values.font = entry->bsb_entry.font->fid;
    values.graphics_exposures = FALSE;
    mask        = GCForeground | GCBackground | GCFont | GCGraphicsExposures;
    entry->bsb_entry.rev_gc = XtGetGC(w, mask, &values);
    
    values.foreground = entry->bsb_entry.foreground;
    values.background = XtParent(w)->core.background_pixel;
    entry->bsb_entry.norm_gc = XtGetGC(w, mask, &values);
    
    values.fill_style = FillTiled;
    values.tile   = XmuCreateStippledPixmap(XtScreenOfObject(w), 
					    entry->bsb_entry.foreground,
					    XtParent(w)->core.background_pixel,
					    XtParent(w)->core.depth);
    values.graphics_exposures = FALSE;
    mask |= GCTile | GCFillStyle;
    entry->bsb_entry.norm_gray_gc = XtGetGC(w, mask, &values);
    
    values.foreground ^= values.background;
    values.background = 0;
    values.function = GXxor;
    mask = GCForeground | GCBackground | GCGraphicsExposures | GCFunction;
    entry->bsb_entry.invert_gc = XtGetGC(w, mask, &values);
}

/*      Function Name: DestroyGCs
 *      Description: Removes all gc's for the simple menu widget.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
DestroyGCs(w)
Widget w;
{
    BSBMenuEntryObject entry = (BSBMenuEntryObject) w;    

    XtReleaseGC(w, entry->bsb_entry.norm_gc);
    XtReleaseGC(w, entry->bsb_entry.norm_gray_gc);
    XtReleaseGC(w, entry->bsb_entry.rev_gc);
    XtReleaseGC(w, entry->bsb_entry.invert_gc);
}
