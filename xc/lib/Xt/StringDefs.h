/*
* $XConsortium: StringDefs.h,v 1.50 91/01/04 19:11:50 converse Exp $
*/

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

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

#ifndef _XtStringDefs_h_
#define _XtStringDefs_h_

#ifndef _Xconst
#if __STDC__ || defined(__cplusplus) || defined(c_plusplus)
#define _Xconst const
#else
#define _Xconst
#endif
#endif

/* Resource names */

#ifdef XTSTRINGDEFINES
#define XtNaccelerators "accelerators"
#define XtNallowHoriz "allowHoriz"
#define XtNallowVert "allowVert"
#define XtNancestorSensitive "ancestorSensitive"
#define XtNbackground "background"
#define XtNbackgroundPixmap "backgroundPixmap"
#define XtNbitmap "bitmap"
#define XtNborderColor "borderColor"
#define XtNborder "borderColor"
#define XtNborderPixmap "borderPixmap"
#define XtNborderWidth "borderWidth"
#define XtNcallback "callback"
#define XtNchildren "children"
#define XtNcolormap "colormap"
#define XtNdepth "depth"
#define XtNdestroyCallback "destroyCallback"
#define XtNeditType "editType"
#define XtNfile "file"
#define XtNfont "font"
#define XtNfontSet "fontSet"
#define XtNforceBars "forceBars"
#define XtNforeground "foreground"
#define XtNfunction "function"
#define XtNheight "height"
#define XtNhighlight "highlight"
#define XtNhSpace "hSpace"
#define XtNindex "index"
#define XtNinitialResourcesPersistent "initialResourcesPersistent"
#define XtNinnerHeight "innerHeight"
#define XtNinnerWidth "innerWidth"
#define XtNinnerWindow "innerWindow"
#define XtNinsertPosition "insertPosition"
#define XtNinternalHeight "internalHeight"
#define XtNinternalWidth "internalWidth"
#define XtNjumpProc "jumpProc"
#define XtNjustify "justify"
#define XtNknobHeight "knobHeight"
#define XtNknobIndent "knobIndent"
#define XtNknobPixel "knobPixel"
#define XtNknobWidth "knobWidth"
#define XtNlabel "label"
#define XtNlength "length"
#define XtNlowerRight "lowerRight"
#define XtNmappedWhenManaged "mappedWhenManaged"
#define XtNmenuEntry "menuEntry"
#define XtNname "name"
#define XtNnotify "notify"
#define XtNnumChildren "numChildren"
#define XtNorientation "orientation"
#define XtNparameter "parameter"
#define XtNpixmap "pixmap"
#define XtNpopupCallback "popupCallback"
#define XtNpopdownCallback "popdownCallback"
#define XtNresize "resize"
#define XtNreverseVideo "reverseVideo"
#define XtNscreen "screen"
#define XtNscrollProc "scrollProc"
#define XtNscrollDCursor "scrollDCursor"
#define XtNscrollHCursor "scrollHCursor"
#define XtNscrollLCursor "scrollLCursor"
#define XtNscrollRCursor "scrollRCursor"
#define XtNscrollUCursor "scrollUCursor"
#define XtNscrollVCursor "scrollVCursor"
#define XtNselection "selection"
#define XtNselectionArray "selectionArray"
#define XtNsensitive "sensitive"
#define XtNshown "shown"
#define XtNspace "space"
#define XtNstring "string"
#define XtNtextOptions "textOptions"
#define XtNtextSink "textSink"
#define XtNtextSource "textSource"
#define XtNthickness "thickness"
#define XtNthumb "thumb"
#define XtNthumbProc "thumbProc"
#define XtNtop "top"
#define XtNtranslations "translations"
#define XtNunrealizeCallback "unrealizeCallback"
#define XtNupdate "update"
#define XtNuseBottom "useBottom"
#define XtNuseRight "useRight"
#define XtNvalue "value"
#define XtNvSpace "vSpace"
#define XtNwidth "width"
#define XtNwindow "window"
#define XtNx "x"
#define XtNy "y"
#else
extern char XtNaccelerators[];
extern char XtNallowHoriz[];
extern char XtNallowVert[];
extern char XtNancestorSensitive[];
extern char XtNbackground[];
extern char XtNbackgroundPixmap[];
extern char XtNbitmap[];
extern char XtNborderColor[];
extern char XtNborder[];
extern char XtNborderPixmap[];
extern char XtNborderWidth[];
extern char XtNcallback[];
extern char XtNchildren[];
extern char XtNcolormap[];
extern char XtNdepth[];
extern char XtNdestroyCallback[];
extern char XtNeditType[];
extern char XtNfile[];
extern char XtNfont[];
extern char XtNfontSet[];
extern char XtNforceBars[];
extern char XtNforeground[];
extern char XtNfunction[];
extern char XtNheight[];
extern char XtNhighlight[];
extern char XtNhSpace[];
extern char XtNindex[];
extern char XtNinitialResourcesPersistent[];
extern char XtNinnerHeight[];
extern char XtNinnerWidth[];
extern char XtNinnerWindow[];
extern char XtNinsertPosition[];
extern char XtNinternalHeight[];
extern char XtNinternalWidth[];
extern char XtNjumpProc[];
extern char XtNjustify[];
extern char XtNknobHeight[];
extern char XtNknobIndent[];
extern char XtNknobPixel[];
extern char XtNknobWidth[];
extern char XtNlabel[];
extern char XtNlength[];
extern char XtNlowerRight[];
extern char XtNmappedWhenManaged[];
extern char XtNmenuEntry[];
extern char XtNname[];
extern char XtNnotify[];
extern char XtNnumChildren[];
extern char XtNorientation[];
extern char XtNparameter[];
extern char XtNpixmap[];
extern char XtNpopupCallback[];
extern char XtNpopdownCallback[];
extern char XtNresize[];
extern char XtNreverseVideo[];
extern char XtNscreen[];
extern char XtNscrollProc[];
extern char XtNscrollDCursor[];
extern char XtNscrollHCursor[];
extern char XtNscrollLCursor[];
extern char XtNscrollRCursor[];
extern char XtNscrollUCursor[];
extern char XtNscrollVCursor[];
extern char XtNselection[];
extern char XtNselectionArray[];
extern char XtNsensitive[];
extern char XtNshown[];
extern char XtNspace[];
extern char XtNstring[];
extern char XtNtextOptions[];
extern char XtNtextSink[];
extern char XtNtextSource[];
extern char XtNthickness[];
extern char XtNthumb[];
extern char XtNthumbProc[];
extern char XtNtop[];
extern char XtNtranslations[];
extern char XtNunrealizeCallback[];
extern char XtNupdate[];
extern char XtNuseBottom[];
extern char XtNuseRight[];
extern char XtNvalue[];
extern char XtNvSpace[];
extern char XtNwidth[];
extern char XtNwindow[];
extern char XtNx[];
extern char XtNy[];
#endif

