#ifndef _FontGrid_h_
#define _FontGrid_h_

extern WidgetClass fontgridWidgetClass;

#define XtNboxCharacters "boxCharacters"
#define XtCBoxCharacters "BoxCharacters"

#define XtNboxColor "boxColor"
#define XtCBoxColor "BoxColor"

#define XtNfirstChar "firstChar"
#define XtCFirstChar "FirstChar"

#define XtNinternalPad "internalPad"
#define XtCInternalPad "InternalPad"

typedef struct _FontGridCharRec {
    XFontStruct *	thefont;
    XChar2b		thechar;
} FontGridCharRec;

#endif _FontGrid_h_
