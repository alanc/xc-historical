/*
 * $XConsortium: AllWidgets.c,v 1.6 94/01/31 10:37:02 kaleb Exp $
 */

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
 */

#include <X11/IntrinsicP.h>
#include <X11/Xmu/WidgetNode.h>

extern WidgetClass applicationShellWidgetClass;
extern WidgetClass asciiSinkObjectClass;
extern WidgetClass asciiSrcObjectClass;
extern WidgetClass asciiTextWidgetClass;
extern WidgetClass boxWidgetClass;
extern WidgetClass commandWidgetClass;
extern WidgetClass compositeWidgetClass;
extern WidgetClass constraintWidgetClass;
extern WidgetClass coreWidgetClass;
extern WidgetClass dialogWidgetClass;
extern WidgetClass formWidgetClass;
extern WidgetClass gripWidgetClass;
extern WidgetClass labelWidgetClass;
extern WidgetClass listWidgetClass;
extern WidgetClass menuButtonWidgetClass;
extern WidgetClass multiSinkObjectClass;
extern WidgetClass multiSrcObjectClass;
extern WidgetClass objectClass;
extern WidgetClass overrideShellWidgetClass;
extern WidgetClass panedWidgetClass;
extern WidgetClass pannerWidgetClass;
extern WidgetClass portholeWidgetClass;
extern WidgetClass rectObjClass;
extern WidgetClass repeaterWidgetClass;
extern WidgetClass scrollbarWidgetClass;
extern WidgetClass shellWidgetClass;
extern WidgetClass simpleMenuWidgetClass;
extern WidgetClass simpleWidgetClass;
extern WidgetClass smeBSBObjectClass;
extern WidgetClass smeLineObjectClass;
extern WidgetClass smeObjectClass;
extern WidgetClass stripChartWidgetClass;
extern WidgetClass textSinkObjectClass;
extern WidgetClass textSrcObjectClass;
extern WidgetClass textWidgetClass;
extern WidgetClass toggleWidgetClass;
extern WidgetClass topLevelShellWidgetClass;
extern WidgetClass transientShellWidgetClass;
extern WidgetClass treeWidgetClass;
extern WidgetClass vendorShellWidgetClass;
extern WidgetClass viewportWidgetClass;
extern WidgetClass wmShellWidgetClass;

XmuWidgetNode XawWidgetArray[] = {
{ "applicationShell", &applicationShellWidgetClass },
{ "asciiSink", &asciiSinkObjectClass },
{ "asciiSrc", &asciiSrcObjectClass },
{ "asciiText", &asciiTextWidgetClass },
{ "box", &boxWidgetClass },
{ "command", &commandWidgetClass },
{ "composite", &compositeWidgetClass },
{ "constraint", &constraintWidgetClass },
{ "core", &coreWidgetClass },
{ "dialog", &dialogWidgetClass },
{ "form", &formWidgetClass },
{ "grip", &gripWidgetClass },
{ "label", &labelWidgetClass },
{ "list", &listWidgetClass },
{ "menuButton", &menuButtonWidgetClass },
{ "multiSink", &multiSinkObjectClass },
{ "multiSrc", &multiSrcObjectClass },
{ "object", &objectClass },
{ "overrideShell", &overrideShellWidgetClass },
{ "paned", &panedWidgetClass },
{ "panner", &pannerWidgetClass },
{ "porthole", &portholeWidgetClass },
{ "rect", &rectObjClass },
{ "repeater", &repeaterWidgetClass },
{ "scrollbar", &scrollbarWidgetClass },
{ "shell", &shellWidgetClass },
{ "simpleMenu", &simpleMenuWidgetClass },
{ "simple", &simpleWidgetClass },
{ "smeBSB", &smeBSBObjectClass },
{ "smeLine", &smeLineObjectClass },
{ "sme", &smeObjectClass },
{ "stripChart", &stripChartWidgetClass },
{ "textSink", &textSinkObjectClass },
{ "textSrc", &textSrcObjectClass },
{ "text", &textWidgetClass },
{ "toggle", &toggleWidgetClass },
{ "topLevelShell", &topLevelShellWidgetClass },
{ "transientShell", &transientShellWidgetClass },
{ "tree", &treeWidgetClass },
{ "vendorShell", &vendorShellWidgetClass },
{ "viewport", &viewportWidgetClass },
{ "wmShell", &wmShellWidgetClass },
};

int XawWidgetCount = XtNumber(XawWidgetArray);

