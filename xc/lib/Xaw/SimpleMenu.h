/***********************************************************
Copyright 1989 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

 "$XConsortium: SimpleMenu.h,v 1.4 89/05/11 01:06:31 kit Exp $";

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

#ifndef _SimpleMenu_h
#define _SimpleMenu_h

#include <X11/Shell.h>
#include <X11/Xmu/Xmu.h>

/****************************************************************
 *
 * SimpleMenu widget
 *
 ****************************************************************/

/*
 * SimpleMenu.h - Public Header file for SimpleMenu widget.
 *
 * This is the public header file for the Athena SimpleMenu widget.
 * It is intended to provide one pane pulldown and popup menus within
 * the framework of the X Toolkit.  As the name implies it is a first and
 * by no means complete implementation of menu code. It does not attempt fill
 * the needs all applications, but does allow a resource orientated
 * interface to menus.
 *
 * Date:    April 3, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

/* SimpleMenu Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 backgroundPixmap    BackgroundPixmap	Pixmap          None
 borderColor	     BorderColor	Pixel		XtDefaultForeground
 borderPixmap	     BorderPixmap	Pixmap		None
 borderWidth	     BorderWidth	Dimension	1
 bottomMargin        VerticalMargins    Dimension       VerticalSpace
 columnWidth         ColumnWidth        Dimension       Width of widest text
 cursor              Cursor             Cursor          None
 destroyCallback     Callback		Pointer		NULL
 font                Font               XFontStruct *   XtDefaultFont
 foreground          Foreground         Pixel           XtDefaultForeground
 height		     Height		Dimension	0
 label               Label              String          NULL (No label)
 leftMargin          HorizontalMargins  Dimension       4
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 resize              Resize             Boolean         True
 rightMargin         HorizontalMargins  Dimension       4
 rowHeight           RowHeight          Dimension       Height of Font
 sensitive	     Sensitive		Boolean		True
 topMargin           VerticalMargins    Dimension       VerticalSpace
 verticalSpace       VerticalSpace      Dimension       4
 width		     Width		Dimension	0
 x		     Position		Position	0n
 y		     Position		Position	0

*/

/* Resources Specific to each menu entry:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 callback            Callback           Callback        NULL
 label               Label              String          Name of entry
 leftBitmap          LeftBitmap         Pixmap          None
 rightBitmap         RightBitmap        Pixmap          None
 sensitive	     Sensitive		Boolean		True
 type                Type               XawMenuEntryType  Text

*/

/*
 * Valid menu types are:
 * 
 * XawMenuText                text
 * XawMenuSeparator           separator
 * XawMenuBlank               blank
 *
 * The text type is the only one that can be selected.
 * The seperator will be rendered as a one pixel wide line in the center
 * of the area.
 */

typedef enum {XawMenuText, XawMenuSeparator, 
	      XawMenuBlank, XawMenuNone} XawMenuEntryType;

#define XAW_MENU_ENTRY ("MenuEntry")

typedef struct _SimpleMenuClassRec*	SimpleMenuWidgetClass;
typedef struct _SimpleMenuRec*		SimpleMenuWidget;

extern WidgetClass simpleMenuWidgetClass;

#define XtNcursor               "cursor"
#define XtNbottomMargin         "bottomMargin"
#define XtNcolumnWidth          "columnWidth"
#define XtNlabelFont "labelFont"
#define XtNlabelSeparatorType "labelSeparatorType"
#define XtNleftBitmap           "leftBitmap"
#define XtNleftMargin		"leftMargin"
#define XtNmenuOnScreen "menuOnScreen"
#define XtNpopupOnEntry         "popupOnEntry"
#define XtNresize		"resize"
#define XtNrightBitmap          "rightBitmap"
#define XtNrightMargin          "rightMargin"
#define XtNrowHeight            "rowHeight"
#define XtNtopMargin            "topMargin"
#define XtNtype "type"
#define XtNverticalSpacePercent "verticalSpacePercent"

#define XtEblank                "blank"
#define XtEnone                 "none"
#define XtEseparator            "separator"
#define XtEtext                 "text"

#define XtCColumnWidth          "ColumnWidth"
#define XtCHorizontalMargins    "HorizontalMargins"
#define XtCLabelSeparatorType "LabelSeparatorType"
#define XtCLeftBitmap           "LeftBitmap"
#define XtCMenuOnScreen "MenuOnScreen"
#define XtCPopupOnEntry         "PopupOnEntry"
#define XtCResize		"Resize"
#define XtCRightBitmap          "RightBitmap"
#define XtCRowHeight            "RowHeight"
#define XtCType "Type"
#define XtCVerticalMargins      "VerticalMargins"
#define XtCVerticalSpacePercent "VerticalSpacePercent"

#define XtRSimpleMenuType       "SimpleMenuType"

/************************************************************
 *
 * Public Functions.
 *
 ************************************************************/
 
/*      Function Name: XawSimpleMenuAddEntryCallback
 *      Description: Adds an entry to the callback list.
 *      Arguments: w - the menu widget
 *                 name - name of new menu item.
 *                 proc - the callback proceedure.
 *                 data - the callback (client) data.
 *      Returns: none.
 */

void
XawSimpleMenuAddEntryCallback(/* w, name, proc, data */);
/*
Widget w;
char * name;
XtCallbackProc proc;
caddr_t data;
*/
  
/*      Function Name: XawSimpleMenuAddEntry
 *      Description:  Adds an item to the menu.
 *      Arguments: w - the menu widget
 *                 name - name of new menu item.
 *                 args - the argument list.
 *                 num_args -  number of arguments.
 *      Returns: none.
 */

void XawSimpleMenuAddEntry(/* w, name , args, num_args */);
/*
Widget w;
char * name;
ArgList args;
Cardinal num_args;
*/

/*      Function Name: XawSimpleMenuRemoveEntry
 *      Description: removes and entry from the menu.
 *      Arguments: w - the menu widget
 *                 name - name of the menu item to remove.
 *      Returns: none.
 */

void XawSimpleMenuRemoveEntry( /* w, name */);
/*
Widget w;
char * name;
*/

/*      Function Name: XawSimpleMenuSetEntryValues
 *      Description: Sets the values for an entry's resources.
 *      Arguments: w - the menu widget
 *                 name - name of menu item.
 *                 args - the argument list.
 *                 num_args -  number of arguments.
 *      Returns: none.
 */

void XawSimpleMenuSetEntryValues(/* w, name , args, num_args */);
/*
Widget w;
char * name;
ArgList args;
Cardinal num_args;
*/
  
/*      Function Name: XawSimpleMenuGetEntryValues
 *      Description: Gets the current values for an entry.
 *      Arguments: w - the menu widget
 *                 name - name of menu item.
 *                 args - the argument list. (name/address pairs)
 *                 num_args -  number of arguments.
 *      Returns: none.
 */

void XawSimpleMenuGetEntryValues(/* w, name , args, num_args */);
/*
Widget w;
char * name;
ArgList args;
Cardinal num_args;
*/

#endif  _SimpleMenu_h