/* Class types */ 

#ifdef XTSTRINGDEFINES
#define XtCAccelerators "Accelerators"
#define XtCBackground "Background"
#define XtCBitmap "Bitmap"
#define XtCBoolean "Boolean"
#define XtCBorderColor "BorderColor"
#define XtCBorderWidth "BorderWidth"
#define XtCCallback "Callback"
#define XtCColormap "Colormap"
#define XtCColor "Color"
#define XtCCursor "Cursor"
#define XtCDepth "Depth"
#define XtCEditType "EditType"
#define XtCEventBindings "EventBindings"
#define XtCFile "File"
#define XtCFont "Font"
#define XtCFontSet "FontSet"
#define XtCForeground "Foreground"
#define XtCFraction "Fraction"
#define XtCFunction "Function"
#define XtCHeight "Height"
#define XtCHSpace "HSpace"
#define XtCIndex "Index"
#define XtCInitialResourcesPersistent "InitialResourcesPersistent"
#define XtCInsertPosition "InsertPosition"
#define XtCInterval "Interval"
#define XtCJustify "Justify"
#define XtCKnobIndent "KnobIndent"
#define XtCKnobPixel "KnobPixel"
#define XtCLabel "Label"
#define XtCLength "Length"
#define XtCMappedWhenManaged "MappedWhenManaged"
#define XtCMargin "Margin"
#define XtCMenuEntry "MenuEntry"
#define XtCNotify "Notify"
#define XtCOrientation "Orientation"
#define XtCParameter "Parameter"
#define XtCPixmap "Pixmap"
#define XtCPosition "Position"
#define XtCReadOnly "ReadOnly"
#define XtCResize "Resize"
#define XtCReverseVideo "ReverseVideo"
#define XtCScreen "Screen"
#define XtCScrollProc "ScrollProc"
#define XtCScrollDCursor "ScrollDCursor"
#define XtCScrollHCursor "ScrollHCursor"
#define XtCScrollLCursor "ScrollLCursor"
#define XtCScrollRCursor "ScrollRCursor"
#define XtCScrollUCursor "ScrollUCursor"
#define XtCScrollVCursor "ScrollVCursor"
#define XtCSelection "Selection"
#define XtCSensitive "Sensitive"
#define XtCSelectionArray "SelectionArray"
#define XtCSpace "Space"
#define XtCString "String"
#define XtCTextOptions "TextOptions"
#define XtCTextPosition "TextPosition"
#define XtCTextSink "TextSink"
#define XtCTextSource "TextSource"
#define XtCThickness "Thickness"
#define XtCThumb "Thumb"
#define XtCTranslations "Translations"
#define XtCValue "Value"
#define XtCVSpace "VSpace"
#define XtCWidth "Width"
#define XtCWindow "Window"
#define XtCX "X"
#define XtCY "Y"
#else
extern char XtCAccelerators[];
extern char XtCBackground[];
extern char XtCBitmap[];
extern char XtCBoolean[];
extern char XtCBorderColor[];
extern char XtCBorderWidth[];
extern char XtCCallback[];
extern char XtCColormap[];
extern char XtCColor[];
extern char XtCCursor[];
extern char XtCDepth[];
extern char XtCEditType[];
extern char XtCEventBindings[];
extern char XtCFile[];
extern char XtCFont[];
extern char XtCFontSet[];
extern char XtCForeground[];
extern char XtCFraction[];
extern char XtCFunction[];
extern char XtCHeight[];
extern char XtCHSpace[];
extern char XtCIndex[];
extern char XtCInitialResourcesPersistent[];
extern char XtCInsertPosition[];
extern char XtCInterval[];
extern char XtCJustify[];
extern char XtCKnobIndent[];
extern char XtCKnobPixel[];
extern char XtCLabel[];
extern char XtCLength[];
extern char XtCMappedWhenManaged[];
extern char XtCMargin[];
extern char XtCMenuEntry[];
extern char XtCNotify[];
extern char XtCOrientation[];
extern char XtCParameter[];
extern char XtCPixmap[];
extern char XtCPosition[];
extern char XtCReadOnly[];
extern char XtCResize[];
extern char XtCReverseVideo[];
extern char XtCScreen[];
extern char XtCScrollProc[];
extern char XtCScrollDCursor[];
extern char XtCScrollHCursor[];
extern char XtCScrollLCursor[];
extern char XtCScrollRCursor[];
extern char XtCScrollUCursor[];
extern char XtCScrollVCursor[];
extern char XtCSelection[];
extern char XtCSensitive[];
extern char XtCSelectionArray[];
extern char XtCSpace[];
extern char XtCString[];
extern char XtCTextOptions[];
extern char XtCTextPosition[];
extern char XtCTextSink[];
extern char XtCTextSource[];
extern char XtCThickness[];
extern char XtCThumb[];
extern char XtCTranslations[];
extern char XtCValue[];
extern char XtCVSpace[];
extern char XtCWidth[];
extern char XtCWindow[];
extern char XtCX[];
extern char XtCY[];
#endif

