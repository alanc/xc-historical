#ifndef lint
static char rcsid[] = "$Header: AsciiText.c,v 1.3 88/01/27 09:24:13 swick Locked $";
#endif lint

/* Copyright	Massachusetts Institute of Technology	1987 */

#include <X/copyright.h>
#include "IntrinsicP.h"
#include "Atoms.h"
#include "AsciiTextP.h"

/* from Text.c */
#define DEFAULTVALUE ~0

extern void ForceBuildLineTable(); /* in Text.c */

static XtResource string_resources[] = {
  {XtNstring, XtCString, XtRString, sizeof(String),
     XtOffset(AsciiStringWidget, ascii_string.string), XtRString, NULL}
};

static XtResource disk_resources[] = {
  {XtNfile, XtCFile, XtRString, sizeof(String),
     XtOffset(AsciiDiskWidget, ascii_disk.file_name), XtRString, NULL}
};

static void StringClassInitialize(), StringInitialize(),
    StringCreateSourceSink(), StringDestroy();
static Boolean StringSetValues();

static void DiskClassInitialize(), DiskInitialize(),
    DiskCreateSourceSink(), DiskDestroy();
static Boolean DiskSetValues();

AsciiStringClassRec asciiStringClassRec = {
  { /* core fields */
    /* superclass       */      (WidgetClass) &textClassRec,
    /* class_name       */      "Text",
    /* widget_size      */      sizeof(AsciiStringRec),
    /* class_initialize */      StringClassInitialize,
    /* class_part_init  */	NULL,
    /* class_inited     */      FALSE,
    /* initialize       */      StringInitialize,
    /* initialize_hook  */	StringCreateSourceSink,
    /* realize          */      XtInheritRealize,
    /* actions          */      textActionsTable,
    /* num_actions      */      0,
    /* resources        */      string_resources,
    /* num_ resource    */      XtNumber(string_resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion  */      TRUE,
    /* compress_exposure*/      FALSE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest */      FALSE,
    /* destroy          */      StringDestroy,
    /* resize           */      XtInheritResize,
    /* expose           */      XtInheritExpose,
    /* set_values       */      StringSetValues,
    /* set_values_hook  */	NULL,
    /* set_values_almost*/	XtInheritSetValuesAlmost,
    /* get_values_hook  */	NULL,
    /* accept_focus     */      XtInheritAcceptFocus,
    /* version          */	XtVersion,
    /* callback_private */      NULL,
    /* tm_table         */      NULL
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
    /* class_part_init  */	NULL,
    /* class_inited     */      FALSE,
    /* initialize       */      DiskInitialize,
    /* initialize_hook  */	DiskCreateSourceSink,
    /* realize          */      XtInheritRealize,
    /* actions          */      textActionsTable,
    /* num_actions      */      0,
    /* resources        */      disk_resources,
    /* num_ resource    */      XtNumber(disk_resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion  */      TRUE,
    /* compress_exposure*/      FALSE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest */      FALSE,
    /* destroy          */      DiskDestroy,
    /* resize           */      XtInheritResize,
    /* expose           */      XtInheritExpose,
    /* set_values       */      DiskSetValues,
    /* set_values_hook  */	NULL,
    /* set_values_almost*/	XtInheritSetValuesAlmost,
    /* get_values_hook  */	NULL,
    /* accept_focus     */      XtInheritAcceptFocus,
    /* version          */	XtVersion,
    /* callback_private */      NULL,
    /* tm_table         */      NULL
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
static void StringInitialize(request, new)
    Widget request, new;
{
    /* superclass Initialize can't set the following,
     * as it didn't know the source or sink when it was called */
    if (request->core.height == DEFAULTVALUE)
	new->core.height = DEFAULTVALUE;
}

static void StringCreateSourceSink(widget, args, num_args)
    Widget widget;
    ArgList args;
    Cardinal *num_args;
{
    AsciiStringWidget w = (AsciiStringWidget)widget;

    w->text.source = XtStringSourceCreate( w->core.parent, args, *num_args );
    w->text.sink = XtAsciiSinkCreate( w->core.parent, args, *num_args );


    if (w->core.height == DEFAULTVALUE)
        w->core.height += (*w->text.sink->MaxHeight)(w, 1);

    w->text.lastPos = /* GETLASTPOS */
      (*w->text.source->Scan) ( w->text.source, 0, XtstAll,
			        XtsdRight, 1, TRUE );

    ForceBuildLineTable( (TextWidget)w );
}


/* ARGSUSED */
static Boolean StringSetValues(current, request, new)
    Widget current, request, new;
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
static void DiskInitialize(request, new)
    Widget request, new;
{
    /* superclass Initialize can't set the following,
     * as it didn't know the source or sink when it was called */
    if (request->core.height == DEFAULTVALUE)
	new->core.height = DEFAULTVALUE;
}

static void DiskCreateSourceSink(widget, args, num_args)
    Widget widget;
    ArgList args;
    Cardinal *num_args;
{
    AsciiDiskWidget w = (AsciiDiskWidget)widget;

    w->text.source = XtDiskSourceCreate( w->core.parent, args, *num_args );
    w->text.sink = XtAsciiSinkCreate( w->core.parent, args, *num_args );

    w->text.lastPos = /* GETLASTPOS */
      (*w->text.source->Scan) ( w->text.source, 0, XtstAll,
			        XtsdRight, 1, TRUE );

    if (w->core.height == DEFAULTVALUE)
        w->core.height += (*w->text.sink->MaxHeight)(w, 1);

    ForceBuildLineTable( (TextWidget)w );
}


/* ARGSUSED */
static Boolean DiskSetValues(current, request, new)
    Widget current, request, new;
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
