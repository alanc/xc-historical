/* $XConsortium: pl_free.c,v 1.1 92/05/08 15:13:00 mor Exp $ */

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
	PEXFreeBuf ((char *) info->props);
	info++;
    }

    PEXFreeBuf ((char *) fontInfo);
}


void PEXFreeFontNames (numFontNames, fontNames)

INPUT unsigned long	numFontNames;
INPUT char		**fontNames;

{
    int i;


    for (i = 0; i < numFontNames; i++)
	PEXFreeBuf (fontNames[i]);

    PEXFreeBuf ((char *) fontNames);
}


void PEXFreePCAttributes (pcAttr)

INPUT PEXPCAttributes	*pcAttr;

{
    FreeIfNotNull ((char *) pcAttr->model_clip_volume.half_spaces);
    FreeIfNotNull ((char *) pcAttr->light_state.indices);
    FreeIfNotNull ((char *) pcAttr->para_surf_char.psc.level_curves.parameters);
    FreeIfNotNull ((char *) pcAttr->para_surf_char.psc.imp_dep.data);

    PEXFreeBuf ((char *) pcAttr);
}


void PEXFreePDAttributes (pdAttr)

PEXPDAttributes		*pdAttr;

{
    FreeIfNotNull (pdAttr->path.elements);

    PEXFreeBuf ((char *) pdAttr);
}


void PEXFreePMAttributes (pmAttr)

PEXPMAttributes	 *pmAttr;

{
    FreeIfNotNull ((char *) pmAttr->pick_path.elements);

    PEXFreeBuf ((char *) pmAttr);
}


void PEXFreePickPaths (numPickPaths, pickPaths)

INPUT unsigned long	numPickPaths;
INPUT PEXPickPath	*pickPaths;

{
    int total_size, i;


    /*
     * Note that memory allocation of pick paths is optimized by
     * allocating one chunk for all the pick paths in the list, instead
     * of allocating a seperate buffer for each pick path.
     */

    if (pickPaths == PickCache)
    {
	/*
	 * Make the pick cache available again.
	 */

	PickCacheInUse = 0;
    }
    else if (PickCacheInUse)
    {
	/*
	 * The pick cache is in use, so we must free this pick path.
	 */

	PEXFreeBuf ((char *) pickPaths);
    }
    else
    {
	/*
	 * Calculate the size of the pick path being freed.
	 */

	total_size = numPickPaths * sizeof (PEXPickPath);
	for (i = 0; i < numPickPaths; i++)
	    total_size += (pickPaths[i].count * sizeof (PEXPickElementRef));


	/*
	 * If the size is smaller than the pick cache size or bigger than
	 * the max size, free the pick path.  Otherwise, make this path the
	 * new pick cache buffer.
	 */
	
	if (total_size <= PickCacheSize || total_size > MAX_PICK_CACHE_SIZE)
	    PEXFreeBuf ((char *) pickPaths);
	else
	{
	    if (PickCache)
		PEXFreeBuf ((char *) PickCache);
	    PickCache = pickPaths;
	    PickCacheSize = total_size;
	}
    }
}


void PEXFreeRendererAttributes (rdrAttr)

INPUT PEXRendererAttributes	*rdrAttr;

{
    FreeIfNotNull ((char *) rdrAttr->current_path.elements);
    FreeIfNotNull ((char *) rdrAttr->clip_list.rectangles);
    FreeIfNotNull ((char *) rdrAttr->pick_start_path.elements);

    PEXFreeBuf ((char *) rdrAttr);
}


void PEXFreeSCAttributes (scAttr)

PEXSCAttributes		*scAttr;

{
    FreeIfNotNull ((char *) scAttr->start_path.elements);
    FreeIfNotNull ((char *) scAttr->normal.pairs);
    FreeIfNotNull ((char *) scAttr->inverted.pairs);

    PEXFreeBuf ((char *) scAttr);
}


void PEXFreeStructurePaths (numPaths, paths)

INPUT unsigned long	numPaths;
INPUT PEXStructurePath	*paths;

{
    int i;


    for (i = 0; i < numPaths; i++)
	PEXFreeBuf ((char *) paths[i].elements);

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
	    PEXFreeBuf ((char *) entries[i].colors);
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
    FreeIfNotNull ((char *) wksAttr->posted_structures.structures);

    PEXFreeBuf ((char *) wksAttr);
}