/* Representation types */

#ifdef XTSTRINGDEFINES
#define XtRAcceleratorTable "AcceleratorTable"
#define XtRAtom "Atom"
#define XtRBitmap "Bitmap"
#define XtRBool "Bool"
#define XtRBoolean "Boolean"
#define XtRCallback "Callback"
#define XtRCallProc "CallProc"
#define XtRCardinal "Cardinal"
#define XtRColor "Color"
#define XtRColormap "Colormap"
#define XtRCursor "Cursor"
#define XtRDimension "Dimension"
#define XtRDisplay "Display"
#define XtREditMode "EditMode"
#define XtREnum "Enum"
#define XtRFile "File"
#define XtRFloat "Float"
#define XtRFont "Font"
#define XtRFontSet "FontSet"
#define XtRFontStruct "FontStruct"
#define XtRFunction "Function"
#define XtRGeometry "Geometry"
#define XtRImmediate "Immediate"
#define XtRInitialState "InitialState"
#define XtRInt "Int"
#define XtRJustify "Justify"
#define XtRLongBoolean XtRBool
#define XtRObject "Object"
#define XtROrientation "Orientation"
#define XtRPixel "Pixel"
#define XtRPixmap "Pixmap"
#define XtRPointer "Pointer"
#define XtRPosition "Position"
#define XtRScreen "Screen"
#define XtRShort "Short"
#define XtRString "String"
#define XtRStringArray "StringArray"
#define XtRStringTable "StringTable"
#define XtRUnsignedChar "UnsignedChar"
#define XtRTranslationTable "TranslationTable"
#define XtRVisual "Visual"
#define XtRWidget "Widget"
#define XtRWidgetClass "WidgetClass"
#define XtRWidgetList "WidgetList"
#define XtRWindow "Window"
#else
extern char XtRAcceleratorTable[];
extern char XtRAtom[];
extern char XtRBitmap[];
extern char XtRBool[];
extern char XtRBoolean[];
extern char XtRCallback[];
extern char XtRCallProc[];
extern char XtRCardinal[];
extern char XtRColor[];
extern char XtRColormap[];
extern char XtRCursor[];
extern char XtRDimension[];
extern char XtRDisplay[];
extern char XtREditMode[];
extern char XtREnum[];
extern char XtRFile[];
extern char XtRFloat[];
extern char XtRFont[];
extern char XtRFontSet[];
extern char XtRFontStruct[];
extern char XtRFunction[];
extern char XtRGeometry[];
extern char XtRImmediate[];
extern char XtRInitialState[];
extern char XtRInt[];
extern char XtRJustify[];
#define XtRLongBoolean XtRBool
extern char XtRObject[];
extern char XtROrientation[];
extern char XtRPixel[];
extern char XtRPixmap[];
extern char XtRPointer[];
extern char XtRPosition[];
extern char XtRScreen[];
extern char XtRShort[];
extern char XtRString[];
extern char XtRStringArray[];
extern char XtRStringTable[];
extern char XtRUnsignedChar[];
extern char XtRTranslationTable[];
extern char XtRVisual[];
extern char XtRWidget[];
extern char XtRWidgetClass[];
extern char XtRWidgetList[];
extern char XtRWindow[];
#endif

