/* $XConsortium: StringDefs.c,v 1.1 91/01/04 19:10:43 converse Exp $ */

/*
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

/* Resource names */

Const char XtNaccelerators[] = "accelerators";
Const char XtNallowHoriz[] = "allowHoriz";
Const char XtNallowVert[] = "allowVert";
Const char XtNancestorSensitive[] = "ancestorSensitive";
Const char XtNbackground[] = "background";
Const char XtNbackgroundPixmap[] = "backgroundPixmap";
Const char XtNbitmap[] = "bitmap";
Const char XtNborderColor[] = "borderColor";
Const char XtNborder[] = "borderColor";
Const char XtNborderPixmap[] = "borderPixmap";
Const char XtNborderWidth[] = "borderWidth";
Const char XtNcallback[] = "callback";
Const char XtNchildren[] = "children";
Const char XtNcolormap[] = "colormap";
Const char XtNdepth[] = "depth";
Const char XtNdestroyCallback[] = "destroyCallback";
Const char XtNeditType[] = "editType";
Const char XtNfile[] = "file";
Const char XtNfont[] = "font";
Const char XtNfontSet[] = "fontSet";
Const char XtNforceBars[] = "forceBars";
Const char XtNforeground[] = "foreground";
Const char XtNfunction[] = "function";
Const char XtNheight[] = "height";
Const char XtNhighlight[] = "highlight";
Const char XtNhSpace[] = "hSpace";
Const char XtNindex[] = "index";
Const char XtNinitialResourcesPersistent[] = "initialResourcesPersistent";
Const char XtNinnerHeight[] = "innerHeight";
Const char XtNinnerWidth[] = "innerWidth";
Const char XtNinnerWindow[] = "innerWindow";
Const char XtNinsertPosition[] = "insertPosition";
Const char XtNinternalHeight[] = "internalHeight";
Const char XtNinternalWidth[] = "internalWidth";
Const char XtNjumpProc[] = "jumpProc";
Const char XtNjustify[] = "justify";
Const char XtNknobHeight[] = "knobHeight";
Const char XtNknobIndent[] = "knobIndent";
Const char XtNknobPixel[] = "knobPixel";
Const char XtNknobWidth[] = "knobWidth";
Const char XtNlabel[] = "label";
Const char XtNlength[] = "length";
Const char XtNlowerRight[] = "lowerRight";
Const char XtNmappedWhenManaged[] = "mappedWhenManaged";
Const char XtNmenuEntry[] = "menuEntry";
Const char XtNname[] = "name";
Const char XtNnotify[] = "notify";
Const char XtNnumChildren[] = "numChildren";
Const char XtNorientation[] = "orientation";
Const char XtNparameter[] = "parameter";
Const char XtNpixmap[] = "pixmap";
Const char XtNpopupCallback[] = "popupCallback";
Const char XtNpopdownCallback[] = "popdownCallback";
Const char XtNresize[] = "resize";
Const char XtNreverseVideo[] = "reverseVideo";
Const char XtNscreen[] = "screen";
Const char XtNscrollProc[] = "scrollProc";
Const char XtNscrollDCursor[] = "scrollDCursor";
Const char XtNscrollHCursor[] = "scrollHCursor";
Const char XtNscrollLCursor[] = "scrollLCursor";
Const char XtNscrollRCursor[] = "scrollRCursor";
Const char XtNscrollUCursor[] = "scrollUCursor";
Const char XtNscrollVCursor[] = "scrollVCursor";
Const char XtNselection[] = "selection";
Const char XtNselectionArray[] = "selectionArray";
Const char XtNsensitive[] = "sensitive";
Const char XtNshown[] = "shown";
Const char XtNspace[] = "space";
Const char XtNstring[] = "string";
Const char XtNtextOptions[] = "textOptions";
Const char XtNtextSink[] = "textSink";
Const char XtNtextSource[] = "textSource";
Const char XtNthickness[] = "thickness";
Const char XtNthumb[] = "thumb";
Const char XtNthumbProc[] = "thumbProc";
Const char XtNtop[] = "top";
Const char XtNtranslations[] = "translations";
Const char XtNunrealizeCallback[] = "unrealizeCallback";
Const char XtNupdate[] = "update";
Const char XtNuseBottom[] = "useBottom";
Const char XtNuseRight[] = "useRight";
Const char XtNvalue[] = "value";
Const char XtNvSpace[] = "vSpace";
Const char XtNwidth[] = "width";
Const char XtNwindow[] = "window";
Const char XtNx[] = "x";
Const char XtNy[] = "y";

/* Class types */ 

