#include <X/copyright.h>

/* $Header: $ */
/* Copyright	Massachusetts Institute of Technology	1987 */

#include <X/Intrinsic.h>
#include <X/Atoms.h>
#include <X/AsciiText.h>
#include "AsciiTextP.h"

extern void ForceBuildLineTable();

static XtResource string_resources[] = {
  {XtNstring, XtCString, XrmRString, sizeof(String),
     XtOffset(AsciiStringWidget, ascii_string.string), XrmRString, NULL}
};

static XtResource disk_resources[] = {
  {XtNfile, XtCFile, XrmRString, sizeof(String),
     XtOffset(AsciiDiskWidget, ascii_disk.file_name), XrmRString, NULL}
};

static void StringClassInitialize(), StringInitialize(), StringDestroy();
static Boolean StringSetValues();

static void DiskClassInitialize(), DiskInitialize(), DiskDestroy();
static Boolean DiskSetValues();

AsciiStringClassRec asciiStringClassRec = {
  { /* core fields */
    /* superclass       */      (WidgetClass) &textClassRec,
    /* class_name       */      "Text",
    /* widget_size      */      sizeof(AsciiStringRec),
    /* class_initialize */      StringClassInitialize,
    /* class_inited     */      FALSE,
    /* initialize       */      StringInitialize,
    /* realize          */      XtInheritRealize,
    /* actions          */      textActionsTable,
    /* num_actions      */      0,
    /* resources        */      string_resources,
    /* num_ resource    */      XtNumber(string_resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion  */      TRUE,
    /* compress_exposure*/      FALSE,
    /* visible_interest */      FALSE,
    /* destroy          */      StringDestroy,
    /* resize           */      XtInheritResize,
    /* expose           */      XtInheritExpose,
    /* set_values       */      StringSetValues,
    /* accept_focus     */      XtInheritAcceptFocus,
    /* callback_private */      NULL,
    /* reserved_private */      NULL
  },
  { /* text fields */
    /* empty            */      0
  },
  { /* ascii_string fields */
    /* empty            */      0
  }
};

AsciiDiskClassRec asciiDiskClassRec = {
  { /* core fields */
    /* superclass       */      (WidgetClass) &textClassRec,
    /* class_name       */      "Text",
    /* widget_size      */      sizeof(AsciiDiskRec),
    /* class_initialize */      DiskClassInitialize,
    /* class_inited     */      FALSE,
    /* initialize       */      DiskInitialize,
    /* realize          */      XtInheritRealize,
    /* actions          */      textActionsTable,
    /* num_actions      */      0,
    /* resources        */      disk_resources,
    /* num_ resource    */      XtNumber(disk_resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion  */      TRUE,
    /* compress_exposure*/      FALSE,
    /* visible_interest */      FALSE,
    /* destroy          */      DiskDestroy,
    /* resize           */      XtInheritResize,
    /* expose           */      XtInheritExpose,
    /* set_values       */      DiskSetValues,
    /* accept_focus     */      XtInheritAcceptFocus,
    /* callback_private */      NULL,
    /* reserved_private */      NULL
  },
  { /* text fields */
    /* empty            */      0
  },
  { /* ascii_disk fields */
    /* empty            */      0
  }
};


WidgetClass asciiStringWidgetClass = (WidgetClass)&asciiStringClassRec;
WidgetClass asciiDiskWidgetClass = (WidgetClass)&asciiDiskClassRec;


static void StringClassInitialize()
{
    asciiStringClassRec.core_class.num_actions = textActionsTableCount;
}


/* ARGSUSED */
static void StringInitialize(request, new, args, num_args)
    Widget request, new;
    ArgList args;
    Cardinal *num_args;
{
    AsciiStringWidget w = (AsciiStringWidget)new;

    w->text.source = XtStringSourceCreate( w->core.parent, args, *num_args );
    w->text.sink = XtAsciiSinkCreate( w->core.parent, args, *num_args );

    /* superclass Initialize can't set the following,
     * as it didn't know the source or sink when it was called */
    if (request->core.height == DEFAULTVALUE)
        w->core.height += (*w->text.sink->maxHeight)(new, 1);

    w->text.lastPos = /* GETLASTPOS */
      (*w->text.source->scan) ( w->text.source, 0, XtstFile,
			        XtsdRight, 1, TRUE );

    ForceBuildLineTable( (TextWidget)new );
}


/* ARGSUSED */
static Boolean StringSetValues(current, request, new, last)
    Widget current, request, new;
    Boolean last;
{
    AsciiStringWidget old = (AsciiStringWidget)current;
    AsciiStringWidget w = (AsciiStringWidget)new;

    if (w->ascii_string.string != old->ascii_string.string)
        XtError( "SetValues on AsciiStringWidget string not supported." );

    return False;
}


static void StringDestroy(w)
    Widget w;
{
    XtStringSourceDestroy( ((AsciiStringWidget)w)->text.source );
    XtAsciiSinkDestroy( ((AsciiStringWidget)w)->text.sink );
}


static void DiskClassInitialize()
{
    asciiDiskClassRec.core_class.num_actions = textActionsTableCount;
}


/* ARGSUSED */
static void DiskInitialize(request, new, args, num_args)
    Widget request, new;
    ArgList args;
    Cardinal *num_args;
{
    AsciiDiskWidget w = (AsciiDiskWidget)new;

    w->text.source = XtDiskSourceCreate( w->core.parent, args, *num_args );
    w->text.sink = XtAsciiSinkCreate( w->core.parent, args, *num_args );

    /* superclass Initialize can't set the following,
     * as it didn't know the source or sink when it was called */
    if (request->core.height == DEFAULTVALUE)
        w->core.height += (*w->text.sink->maxHeight)(new, 1);

    w->text.lastPos = /* GETLASTPOS */
      (*w->text.source->scan) ( w->text.source, 0, XtstFile,
			        XtsdRight, 1, TRUE );

    ForceBuildLineTable( (TextWidget)new );
}


/* ARGSUSED */
static Boolean DiskSetValues(current, request, new, last)
    Widget current, request, new;
    Boolean last;
{
    AsciiDiskWidget old = (AsciiDiskWidget)current;
    AsciiDiskWidget w = (AsciiDiskWidget)new;

    if (w->ascii_disk.file_name != old->ascii_disk.file_name)
        XtError( "SetValues on AsciiDiskWidget file not supported." );

    return False;
}


static void DiskDestroy(w)
    Widget w;
{
    XtDiskSourceDestroy( ((AsciiDiskWidget)w)->text.source );
    XtAsciiSinkDestroy( ((AsciiDiskWidget)w)->text.sink );
}
