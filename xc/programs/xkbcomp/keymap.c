/* $XConsortium: keymap.c,v 1.4 93/09/28 20:16:45 rws Exp $ */
/************************************************************
 Copyright (c) 1994 by Silicon Graphics Computer Systems, Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of Silicon Graphics not be 
 used in advertising or publicity pertaining to distribution 
 of the software without specific prior written permission.
 Silicon Graphics makes no representation about the suitability 
 of this software for any purpose. It is provided "as is"
 without any express or implied warranty.
 
 SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
 SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 THE USE OR PERFORMANCE OF THIS SOFTWARE.

 ********************************************************/

#include "xkbcomp.h"
#include "xkbio.h"
#include "tokens.h"
#include "expr.h"
#include "vmod.h"
#include "action.h"

Bool
CompileKeymap(file,result,merge)
    XkbFile	*	file;
    XkbFileResult *	result;
    unsigned	 	merge;
{
unsigned	have;
Bool		ok;
unsigned	required,optional,legal;
unsigned	mainType;

    mainType= file->type;
    switch (mainType) {
	case XkmSemanticsFile:
	    required= XkmSemanticsRequired;
	    optional= XkmSemanticsOptional;
	    legal= XkmSemanticsLegal;
	    break;
	case XkmLayoutFile:
	    required= XkmLayoutRequired;
	    optional= XkmLayoutOptional;
	    legal= XkmKeymapLegal;
	    break;
	case XkmKeymapFile:
	    required= XkmKeymapRequired;
	    optional= XkmKeymapOptional;
	    legal= XkmKeymapLegal;
	    break;
	default:
	    uError("Cannot compile %s alone into an XKM file\n",
						configText(mainType));
	    return False;
    }
    have= 0;
    ok= 1;
    file= (XkbFile *)file->common.next;
    while ((file)&&(ok)) {
	if ((have&(1<<file->type))!=0) {
	    uError("More than one %s section in a %s file\n",
					configText(file->type),
					configText(mainType));
	    uAction("All sections after the first ignored\n");
	    ok= False;
	}
	else if ((1<<file->type)&(~legal)) {
	    uError("Cannot define %s in a ",configText(file->type));
	    uInformation("%s file\n",configText(mainType));
	    ok= False;
	}
	else switch (file->type) {
	    case XkmSemanticsFile:
	    case XkmLayoutFile:
	    case XkmKeymapFile:
		uInternalError("Illegal %s file in a ",configText(file->type));
		uInformation("%s file\n",configText(mainType));
		uAction("Ignored\n");
		ok= False;
		break;
	    case XkmKeyNamesIndex:
		ok= CompileKeycodes(file,result,MergeReplace);
		    
		break;
	    case XkmTypesIndex:
		ok= CompileKeyTypes(file,result,MergeReplace);
		break;
	    case XkmSymbolsIndex:
		ok= CompileSymbols(file,result,MergeReplace,True);
		break;
	    case XkmCompatMapIndex:
		ok= CompileCompatMap(file,result,MergeReplace);
		break;
	    case XkmGeometryIndex:
	    case XkmGeometryFile:
		uInternalError("Geometry file not supported yet\n");
		break;
	    case XkmVirtualModsIndex:
	    case XkmIndicatorsIndex:
		uInternalError("Found an isolated %s section\n",
						configText(file->type));
		break;
	    case XkmAlternateSymsFile:
		uError("Keymap file contains alternate symbol map\n");
		uAction("Illegal component ignored\n");
		ok= False;
		break;
	    default:
		uInternalError("Unknown file type %d\n",file->type);
		break;
	}
	if (ok)
	    have|= (1<<file->type);
	file= (XkbFile*)file->common.next;
    }
    result->configs= have;
    if (required&(~have)) {
	register int i,bit;
	unsigned missing;
	missing= required&(~have);
	for (i=0,bit=1;missing!=0;i++,bit<<=1) {
	    if (missing&bit) {
		uError("Missing %s section in ",configText(i));
		uInformation("%s file\n",configText(mainType));
		missing&=~bit;
	    }
	}
	uAction("Description of %s not compiled\n",configText(mainType));
	ok= False;
    }
    return ok;
}

