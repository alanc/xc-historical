/* $XConsortium: events.c,v 1.2 94/01/12 19:44:28 rws Exp $ */

/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
******************************************************************************/

#define NEED_EVENTS	/* so xEvent will get pulled in */

#include "XIElibint.h"


Status
_XieColorAllocEvent (display, host, wire)

Display		*display;
XEvent		*host;
xEvent		*wire;

{
    XieColorAllocEvent	     *host_event = (XieColorAllocEvent *) host;
    xieColorAllocEvn	     *wire_event = (xieColorAllocEvn *) wire;

    host_event->type                  = wire_event->event & 0x7f;
    host_event->serial                = wire_event->sequenceNum;
    host_event->send_event            = (wire_event->event & 0x80) != 0;
    host_event->display               = display;
    host_event->name_space            = wire_event->instanceNameSpace;
    host_event->time                  = wire_event->time;
    host_event->flo_id                = wire_event->instanceFloID;
    host_event->src                   = wire_event->src;
    host_event->elem_type             = wire_event->type;
    host_event->color_list            = wire_event->colorList;
    host_event->color_alloc_technique = wire_event->colorAllocTechnique;
    host_event->color_alloc_data      = wire_event->data;

    return (True);
}


Status
_XieDecodeNotifyEvent (display, host, wire)

Display		*display;
XEvent		*host;
xEvent		*wire;

{
    XieDecodeNotifyEvent     *host_event = (XieDecodeNotifyEvent *) host;
    xieDecodeNotifyEvn	     *wire_event = (xieDecodeNotifyEvn *) wire;

    host_event->type             = wire_event->event & 0x7f;
    host_event->serial           = wire_event->sequenceNum;
    host_event->send_event       = (wire_event->event & 0x80) != 0;
    host_event->display          = display;
    host_event->name_space       = wire_event->instanceNameSpace;
    host_event->time             = wire_event->time;
    host_event->flo_id           = wire_event->instanceFloID;
    host_event->src              = wire_event->src;
    host_event->elem_type        = wire_event->type;
    host_event->decode_technique = wire_event->decodeTechnique;
    host_event->aborted          = wire_event->aborted;
    host_event->band_number      = wire_event->bandNumber;
    host_event->width		 = wire_event->width;
    host_event->height		 = wire_event->height;

    return (True);
}


Status
_XieExportAvailableEvent (display, host, wire)

Display		*display;
XEvent		*host;
xEvent		*wire;

{
    XieExportAvailableEvent  *host_event = (XieExportAvailableEvent *) host;
    xieExportAvailableEvn    *wire_event = (xieExportAvailableEvn *) wire;

    host_event->type       = wire_event->event & 0x7f;
    host_event->serial     = wire_event->sequenceNum;
    host_event->send_event = (wire_event->event & 0x80) != 0;
    host_event->display    = display;
    host_event->name_space = wire_event->instanceNameSpace;
    host_event->time       = wire_event->time;
    host_event->flo_id     = wire_event->instanceFloID;
    host_event->src        = wire_event->src;
    host_event->elem_type  = wire_event->type;
    host_event->band_number = wire_event->bandNumber;
    host_event->data[0]    = wire_event->data0;
    host_event->data[1]    = wire_event->data1;
    host_event->data[2]    = wire_event->data2;

    return (True);
}


Status
_XieImportObscuredEvent (display, host, wire)

Display		*display;
XEvent		*host;
xEvent		*wire;

{
    XieImportObscuredEvent   *host_event = (XieImportObscuredEvent *) host;
    xieImportObscuredEvn     *wire_event = (xieImportObscuredEvn *) wire;

    host_event->type       = wire_event->event & 0x7f;
    host_event->serial     = wire_event->sequenceNum;
    host_event->send_event = (wire_event->event & 0x80) != 0;
    host_event->display    = display;
    host_event->name_space = wire_event->instanceNameSpace;
    host_event->time       = wire_event->time;
    host_event->flo_id     = wire_event->instanceFloID;
    host_event->src        = wire_event->src;
    host_event->elem_type  = wire_event->type;
    host_event->window     = wire_event->window;
    host_event->x	   = wire_event->x;
    host_event->y	   = wire_event->y;
    host_event->width	   = wire_event->width;
    host_event->height	   = wire_event->height;

    return (True);
}


Status
_XiePhotofloDoneEvent (display, host, wire)

Display		*display;
XEvent		*host;
xEvent		*wire;

{
    XiePhotofloDoneEvent     *host_event = (XiePhotofloDoneEvent *) host;
    xiePhotofloDoneEvn	     *wire_event = (xiePhotofloDoneEvn *) wire;

    host_event->type       = wire_event->event & 0x7f;
    host_event->serial     = wire_event->sequenceNum;
    host_event->send_event = (wire_event->event & 0x80) != 0;
    host_event->display    = display;
    host_event->name_space = wire_event->instanceNameSpace;
    host_event->time       = wire_event->time;
    host_event->flo_id     = wire_event->instanceFloID;

    return (True);
}
