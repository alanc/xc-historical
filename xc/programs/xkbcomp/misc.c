/* $XConsortium: misc.c,v 1.4 93/09/28 20:16:45 rws Exp $ */
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
#include "misc.h"

/***====================================================================***/

char *
configText(config)
    unsigned config;
{
static char buf[32];

    switch (config) {
	case XkmSemanticsFile:
	    strcpy(buf,"semantics");
	    break;
	case XkmLayoutFile:
	    strcpy(buf,"layout");
	    break;
	case XkmKeymapFile:
	    strcpy(buf,"keymap");
	    break;
	case XkmGeometryFile:
	case XkmGeometryIndex:
	    strcpy(buf,"geometry");
	    break;
	case XkmAlternateSymsFile:
	    strcpy(buf,"alternate symbols");
	    break;
	case XkmTypesIndex:
	    strcpy(buf,"types");
	    break;
	case XkmCompatMapIndex:
	    strcpy(buf,"compatibility map");
	    break;
	case XkmSymbolsIndex:
	    strcpy(buf,"symbols");
	    break;
	case XkmIndicatorsIndex:
	    strcpy(buf,"indicators");
	    break;
	case XkmKeyNamesIndex:
	    strcpy(buf,"key names");
	    break;
	case XkmVirtualModsIndex:
	    strcpy(buf,"virtual modifiers");
	    break;
	default:
	    sprintf(buf,"unknown(%d)",config);
	    break;
    }
    return buf;
}

char *
keysymText(sym,format)
    KeySym	sym;
    unsigned	format;
{
static char buf[32],*rtrn;

    if (sym==NoSymbol)
	strcpy(rtrn=buf,"NoSymbol");
    else if ((rtrn=XKeysymToString(sym))==NULL)
	sprintf(rtrn=buf,"0x%x",sym);
    else if (format==XkbCHeaderFile) {
	sprintf(buf,"XK_%s",rtrn);
	rtrn= buf;
    }
    return rtrn;
}

static char *modNames[XkbNumModifiers] = {
    "Shift", "Lock", "Control", "Mod1", "Mod2", "Mod3", "Mod4", "Mod5"
};

char *
modIndexText(ndx,format)
    unsigned	ndx;
    unsigned	format;
{
static char buf[16];

    if (ndx<XkbNumModifiers) {
	if (format==XkbCHeaderFile) {
	    sprintf(buf,"%sMapIndex",modNames[ndx]);
	    return buf;
	}
	return modNames[ndx];
    }
    strcpy(buf,"ILLEGAL");
    return buf;
}

char *
modMaskText(mask,format)
    unsigned	mask;
    unsigned	format;
{
register int i,bit;
static char buf[64];

    if ((mask&0xff)==0xff) {
	if (format==XkbCHeaderFile) 	strcpy(buf,"0xff");
	else				strcpy(buf,"all");
    }
    else if ((mask&0xff)==0) {
	if (format==XkbCHeaderFile)	strcpy(buf,"0");
	else				strcpy(buf,"none");
    }
    else {
	char *str= buf;
	buf[0]= '\0';
	for (i=0,bit=1;i<XkbNumModifiers;i++,bit<<=1) {
	    if (mask&bit) {
		if (str!=buf) {
		    if (format==XkbCHeaderFile)	*str++= '|';
		    else			*str++= '+';
		}
		strcpy(str,modNames[i]);
		str= &str[strlen(str)];
		if (format==XkbCHeaderFile) {
		    strcpy(str,"Mask");
		    str+= 4;
		}
	    }
	}
    }
    return buf;
}

static char *siMatchText[5] = {
	"NoneOf", "AnyOfOrNone", "AnyOf", "AllOf", "Exactly"
};

char *
SIMatchText(type,format)
    unsigned	type;
    unsigned	format;
{
static char buf[40];
char *rtrn;

    switch (type&XkbSI_OpMask) {
	case XkbSI_NoneOf:	rtrn= siMatchText[0]; break;
	case XkbSI_AnyOfOrNone:	rtrn= siMatchText[1]; break;
	case XkbSI_AnyOf:	rtrn= siMatchText[2]; break;
	case XkbSI_AllOf:	rtrn= siMatchText[3]; break;
	case XkbSI_Exactly:	rtrn= siMatchText[4]; break;
	default:		sprintf(buf,"0x%x",type&XkbSI_OpMask);
				return buf;
    }
    if (format==XkbCHeaderFile) {
	if (type&XkbSI_LevelOneOnly)
	     sprintf(buf,"XkbSI_LevelOneOnly|XkbSI_%s",rtrn);
	else sprintf(buf,"XkbSI_%s",rtrn);
	rtrn= buf;
    }
    return rtrn;
}