/* Boolean enumeration constants */

#ifdef XTSTRINGDEFINES
#define XtEoff "off"
#define XtEfalse "false"
#define XtEno "no"
#define XtEon "on"
#define XtEtrue "true"
#define XtEyes "yes"
#else
extern char XtEoff[];
extern char XtEfalse[];
extern char XtEno[];
extern char XtEon[];
extern char XtEtrue[];
extern char XtEyes[];
#endif

/* Orientation enumeration constants */

#ifdef XTSTRINGDEFINES
#define XtEvertical "vertical"
#define XtEhorizontal "horizontal"
#else
extern char XtEvertical[];
extern char XtEhorizontal[];
#endif

/* text edit enumeration constants */

#ifdef XTSTRINGDEFINES
#define XtEtextRead "read"
#define XtEtextAppend "append"
#define XtEtextEdit "edit"
#else
extern char XtEtextRead[];
extern char XtEtextAppend[];
extern char XtEtextEdit[];
#endif

/* color enumeration constants */

#ifdef XTSTRINGDEFINES
#define XtExtdefaultbackground "xtdefaultbackground"
#define XtExtdefaultforeground "xtdefaultforeground"
#else
extern char XtExtdefaultbackground[];
extern char XtExtdefaultforeground[];
#endif

/* font constant */

#ifdef XTSTRINGDEFINES
#define XtExtdefaultfont "xtdefaultfont"
#else
extern char XtExtdefaultfont[];
#endif

#endif /*_XtStringDefs_h_*/
