/* $XConsortium: Scale.h,v 1.4 94/02/06 20:13:21 rws Exp $ */
/*
 * Copyright 1993 Massachusetts Institute of Technology
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
#ifndef _XawScale_h
#define _XawScale_h

/***********************************************************************
 *
 * Scale Widget
 *
 ***********************************************************************/

#include <X11/Xaw/Simple.h>
#include <X11/Xmu/Converters.h>

/* Resources:

 All the SimpleWidget resources plus...
 Name                Class              RepType         Default Value
 ----                -----              -------         -------------
 aspectRatio         AspectRatio        Float           1.0
 autoscale           Autoscale          Boolean         True
 bufferSize          BufferSize         Cardinal        1024
 foreground          Foreground         Pixel           XtDefaultForeground
 gravity             Gravity            XtGravity       ForgetGravity
 image               Image              XImage*         NULL
 internalHeight      Height             Dimension       2
 internalWidth       Width              Dimension       2
 pasteBuffer         PasteBuffer        Boolean         False
 precision           Precision          Float           0.001
 proportional        Proportional       Boolean         False
 resize              Resize             Boolean         True
 scaleX              ScaleValue         Float           1.0
 scaleY              ScaleValue         Float           1.0
 userData            UserData           XtPointer       NULL
 visual              Visual             Visual*         CopyFromParent
*/

#ifndef _XtStringDefs_h_
#define XtNforeground "foreground"
#define XtNinternalWidth "internalWidth"
#define XtNinternalHeight "internalHeight"
#define XtNresize "resize"
#define XtCResize "Resize"
#endif

#define XtNaspectRatio "aspectRatio"
#define XtCAspectRatio "AspectRatio"
#define XtNbufferSize "bufferSize"
#define XtCBufferSize "BufferSize"
#define XtNscaleX "scaleX"
#define XtNscaleY "scaleY"
#define XtCScaleFactor "ScaleFactor"
#define XtNautoscale "autoscale"
#define XtCAutoscale "Autoscale"
#define XtNproportional "proportional"
#define XtCProportional "Proportional"
#define XtNprecision "precision"
#define XtCPrecision "Precision"
#define XtNgravity "gravity"
#define XtCGravity "Gravity"
#define XtNpasteBuffer "pasteBuffer"
#define XtCPasteBuffer "PasteBuffer"
#define XtNimage "image"
#define XtCImage "image"
#define XtNexponent "exponent"
#define XtCExponent "Exponent"
#define XtNuserData "userData"
#define XtCuserData "UserData"
#define XtRuserData "UserData"
#define XtRImage "Image"
#ifndef XtNvisual
#define XtNvisual "visual"
#endif
#define XtCvisual "Visual"
#define XtRvisual "Visual"

extern void AWSetImage();
extern void SWSetImage();

/* Class record constants */

extern WidgetClass scaleWidgetClass;

typedef struct _ScaleClassRec *ScaleWidgetClass;
typedef struct _ScaleRec      *ScaleWidget;

#endif /* _XawScale_h */

