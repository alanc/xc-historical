/* $XConsortium: pl_free.c,v 1.7 92/05/07 23:28:34 mor Exp $ */

/************************************************************************
Copyright 1992 by the Massachusetts Institute of Technology, Cambridge,
Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

*************************************************************************/

#include "PEXlib.h"
#include "PEXlibint.h"

#define FreeIfNotNull(_ptr) if (_ptr) PEXFreeBuf (_ptr)


void PEXFreeEnumInfo (numCounts, infoCount, enumInfo)

INPUT unsigned long 	numCounts;
INPUT unsigned long 	*infoCount;
INPUT PEXEnumTypeDesc 	*enumInfo;

{
    PEXEnumTypeDesc	*desc = enumInfo;
    int			i, j;


    for (i = 0; i < numCounts; i++)
	for (j = 0; j < infoCount[i]; j++)
	{
	    FreeIfNotNull ((char *) desc->descriptor);
	    desc++;
	}

    PEXFreeBuf ((char *) infoCount);
    PEXFreeBuf ((char *) enumInfo);
}


void PEXFreeEscapeReply (escapeReply)

INPUT char		*escapeReply;

{
    PEXFreeBuf ((char *) escapeReply);
}


void PEXFreeFontInfo (numFontInfo, fontInfo)

INPUT unsigned long	numFontInfo;
INPUT PEXFontInfo	*fontInfo;

{
    PEXFontInfo		*info = fontInfo;
    int			i;


    for (i = 0; i < numFontInfo; i++)
    {
	PEXFreeBuf ((char *) info->prop);
	info++;
    }

    PEXFreeBuf ((char *) fontInfo);
}


void PEXFreeFontNames (numFontNames, fontNames)

INPUT unsigned long	numFontNames;
INPUT PEXStringData	*fontNames;

{
    int i;


    for (i = 0; i < numFontNames; i++)
	PEXFreeBuf ((char *) fontNames[i].ch);

    PEXFreeBuf ((char *) fontNames);
}


void PEXFreePCAttributes (pcAttr)

INPUT PEXPCAttributes	*pcAttr;

{
    FreeIfNotNull ((char *) pcAttr->model_clip_volume.half_space);
    FreeIfNotNull ((char *) pcAttr->light_state.light_index);
    FreeIfNotNull ((char *) pcAttr->para_surf_char.psc.level_curves.parameter);
    FreeIfNotNull ((char *) pcAttr->para_surf_char.psc.data.ch);

    PEXFreeBuf ((char *) pcAttr);
}


void PEXFreePickDeviceAttributes (pdAttr)

PEXPickDeviceAttributes		*pdAttr;

{
    FreeIfNotNull (pdAttr->path.element);

    PEXFreeBuf ((char *) pdAttr);
}


void PEXFreePickMeasureAttributes (pmAttr)

PEXPickMeasureAttributes	 *pmAttr;

{
    FreeIfNotNull ((char *) pmAttr->picked_prim.element);

    PEXFreeBuf ((char *) pmAttr);
}


void PEXFreePickPaths (numPickPaths, pickPaths)

INPUT unsigned long	numPickPaths;
INPUT PEXPickPath	*pickPaths;

{
    int i;


    for (i = 0; i < numPickPaths; i++)
	PEXFreeBuf ((char *) pickPaths[i].element);

    PEXFreeBuf ((char *) pickPaths);
}


void PEXFreeRendererAttributes (rdrAttr)

INPUT PEXRendererAttributes	*rdrAttr;

{
    FreeIfNotNull ((char *) rdrAttr->current_path.element);
    FreeIfNotNull ((char *) rdrAttr->clip_list.device_rect);
    FreeIfNotNull ((char *) rdrAttr->pick_start_path.element);

    PEXFreeBuf ((char *) rdrAttr);
}


void PEXFreeSearchContextAttributes (scAttr)

PEXSCAttributes		*scAttr;

{
    FreeIfNotNull ((char *) scAttr->start_path.element);
    FreeIfNotNull ((char *) scAttr->normal.pair);
    FreeIfNotNull ((char *) scAttr->inverted.pair);

    PEXFreeBuf ((char *) scAttr);
}


void PEXFreeStructurePaths (numPaths, paths)

INPUT unsigned long	numPaths;
INPUT PEXStructurePath	*paths;

{
    int i;


    for (i = 0; i < numPaths; i++)
	PEXFreeBuf ((char *) paths[i].element);

    PEXFreeBuf ((char *) paths);
}


void PEXFreeTableEntries (tableType, numTableEntries, tableEntries)

INPUT int		tableType;
INPUT unsigned int	numTableEntries;
INPUT PEXPointer	tableEntries;

{
    int 	i;
    

    switch (tableType)
    {
    case PEXLUTPattern:
    {
	PEXPatternEntry *entries = (PEXPatternEntry *) tableEntries;
	
	for (i = 0; i < numTableEntries; i++)
	    PEXFreeBuf ((char *) entries[i].color);
	break;
    }
    
    case PEXLUTTextFont:
    {
	PEXTextFontEntry *entries = (PEXTextFontEntry *) tableEntries;
	
	for (i = 0; i < numTableEntries; i++)
	    PEXFreeBuf ((char *) entries[i].fonts);
	break;
    }

    case PEXLUTLineBundle:
    case PEXLUTMarkerBundle:
    case PEXLUTTextBundle:
    case PEXLUTInteriorBundle:
    case PEXLUTEdgeBundle:
    case PEXLUTColor:
    case PEXLUTView:
    case PEXLUTLight:
    case PEXLUTDepthCue:
    case PEXLUTColorApprox:
        break;
    }
}


void PEXFreeWorkstationAttributes (wksAttr)

INPUT PEXWorkstationAttributes	*wksAttr;

{
    FreeIfNotNull ((char *) wksAttr->defined_views.views);
    FreeIfNotNull ((char *) wksAttr->posted_structures.structure);

    PEXFreeBuf ((char *) wksAttr);
}
