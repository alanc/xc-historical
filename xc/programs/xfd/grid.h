#ifndef _FontGrid_h_
#define _FontGrid_h_

typedef struct _FontGridRec *FontGridWidget;
extern WidgetClass fontgridWidgetClass;

#define XtNcenterChars "centerChars"
#define XtCCenterChars "CenterChars"

#define XtNboxChars "boxChars"
#define XtCBoxChars "BoxChars"

#define XtNboxColor "boxColor"
#define XtCBoxColor "BoxColor"

#define XtNstartChar "startChar"
#define XtCStartChar "StartChar"

#define XtNinternalPad "internalPad"
#define XtCInternalPad "InternalPad"

#define XtNgridWidth "gridWidth"
#define XtCGridWidth "GridWidth"

typedef struct _FontGridCharRec {
    XFontStruct *	thefont;
    XChar2b		thechar;
} FontGridCharRec;

void GetFontGridCellDimensions();

#endif /* _FontGrid_h_ */
