/* $XConsortium: pl_global_def.h,v 1.4 92/05/07 23:31:07 mor Exp $ */

/************************************************************************
Copyright 1987,1991,1992 by Digital Equipment Corporation, Maynard,
Massachusetts, and the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*************************************************************************/

PEXDisplayInfo *PEXDisplayInfoHeader = NULL;

char	*pexErrorList[(PEXMaxError + 1)] = {
/* ColourType */
    "PEXColourTypeError, specified colour type not supported",
/* RendererState */
    "PEXRendererStateError, renderer not in proper state for operation",
/* FloatingPointFormat */
    "PEXFloatingPointFormatError, specified fp format not supported",
/* Label */
    "PEXLabelError, specified label not in structure",
/* LookupTable */
    "LookupTableError, specified lookup table ID invalid",
/* NameSet */
    "PEXNameSetError, specified nameset ID invalid",
/* Path */
    "PEXPathError, illegal or invalid values in path",
/* PEXFont */
    "PEXFontError, specified font ID invalid",
/* PhigsWKS */
    "PhigsWKSError, specified PHIGS workstation ID invalid", 
/* PickMeasure */
    "PEXPickMeasureError, specified pick measure ID invalid",
/* PipelineContext */
    "PEXPipelineContextError, specified pipeline context ID invalid",
/* Renderer */
    "PEXRendererError, specified renderer ID invalid",
/* SearchContext */
    "PEXSearchContextError, specified search context ID invalid",
/* Structure */
    "PEXStructureError, specified structure ID invalid",
/* OutputCommand */
    "PEXOutputCommandError, illegal value in output commands",
};
