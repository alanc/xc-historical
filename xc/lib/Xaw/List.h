/*  This is the List widget, it is useful to display a list, without the
 *  overhead of having a widget for each item in the list.  It allows 
 *  the user to select an item in a list and notifies the application through
 *  a callback function.
 *
 *	Created: 	8/13/88
 *	By:		Chris D. Peterson
 *                      MIT - Project Athena
 *
 *      $Source: /mit/vtk/List/RCS/List.h,v $
 *      $Author: kit $
 *      $Header: List.h,v 1.4 88/08/30 16:36:53 kit Exp $
 *	
 *
 */

/***********************************************************
Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _XtList_h
#define _XtList_h

/***********************************************************************
 *
 * List Widget
 *
 *
 ***********************************************************************/

#include <X11/Simple.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 foreground	     Foreground		pixel		Black
 background	     Background		pixel		White
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		1
 destroyCallback     Callback		Pointer		NULL 
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 x		     Position		int		0
 y		     Position		int		0
 height		     Height		int		0          **1
 width		     Width		int		0          **1

 cursor		     Cursor		Cursor		None
 insensitiveBorder   Insensitive	Pixmap		Gray

 font		     Font		FontStruct	fixed
 list                List               String *        NULL       **2
 longest             Longest            int             0          **3  **4
 numberStrings       NumberStrings      int             0          **4
 pasterBuffer        Boolean            Boolean         False
 defaultColumns      Columns            int             2          **5
 forceColumns        Columns            Boolean         False      **5
 verticalList        Boolean            Boolean         False
 columnSpacing       Spacing            Dimension       6
 rowSpacing          Spacing            Dimension       4
 internalHeight	     Height		int		2
 internalWidth	     Width		int		4
 callback            Callback           XtCallbackList  NULL       **6

 **1 - If the Width or Height of the list widget is zero (0) then the value
       is set to the minimum size necessay to fit the entire list.

       If both Width and Height are zero then they are adjusted to fit the
       entire list that is created width the number of default columns 
       specified in the defaultColumns resource.

 **2 - This is an array of strings the specify elements of the list.
       This resource must be specified. 
       (What good is a list widget without a list??  :-)

 **3 - Longest is the length of the widest string in pixels.

 **4 - If either of these values are zero (0) then the list widget calculates
       the correct value. 

       (This allows you to make startup faster if you already have 
        this information calculated)

       NOTE: If the numberStrings value is zero the list must 
             be NULL terminated.

 **5 - By setting the List.Columns resource you can force the application to
       have a given number of columns.	     
        
 **6 - This returns the name and index of the item selected in an 
       XtListReturnStruct that is pointed to by the client_data
       in the CallbackProc.

*/

#define XtCList                 "List"
#define XtCSpacing              "Spacing"
#define XtCColumns              "Columns"
#define XtCLongest              "Longest"
#define XtCNumberStrings        "NumberStrings"

#define XtNpasteBuffer          "pasteBuffer"
#define XtNcolumnSpacing        "columnSpacing"
#define XtNrowSpacing           "rowSpacing"
#define XtNverticalList         "verticalList"
#define XtNforceColumns         "forceColumns"
#define XtNnumberStrings        "numberStrings"
#define XtNlongest              "longest"
#define XtNdefaultColumns       "defaultColumns"
#define XtNforeground		"foreground"
#define XtNfont			"font"
#define XtNlist                 "list"
 
/* Class record constants */

extern WidgetClass listWidgetClass;

typedef struct _ListClassRec *ListWidgetClass;
typedef struct _ListRec      *ListWidget;

/* The list return structure. */

typedef struct _XtListReturnStruct {
  String string;
  int index;
} XtListReturnStruct;

/******************************************************************
 *
 * Exported Functions
 *
 *****************************************************************/

/*	Function Name: XtListChange.
 *	Description: Changes the list being used and shown.
 *	Arguments: w - the list widget.
 *                 list - the new list.
 *                 nitems - the number of items in the list.
 *                 longest - the length (in Pixels) of the longest element
 *                           in the list.
 *                 resize - if TRUE the the list widget will
 *                          try to resize itself.
 *	Returns: none.
 *      NOTE:      If nitems of longest are <= 0 then they will be caluculated.
 *                 If nitems is <= 0 then the list needs to be NULL terminated.
 */

extern void XtListChange(); /* w, list, nitems, longest, resize */

/*	Function Name: XtListUnhighlight
 *	Description: unlights the current highlighted element.
 *	Arguments: w - the widget.
 *	Returns: none.
 */

extern void XtListUnhighlight(); /* w */

/*	Function Name: XtListHighlight
 *	Description: Highlights the given item.
 *	Arguments: w - the list widget.
 *                 item - the item to hightlight.
 *	Returns: none.
 */

extern void XtListHighlight(); /* w, item */


/*	Function Name: XtListShowCurrent
 *	Description: returns the currently highlighted object.
 *	Arguments: w - the list widget.
 *	Returns: the info about the currently highlighted object.
 */

extern XtListReturnStruct * XtListShowCurrent(); /* w */

/*
 * Value returned when there are no highlighted objects. 
 */

#define XT_LIST_NONE -1	

#endif _XtList_h
/* DON'T ADD STUFF AFTER THIS #endif */