Const char XtCAccelerators[] = "Accelerators";
Const char XtCBackground[] = "Background";
Const char XtCBitmap[] = "Bitmap";
Const char XtCBoolean[] = "Boolean";
Const char XtCBorderColor[] = "BorderColor";
Const char XtCBorderWidth[] = "BorderWidth";
Const char XtCCallback[] = "Callback";
Const char XtCColormap[] = "Colormap";
Const char XtCColor[] = "Color";
Const char XtCCursor[] = "Cursor";
Const char XtCDepth[] = "Depth";
Const char XtCEditType[] = "EditType";
Const char XtCEventBindings[] = "EventBindings";
Const char XtCFile[] = "File";
Const char XtCFont[] = "Font";
Const char XtCFontSet[] = "FontSet";
Const char XtCForeground[] = "Foreground";
Const char XtCFraction[] = "Fraction";
Const char XtCFunction[] = "Function";
Const char XtCHeight[] = "Height";
Const char XtCHSpace[] = "HSpace";
Const char XtCIndex[] = "Index";
Const char XtCInitialResourcesPersistent[] = "InitialResourcesPersistent";
Const char XtCInsertPosition[] = "InsertPosition";
Const char XtCInterval[] = "Interval";
Const char XtCJustify[] = "Justify";
Const char XtCKnobIndent[] = "KnobIndent";
Const char XtCKnobPixel[] = "KnobPixel";
Const char XtCLabel[] = "Label";
Const char XtCLength[] = "Length";
Const char XtCMappedWhenManaged[] = "MappedWhenManaged";
Const char XtCMargin[] = "Margin";
Const char XtCMenuEntry[] = "MenuEntry";
Const char XtCNotify[] = "Notify";
Const char XtCOrientation[] = "Orientation";
Const char XtCParameter[] = "Parameter";
Const char XtCPixmap[] = "Pixmap";
Const char XtCPosition[] = "Position";
Const char XtCReadOnly[] = "ReadOnly";
Const char XtCResize[] = "Resize";
Const char XtCReverseVideo[] = "ReverseVideo";
Const char XtCScreen[] = "Screen";
Const char XtCScrollProc[] = "ScrollProc";
Const char XtCScrollDCursor[] = "ScrollDCursor";
Const char XtCScrollHCursor[] = "ScrollHCursor";
Const char XtCScrollLCursor[] = "ScrollLCursor";
Const char XtCScrollRCursor[] = "ScrollRCursor";
Const char XtCScrollUCursor[] = "ScrollUCursor";
Const char XtCScrollVCursor[] = "ScrollVCursor";
Const char XtCSelection[] = "Selection";
Const char XtCSensitive[] = "Sensitive";
Const char XtCSelectionArray[] = "SelectionArray";
Const char XtCSpace[] = "Space";
Const char XtCString[] = "String";
Const char XtCTextOptions[] = "TextOptions";
Const char XtCTextPosition[] = "TextPosition";
Const char XtCTextSink[] = "TextSink";
Const char XtCTextSource[] = "TextSource";
Const char XtCThickness[] = "Thickness";
Const char XtCThumb[] = "Thumb";
Const char XtCTranslations[] = "Translations";
Const char XtCValue[] = "Value";
Const char XtCVSpace[] = "VSpace";
Const char XtCWidth[] = "Width";
Const char XtCWindow[] = "Window";
Const char XtCX[] = "X";
Const char XtCY[] = "Y";

/* Representation types */

Const char XtRAcceleratorTable[] = "AcceleratorTable";
Const char XtRAtom[] = "Atom";
Const char XtRBitmap[] = "Bitmap";
Const char XtRBool[] = "Bool";
Const char XtRBoolean[] = "Boolean";
Const char XtRCallback[] = "Callback";
Const char XtRCallProc[] = "CallProc";
Const char XtRCardinal[] = "Cardinal";
Const char XtRColor[] = "Color";
Const char XtRColormap[] = "Colormap";
Const char XtRCursor[] = "Cursor";
Const char XtRDimension[] = "Dimension";
Const char XtRDisplay[] = "Display";
Const char XtREditMode[] = "EditMode";
Const char XtREnum[] = "Enum";
Const char XtRFile[] = "File";
Const char XtRFloat[] = "Float";
Const char XtRFont[] = "Font";
Const char XtRFontSet[] = "FontSet";
Const char XtRFontStruct[] = "FontStruct";
Const char XtRFunction[] = "Function";
Const char XtRGeometry[] = "Geometry";
Const char XtRImmediate[] = "Immediate";
Const char XtRInitialState[] = "InitialState";
Const char XtRInt[] = "Int";
Const char XtRJustify[] = "Justify";

Const char XtRObject[] = "Object";
Const char XtROrientation[] = "Orientation";
Const char XtRPixel[] = "Pixel";
Const char XtRPixmap[] = "Pixmap";
Const char XtRPointer[] = "Pointer";
Const char XtRPosition[] = "Position";
Const char XtRScreen[] = "Screen";
Const char XtRShort[] = "Short";
Const char XtRString[] = "String";
Const char XtRStringArray[] = "StringArray";
Const char XtRStringTable[] = "StringTable";
Const char XtRUnsignedChar[] = "UnsignedChar";
Const char XtRTranslationTable[] = "TranslationTable";
Const char XtRVisual[] = "Visual";
Const char XtRWidget[] = "Widget";
Const char XtRWidgetClass[] = "WidgetClass";
Const char XtRWidgetList[] = "WidgetList";
Const char XtRWindow[] = "Window";

