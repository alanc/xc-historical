/* $XConsortium: errors.c,v 1.1 93/07/19 11:39:19 mor Exp $ */

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

#define NEED_EVENTS	/* so XErrorEvent will get pulled in */

#include "XIElibint.h"


Bool
_XieFloError (display, host, wire)

Display		*display;
XErrorEvent	*host;
xError		*wire;

{
    XieFloAccessError	*flo_host_error = (XieFloAccessError *) host;
    xieFloAccessErr	*flo_wire_error = (xieFloAccessErr *) wire;

    /*
     * All flo errors have this basic info.
     */

    flo_host_error->error_code     = flo_wire_error->code;
    flo_host_error->flo_error_code = flo_wire_error->floErrorCode;
    flo_host_error->name_space     = flo_wire_error->nameSpace;
    flo_host_error->flo_id         = flo_wire_error->floID;
    flo_host_error->phototag       = flo_wire_error->phototag;
    flo_host_error->elem_type      = flo_wire_error->type;
    flo_host_error->minor_code     = flo_wire_error->minorOpcode;
    flo_host_error->request_code   = flo_wire_error->majorOpcode;


    /*
     * Now handle the particularites of each flo error.
     */

    switch (((xieFloAccessErr *) wire)->floErrorCode)
    {
    case xieErrNoFloAccess:
    case xieErrNoFloAlloc:
    case xieErrNoFloElement:
    case xieErrNoFloID:
    case xieErrNoFloMatch:
    case xieErrNoFloSource:
    case xieErrNoFloImplementation:

	break;

    case xieErrNoFloColormap:
    case xieErrNoFloColorList:
    case xieErrNoFloDrawable:
    case xieErrNoFloGC:
    case xieErrNoFloLUT:
    case xieErrNoFloPhotomap:
    case xieErrNoFloROI:

    {
	XieFloResourceError	*host_error = (XieFloResourceError *) host;
	xieFloResourceErr	*wire_error = (xieFloResourceErr *) wire;

	host_error->resource_id = wire_error->resourceID;
	break;
    }

    case xieErrNoFloDomain:
    {
	XieFloDomainError	*host_error = (XieFloDomainError *) host;
	xieFloDomainErr		*wire_error = (xieFloDomainErr *) wire;

	host_error->domain_src = wire_error->domainSrc;
	break;
    }

    case xieErrNoFloOperator:
    {
	XieFloOperatorError	*host_error = (XieFloOperatorError *) host;
	xieFloOperatorErr	*wire_error = (xieFloOperatorErr *) wire;

	host_error->operator = wire_error->operator;
	break;
    }

    case xieErrNoFloTechnique:
    {
	XieFloTechniqueError	*host_error = (XieFloTechniqueError *) host;
	xieFloTechniqueErr	*wire_error = (xieFloTechniqueErr *) wire;

	host_error->technique_number = wire_error->techniqueNumber;
	host_error->num_tech_params  = wire_error->lenTechParams;
	break;
    }

    case xieErrNoFloValue:
    {
	XieFloValueError	*host_error = (XieFloValueError *) host;
	xieFloValueErr		*wire_error = (xieFloValueErr *) wire;

	host_error->bad_value = wire_error->badValue;
	break;
    }

    default:
        return (False);
    }

    return (True);
}
