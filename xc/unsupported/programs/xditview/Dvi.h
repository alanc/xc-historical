/*
* $XConsortium: Exp $
*/

#ifndef _XtDvi_h
#define _XtDvi_h

/***********************************************************************
 *
 * Dvi Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 foreground	     Foreground		Pixel		Black
 fontMap	     FontMap		char *		...
 pageNumber	     PageNumber		int		1
*/

#define XtNfileName	"fileName"
#define XtNfontMap	"fontMap"
#define XtNpageNumber	"pageNumber"
#define XtNlastPageNumber   "lastPageNumber"

#define XtCFileName	"FileName"
#define XtCFontMap	"FontMap"
#define XtCPageNumber	"PageNumber"
#define XtCLastPageNumber   "LastPageNumber"

typedef struct _DviRec *DviWidget;  /* completely defined in DviPrivate.h */
typedef struct _DviClassRec *DviWidgetClass;    /* completely defined in DviPrivate.h */

extern WidgetClass dviWidgetClass;

#endif _XtDvi_h
/* DON'T ADD STUFF AFTER THIS #endif */