/* Boolean enumeration constants */

Const char XtEoff[] = "off";
Const char XtEfalse[] = "false";
Const char XtEno[] = "no";
Const char XtEon[] = "on";
Const char XtEtrue[] = "true";
Const char XtEyes[] = "yes";

/* Orientation enumeration constants */

Const char XtEvertical[] = "vertical";
Const char XtEhorizontal[] = "horizontal";

/* text edit enumeration constants */

Const char XtEtextRead[] = "read";
Const char XtEtextAppend[] = "append";
Const char XtEtextEdit[] = "edit";

/* color enumeration constants */

Const char XtExtdefaultbackground[] = "xtdefaultbackground";
Const char XtExtdefaultforeground[] = "xtdefaultforeground";

/* font constant */

Const char XtExtdefaultfont[] = "xtdefaultfont";

/* Shell-specific resource names, class types, and representation types */

Const char XtNiconName[] = "iconName";
Const char XtCIconName[] = "IconName";
Const char XtNiconPixmap[] = "iconPixmap";
Const char XtCIconPixmap[] = "IconPixmap";
Const char XtNiconWindow[] = "iconWindow";
Const char XtCIconWindow[] = "IconWindow";
Const char XtNiconMask[] = "iconMask";
Const char XtCIconMask[] = "IconMask";
Const char XtNwindowGroup[] = "windowGroup";
Const char XtCWindowGroup[] = "WindowGroup";
Const char XtNvisual[] = "visual";
Const char XtCVisual[] = "Visual";
Const char XtNtitleEncoding[] = "titleEncoding";
Const char XtCTitleEncoding[] = "TitleEncoding";
 
Const char XtNsaveUnder[] = "saveUnder";
Const char XtCSaveUnder[] = "SaveUnder";
Const char XtNtransient[] = "transient";
Const char XtCTransient[] = "Transient";
Const char XtNoverrideRedirect[] = "overrideRedirect";
Const char XtCOverrideRedirect[] = "OverrideRedirect";
Const char XtNtransientFor[] = "transientFor";
Const char XtCTransientFor[] = "TransientFor";

Const char XtNiconNameEncoding[] = "iconNameEncoding";
Const char XtCIconNameEncoding[] = "IconNameEncoding";

Const char XtNallowShellResize[] = "allowShellResize";
Const char XtCAllowShellResize[] = "AllowShellResize";
Const char XtNcreatePopupChildProc[] = "createPopupChildProc";
Const char XtCCreatePopupChildProc[] = "CreatePopupChildProc";

Const char XtNtitle[] = "title";
Const char XtCTitle[] = "Title";

Const char XtNargc[] = "argc";
Const char XtCArgc[] = "Argc";
Const char XtNargv[] = "argv";
Const char XtCArgv[] = "Argv";
Const char XtNiconX[] = "iconX";
Const char XtCIconX[] = "IconX";
Const char XtNiconY[] = "iconY";
Const char XtCIconY[] = "IconY";
Const char XtNinput[] = "input";
Const char XtCInput[] = "Input";
Const char XtNiconic[] = "iconic";
Const char XtCIconic[] = "Iconic";
Const char XtNinitialState[] = "initialState";
Const char XtCInitialState[] = "InitialState";
Const char XtNgeometry[] = "geometry";
Const char XtCGeometry[] = "Geometry";
Const char XtNbaseWidth[] = "baseWidth";
Const char XtCBaseWidth[] = "BaseWidth";
Const char XtNbaseHeight[] = "baseHeight";
Const char XtCBaseHeight[] = "BaseHeight";
Const char XtNwinGravity[] = "winGravity";
Const char XtCWinGravity[] = "WinGravity";
Const char XtNminWidth[] = "minWidth";
Const char XtCMinWidth[] = "MinWidth";
Const char XtNminHeight[] = "minHeight";
Const char XtCMinHeight[] = "MinHeight";
Const char XtNmaxWidth[] = "maxWidth";
Const char XtCMaxWidth[] = "MaxWidth";
Const char XtNmaxHeight[] = "maxHeight";
Const char XtCMaxHeight[] = "MaxHeight";
Const char XtNwidthInc[] = "widthInc";
Const char XtCWidthInc[] = "WidthInc";
Const char XtNheightInc[] = "heightInc";
Const char XtCHeightInc[] = "HeightInc";
Const char XtNminAspectY[] = "minAspectY";
Const char XtCMinAspectY[] = "MinAspectY";
Const char XtNmaxAspectY[] = "maxAspectY";
Const char XtCMaxAspectY[] = "MaxAspectY";
Const char XtNminAspectX[] = "minAspectX";
Const char XtCMinAspectX[] = "MinAspectX";
Const char XtNmaxAspectX[] = "maxAspectX";
Const char XtCMaxAspectX[] = "MaxAspectX";
Const char XtNwmTimeout[] = "wmTimeout";
Const char XtCWmTimeout[] = "WmTimeout";
Const char XtNwaitForWm[] = "waitforwm";
Const char XtCWaitForWm[] = "Waitforwm";