static char *actionTypeNames[XkbSA_NumActions]= {
    "NoAction", 
    "SetMods",      "LatchMods",    "LockMods", 
    "SetGroup",     "LatchGroup",   "LockGroup",
    "MovePtr",      "AccelPtr",
    "PtrBtn",       "ClickPtrBtn",  "LockPtrBtn",
    "SetPtrDflt",
    "ISOLock",
    "Terminate",    "SwitchScreen", 
    "SetControls",  "LockControls",
    "ActionMessage"
};

char *
actionTypeText(type,format)
    unsigned	type;
    unsigned	format;
{
static char buf[20];
char *rtrn;

    if (type<=XkbSA_LastAction) {
	rtrn= actionTypeNames[type];
	if (format==XkbCHeaderFile) {
	    sprintf(buf,"XkbSA_%s",rtrn);
	    return buf;
	}
	return rtrn;
    }
    sprintf(buf,"0x%x",type);
    return buf;
}

/***====================================================================***/

char *
DirectoryForInclude(type)
    unsigned	type;
{
static char buf[32];

    switch (type) {
	case XkmSemanticsFile:
	    strcpy(buf,"semantics");
	    break;
	case XkmLayoutFile:
	    strcpy(buf,"layout");
	    break;
	case XkmKeymapFile:
	    strcpy(buf,"keymap");
	    break;
	case XkmKeyNamesIndex:
	    strcpy(buf,"keycodes");
	    break;
	case XkmTypesIndex:
	    strcpy(buf,"types");
	    break;
	case XkmAlternateSymsFile:
	case XkmSymbolsIndex:
	    strcpy(buf,"symbols");
	    break;
	case XkmCompatMapIndex:
	    strcpy(buf,"compat");
	    break;
	case XkmGeometryFile:
	case XkmGeometryIndex:
	    strcpy(buf,"geometry");
	    break;
	default:
	    strcpy(buf,"");
	    break;
    }
    return buf;
}

/***====================================================================***/

Bool
ProcessIncludeFile(stmt,file_type,file_rtrn,merge_rtrn)
    IncludeStmt	*	stmt;
    unsigned		file_type;
    XkbFile **		file_rtrn;
    unsigned *		merge_rtrn;
{
extern int lineNum;
unsigned newMerge;
FILE	*file;
XkbFile	*rtrn;
char	 oldFile[1024];
int	 oldLine;

    switch (stmt->merge) {
	case MergeDefault:
	    newMerge= MergeAugment;
	    break;
	case MergeReplace:
	case MergeAugment:
	case MergeOverride:
	    newMerge= stmt->merge;
	    break;
	default:
	    uInternalError("Unknown merge mode in ProcessIncludeFile\n",
								stmt->merge);
	    return False;
    }	
    file= FindFileInPath(stmt->file,DirectoryForInclude(file_type),NULL);
    if (file==NULL) {
	uError("Can't find file \"%s\" for include\n",stmt->file);
	uAction("Exiting\n");
	return False;
    }
    strcpy(oldFile,scanFile);
    oldLine= lineNum;
    setScanState(stmt->file,1);
    if (debugFlags&2)
	uInformation("About to parse include file %s\n",stmt->file);
    if ((XKBParseFile(file,&rtrn)==0)||(rtrn==NULL)) {
	setScanState(oldFile,oldLine);
	uError("Error interpreting include file \"%s\"\n",stmt->file);
	uAction("Exiting\n");
	fclose(file);
	return False;
    }
    fclose(file);
    setScanState(oldFile,oldLine);
    if (rtrn->type!=file_type) {
	uError("Include file wrong type (expected %s,",configText(file_type));
	uInformation(" got %s)\n",configText(rtrn->type));
	uAction("Include file \"%s\" ignored\n",stmt->file);
	return False;
    }
    *file_rtrn= rtrn;
    *merge_rtrn= newMerge;
    return True;
}

