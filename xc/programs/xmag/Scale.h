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

#define XtNaspectRatio "aspectRatio"
#define XtCAspectRatio "AspectRatio"
#define XtNforeground "foreground"
#define XtNinternalWidth "internalWidth"
#define XtNinternalHeight "internalHeight"
#define XtNbufferSize "bufferSize"
#define XtCBufferSize "BufferSize"
#define XtNscaleX "scaleX"
#define XtNscaleY "scaleY"
#define XtCScaleFactor "ScaleFactor"
#define XtNresize "resize"
#define XtCResize "Resize"
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
#define XtNvisual "visual"
#define XtCvisual "Visual"
#define XtRvisual "Visual"

extern void AWSetImage();
extern void SWSetImage();

/* Class record constants */

extern WidgetClass scaleWidgetClass;

typedef struct _ScaleClassRec *ScaleWidgetClass;
typedef struct _ScaleRec      *ScaleWidget;

#endif /* _XawScale_h */

