/* $XConsortium: xkmout.c,v 1.4 93/09/28 20:16:45 rws Exp $ */
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#define	DEBUG_VAR_NOT_LOCAL
#define	DEBUG_VAR debugFlags
#include "xkbcomp.h"
#include "tokens.h"

#include <X11/extensions/XKB.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKMformat.h>


typedef struct _XkmInfo {
     unsigned short	bound_vmods;
     unsigned short	named_vmods;
     unsigned char	num_bound;
     unsigned char	compat_mods;
     unsigned short	compat_vmods;
} XkmInfo;

/***====================================================================***/

#define	xkmPutCARD8(f,v)	(putc(v,f),1)

int
xkmPutCARD16(file,val)
    FILE *		file;
    unsigned		val;
{
    putc(((val>>8)&0xff),file);
    putc((val&0xff),file);
    return 2;
}

int
xkmPutCARD32(file,val)
    FILE *		file;
    unsigned long	val;
{
    putc(((val>>24)&0xff),file);
    putc(((val>>16)&0xff),file);
    putc(((val>>8)&0xff),file);
    putc((val&0xff),file);
    return 4;
}

int
xkmPutPadding(file,pad)
    FILE *		file;
    unsigned		pad;
{
int	i;
    for (i=0;i<pad;i++) {
	putc('\0',file);
    }
    return pad;
}

int
xkmPutCountedBytes(file,ptr,count)
    FILE *		file;
    char *		ptr;
    unsigned		count;
{
register int nOut,pad;
    if (count==0)
	return xkmPutCARD32(file,0);

    xkmPutCARD16(file,count);
    nOut= fwrite(ptr,1,count,file);
    if (nOut<0)
	return 2;
    nOut= count+2;
    pad= XkbPaddedSize(nOut)-nOut;
    if (pad)
	xkmPutPadding(file,pad);
    return nOut+pad;
}

unsigned
xkmSizeCountedString(str)
    char *	str;
{
    if (str==NULL)
	return 4;
    return XkbPaddedSize(strlen(str)+2);
}

int
xkmPutCountedString(file,str)
    FILE *	file;
    char *	str;
{
    if (str==NULL)
	 return xkmPutCARD32(file,0);
    return xkmPutCountedBytes(file,str,strlen(str));
}

/***====================================================================***/

static unsigned
SizeXKMVirtualMods(result,info)
    XkbFileResult *	result;
    XkmInfo *		info;
{
XkbDescPtr	xkb;
unsigned	nBound,bound;
unsigned	nNamed,named,szNames;
register unsigned	i,bit;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->names)||(!xkb->server)) {
	uInternalError("Vmod mod bindings or names missing in SizeXKMVirtualMods\n");
	return 0;
    }
    bound=named=0;
    for (i=nBound=nNamed=szNames=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	if (xkb->server->vmods[i]!=XkbNoModifier) {
	    bound|= bit;
	    nBound++;
	}
	if (xkb->names->vmods[i]!=(Atom)NullStringToken) {
	    register char *name;
	    named|= bit;
	    name= stGetString((StringToken)xkb->names->vmods[i]);
	    szNames+= xkmSizeCountedString(name);
	    nNamed++;
	}
    }
    info->num_bound= nBound;
    info->bound_vmods= bound;
    info->named_vmods= named;
    if ((nBound==0)&&(nNamed==0))
	return 0;
    return 4+XkbPaddedSize(nBound)+szNames;
}

static Bool
WriteXKMVirtualMods(file,result,info)
    FILE *		file;
    XkbFileResult *	result;
    XkmInfo *		info;
{
register unsigned int i,bit;
XkbDescPtr	xkb;

    xkb= &result->xkb;
    xkmPutCARD16(file,info->bound_vmods);
    xkmPutCARD16(file,info->named_vmods);
    for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	if (info->bound_vmods&bit)
	    xkmPutCARD8(file,xkb->server->vmods[i]);
    }
    if ((i= XkbPaddedSize(info->num_bound)-info->num_bound)>0)
	xkmPutPadding(file,i);
    for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	if (info->named_vmods&bit) {
	    register char *name;
	    name= stGetString((StringToken)xkb->names->vmods[i]);
	    xkmPutCountedString(file,name);
	}
    }
    return True;
}

/***====================================================================***/

static unsigned
SizeXKMKeycodes(result,info)
    XkbFileResult *	result;
    XkmInfo *		info;
{
XkbDescPtr	xkb;
StringToken	kcName;
int	size;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->names)||(!xkb->names->keys)) {
	uInternalError("Key names not defined in SizeXKMKeycodes\n");
	return 0;
    }
    kcName= (StringToken)xkb->names->keycodes;
    size= 4;	/* min and max keycode */
    size+= xkmSizeCountedString(stGetString(kcName));
    size+= XkbNumKeys(xkb)*sizeof(XkbKeyNameRec);
    return size;
}

static Bool
WriteXKMKeycodes(file,result,info)
    FILE *		file;
    XkbFileResult *	result;
    XkmInfo *		info;
{
XkbDescPtr	xkb;
StringToken	 kcName;
char 		*start;
register int	 i;

    xkb= &result->xkb;
    kcName= (StringToken)xkb->names->keycodes;
    start= xkb->names->keys[xkb->min_key_code].name;
    
    xkmPutCARD8(file,xkb->min_key_code);
    xkmPutCARD8(file,xkb->max_key_code);
    xkmPutPadding(file,2);
    xkmPutCountedString(file,stGetString(kcName));
    fwrite(start,sizeof(XkbKeyNameRec),XkbNumKeys(xkb),file);
    return True;
}

/***====================================================================***/

static unsigned
SizeXKMKeyTypes(result,info)
    XkbFileResult *	result;
    XkmInfo *		info;
{
register int i,n,size;
XkbKeyTypePtr	type;
XkbDescPtr	xkb;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->map)||(!xkb->map->types)) {
	uInternalError("Key types not defined in SizeXKBKeyTypes\n");
	return 0;
    }
    size= 4;	/* room for # of key types + padding */
    for (i=0,type=xkb->map->types;i<xkb->map->num_types;i++,type++) {
	size+= SIZEOF(xkmKeyTypeDesc);
	size+= SIZEOF(xkmKTMapEntryDesc)*type->map_count;
	if (type->preserve)
	    size+= SIZEOF(xkmKTPreserveDesc)*type->map_count;
	if (type->lvl_names) {
	    StringToken *names;
	    names= (StringToken *)type->lvl_names;
	    for (n=0;n<type->group_width;n++) {
		size+= xkmSizeCountedString(stGetString(names[n]));
	    }
	}
    }
    return size;
}

static Bool
WriteXKMKeyTypes(file,result,info)
    FILE *	file;
    XkbFileResult *	result;
    XkmInfo *	info;
{
register int		i,n;
XkbDescPtr	xkb;
XkbKeyTypePtr		type;
xkmKeyTypeDesc		wire;
XkbKTMapEntryPtr	entry;
xkmKTMapEntryDesc	wire_entry;
StringToken *		names;

    xkb= &result->xkb;
    xkmPutCARD16(file,xkb->map->num_types);
    xkmPutPadding(file,2);
    type= xkb->map->types;
    for (i=0;i<xkb->map->num_types;i++,type++) {
	wire.realMods= type->real_mods;
	wire.groupWidth= type->group_width;
	wire.virtualMods= type->vmods;
	wire.nMapEntries= type->map_count;
	wire.preserve= (type->preserve!=NULL);
	if (type->lvl_names!=NULL)
	     wire.nLevelNames= type->group_width;
	else wire.nLevelNames= 0;
	fwrite(&wire,SIZEOF(xkmKeyTypeDesc),1,file);
	for (n=0,entry= type->map;n<type->map_count;n++,entry++) {
	    wire_entry.level= entry->level;
	    wire_entry.realMods= entry->real_mods;
	    wire_entry.virtualMods= entry->vmods;
	    fwrite(&wire_entry,SIZEOF(xkmKTMapEntryDesc),1,file);
	}
	if (type->preserve) {
	    xkmKTPreserveDesc	p_entry;
	    XkbKTPreservePtr	pre;
	    for (n=0,pre=type->preserve;n<type->map_count;n++,pre++) {
		p_entry.realMods= pre->real_mods;
		p_entry.virtualMods= pre->vmods;
		fwrite(&wire_entry,SIZEOF(xkmKTPreserveDesc),1,file);
	    }
	}
	if (type->lvl_names!=NULL) {
	    names= type->lvl_names;
	    for (n=0;n<wire.nLevelNames;n++) {
		xkmPutCountedString(file,stGetString(names[n]));
	    }
	}
    }
    return True;
}

/***====================================================================***/

static unsigned
SizeXKMCompatMap(result,info)
    XkbFileResult *	result;
    XkmInfo *		info;
{
XkbDescPtr	xkb;
char	*name;
int	size;
register int	i;
unsigned mods,nMods;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->compat)||(!xkb->compat->sym_interpret)) {
	uInternalError("Compat map not defined in SizeXKMCompatMap\n");
	return 0;
    }
    if (xkb->names)	name= stGetString((StringToken)xkb->names->semantics);
    else		name= NULL;

    nMods= 0;
    for (i=mods=0;i<XkbNumModifiers;i++) {
	if ((xkb->compat->real_mod_compat[i].groups!=0)||
			(xkb->compat->real_mod_compat[i].mods!=(1<<i))) {
	    mods|= (1<<i);
	    nMods++;
	}
    }
    info->compat_mods= mods;
    for (i=mods=0;i<XkbNumVirtualMods;i++) {
	if ((xkb->compat->vmod_compat[i].groups!=0)||
				(xkb->compat->vmod_compat[i].mods!=0)) {
	    mods|= (1<<i);
	    nMods++;
	}
    }
    info->compat_vmods= mods;
    size= 4+xkmSizeCountedString(name);
    size+= (SIZEOF(xkmSymInterpretDesc)*xkb->compat->num_si);
    size+= XkbPaddedSize(2*nMods);
    return size;
}

static Bool
WriteXKMCompatMap(file,result,info)
    FILE *	file;
    XkbFileResult *	result;
    XkmInfo *	info;
{
register int		i;
char *			name;
XkbDescPtr		xkb;
XkbSymInterpretPtr	interp;
xkmSymInterpretDesc	wire;

    xkb= &result->xkb;
    if (xkb->names)	name= stGetString((StringToken)xkb->names->semantics);
    else		name= NULL;
    xkmPutCARD16(file,xkb->compat->num_si);
    xkmPutPadding(file,2);
    xkmPutCountedString(file,name);
    interp= xkb->compat->sym_interpret;
    for (i=0;i<xkb->compat->num_si;i++,interp++) {
	wire.sym= interp->sym;
	wire.mods= interp->mods;
	wire.match= interp->match;
	wire.virtualMod= interp->virtual_mod;
	wire.flags= interp->flags;
	wire.actionType= interp->act.type;
	wire.actionData[0]= interp->act.data[0];
	wire.actionData[1]= interp->act.data[1];
	wire.actionData[2]= interp->act.data[2];
	wire.actionData[3]= interp->act.data[3];
	wire.actionData[4]= interp->act.data[4];
	wire.actionData[5]= interp->act.data[5];
	wire.actionData[6]= interp->act.data[6];
	fwrite(&wire,SIZEOF(xkmSymInterpretDesc),1,file);
    }
    xkmPutCARD8(file,info->compat_mods);
    xkmPutPadding(file,1);
    xkmPutCARD16(file,info->compat_vmods);
    if (info->compat_mods) {
	register unsigned bit;
	for (i=0,bit=1;i<XkbNumModifiers;i++,bit<<=1) {
	    if (info->compat_mods&bit) {
		xkmPutCARD8(file,xkb->compat->real_mod_compat[i].mods);
		xkmPutCARD8(file,xkb->compat->real_mod_compat[i].groups);
	    }
	}
    }
    if (info->compat_vmods) {
	register unsigned bit;
	for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	    if (info->compat_vmods&bit) {
		xkmPutCARD8(file,xkb->compat->vmod_compat[i].mods);
		xkmPutCARD8(file,xkb->compat->vmod_compat[i].groups);
	    }
	}
    }
    return True;
}

/***====================================================================***/

static unsigned
SizeXKMSymbols(result,info)
    XkbFileResult *	result;
    XkmInfo *		info;
{
XkbDescPtr	xkb;
unsigned 	size;
register int	i,nSyms;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->map)||((!xkb->map->syms))) {
	uInternalError("Symbols not defined in SizeXKMSymbols\n");
	return 0;
    }
    size= 4;	/* min and max keycode */
    for (i=xkb->min_key_code;i<=(int)xkb->max_key_code;i++) {
	nSyms= XkbKeyNumSyms(xkb,i);
	size+= SIZEOF(xkmKeySymMapDesc)+(nSyms*4);
	if (xkb->server) {
	    if (xkb->server->explicit[i]&XkbExplicitKeyTypeMask) {
		char *name;
		name= stGetString((StringToken)XkbKeyKeyType(xkb,i)->name);
		if (name!=NULL)
		     size+= xkmSizeCountedString(name);
	    }
	    if (XkbKeyHasActions(xkb,i))
		size+= nSyms*SIZEOF(xkmActionDesc);
	    if (xkb->server->behaviors[i].type!=XkbKB_Default)
		size+= SIZEOF(xkmBehaviorDesc);
	}
    }
    return size;
}

static Bool
WriteXKMSymbols(file,result,info)
    FILE *		file;
    XkbFileResult *	result;
    XkmInfo *		info;
{
XkbDescPtr		xkb;
register int		i,n;
xkmKeySymMapDesc 	wireMap;

    xkb= &result->xkb;
    xkmPutCARD8(file,xkb->min_key_code);
    xkmPutCARD8(file,xkb->max_key_code);
    for (i=xkb->min_key_code;i<(int)xkb->max_key_code;i++)  {
	register char *typeName;
	wireMap.group_width= XkbKeyGroupWidth(xkb,i);
	wireMap.num_groups= XkbKeyNumGroups(xkb,i);
	if (result->defined&ModMapDefined)
	     wireMap.modifier_map= result->modmap[i];
	else wireMap.modifier_map= 0;
	wireMap.flags= 0;
	typeName= NULL;
	if (xkb->server) {
	    if (xkb->server->explicit[i]&XkbExplicitKeyTypeMask) {
		typeName= stGetString((StringToken)XkbKeyKeyType(xkb,i)->name);
		if (typeName!=NULL)
		    wireMap.flags|= XkmKeyHasType;
	    }
	    if (XkbKeyHasActions(xkb,i))
		wireMap.flags|= XkmKeyHasActions;
	    if (xkb->server->behaviors[i].type!=XkbKB_Default)
		wireMap.flags|= XkmKeyHasBehavior;
	    if ((result->defined&PerKeyRepeatDefined)&&
		(xkb->server->explicit[i]&XkbExplicitAutorepeatMask)) {
		if (result->repeat[(i/8)]&(1<<(i%8)))
		     wireMap.flags|= XkmRepeatingKey;
		else wireMap.flags|= XkmNonRepeatingKey;
	    }
	}
	fwrite(&wireMap,SIZEOF(xkmKeySymMapDesc),1,file);
	if (typeName!=NULL)
	    xkmPutCountedString(file,typeName);
	if (wireMap.num_groups>0) {
	    KeySym	*sym;
	    sym= XkbKeySymsPtr(xkb,i);
	    for (n=XkbKeyNumSyms(xkb,i);n>0;n--,sym++) {
		xkmPutCARD32(file,(CARD32)*sym);
	    }
	    if (wireMap.flags&XkmKeyHasActions) {
		XkbAction *	act;
		act= XkbKeyActionsPtr(xkb,i);
		for (n=XkbKeyNumActions(xkb,i);n>0;n--,act++) {
		    fwrite(&act,SIZEOF(xkmActionDesc),1,file);
		}
	    }
	}
	if (wireMap.flags&XkmKeyHasBehavior) {
	    xkmBehaviorDesc	b;
	    b.type= xkb->server->behaviors[i].type;
	    b.data= xkb->server->behaviors[i].data;
	    fwrite(&b,SIZEOF(xkmBehaviorDesc),1,file);
	}
    }
    return True;
}

/***====================================================================***/

static int
GetXKMSemanticsTOC(result,info,max_toc,toc_rtrn)
    XkbFileResult *	result;
    XkmInfo *		info;
    int			max_toc;
    xkmSectionInfo *	toc_rtrn;
{
int	num_toc;
int	total_size;

    total_size= num_toc=0;
    toc_rtrn[num_toc].size= SizeXKMVirtualMods(result,info);
    if (toc_rtrn[num_toc].size>0) {
	toc_rtrn[num_toc].type= XkmVirtualModsIndex;
	toc_rtrn[num_toc].format= MSBFirst;
	toc_rtrn[num_toc].size+= SIZEOF(xkmSectionInfo);
	toc_rtrn[num_toc].offset= total_size;
	total_size+= toc_rtrn[num_toc].size;
	num_toc++;
    }
    toc_rtrn[num_toc].size= SizeXKMKeyTypes(result,info);
    if (toc_rtrn[num_toc].size>0) {
	toc_rtrn[num_toc].type= XkmTypesIndex;
	toc_rtrn[num_toc].format= MSBFirst;
	toc_rtrn[num_toc].size+= SIZEOF(xkmSectionInfo);
	toc_rtrn[num_toc].offset= total_size;
	total_size+= toc_rtrn[num_toc].size;
	num_toc++;
    }
    toc_rtrn[num_toc].size= SizeXKMCompatMap(result,info);
    if (toc_rtrn[num_toc].size>0) {
	toc_rtrn[num_toc].type= XkmCompatMapIndex;
	toc_rtrn[num_toc].format= MSBFirst;
	toc_rtrn[num_toc].size+= SIZEOF(xkmSectionInfo);
	toc_rtrn[num_toc].offset= total_size;
	total_size+= toc_rtrn[num_toc].size;
	num_toc++;
    }
    return num_toc;
}

static int
GetXKMLayoutTOC(result,info,max_toc,toc_rtrn)
    XkbFileResult *	result;
    XkmInfo *		info;
    int			max_toc;
    xkmSectionInfo *	toc_rtrn;
{
    return 0;
}

static int
GetXKMKeymapTOC(result,info,max_toc,toc_rtrn)
    XkbFileResult *	result;
    XkmInfo *		info;
    int			max_toc;
    xkmSectionInfo *	toc_rtrn;
{
    return 0;
}

static Bool
WriteXKMFile(file,result,num_toc,toc,info)
    FILE *		file;
    XkbFileResult *	result;
    int			num_toc;
    xkmSectionInfo *	toc;
    XkmInfo *		info;
{
register int i,size;
Bool	ok= True;
    
    for (i=0;(i<num_toc)&&(ok);i++) {
	fwrite(&toc[i],SIZEOF(xkmSectionInfo),1,file);
	switch (toc[i].type) {
	    case XkmVirtualModsIndex:
		ok= WriteXKMVirtualMods(file,result,info);
		break;
	    case XkmTypesIndex:
		ok= WriteXKMKeyTypes(file,result,info);
		break;
	    case XkmCompatMapIndex:
		ok= WriteXKMCompatMap(file,result,info);
		break;
	    default:
		uInternalError("Bogus toc type %d in WriteXKMSemantics\n",
								toc[i].type);
		break;
	}
    }
    return ok;
}


#define	MAX_TOC	16

Bool
WriteCompiledKbdDesc(name,type,result)
    char *		name;
    unsigned		type;
    XkbFileResult *	result;
{
FILE	*		out;
Bool	 		ok;
XkbDescPtr		xkb;
XkmInfo			info;
Bool			(*writer)();
int			(*getTOC)();
int			size_toc,i;
unsigned		hdr;
xkmSectionInfo		toc[MAX_TOC];

    switch (type) {
	case XkmSemanticsFile:
	    writer= WriteXKMFile;
	    getTOC= GetXKMSemanticsTOC;
	    break;
	case XkmLayoutFile:
	    writer= WriteXKMFile;
	    getTOC= GetXKMLayoutTOC;
	    break;
	case XkmKeymapFile:
	    writer= WriteXKMFile;
	    getTOC= GetXKMKeymapTOC;
	    break;
	default:
	    uError("Cannot create an XKM file from a %s description\n",
							configText(type));
	    uAction("You must specify a semantics, layout or keymap file\n");
	    return False;
    }
    xkb= &result->xkb;

    bzero(&info,sizeof(XkmInfo));
    size_toc= (*getTOC)(result,&info,MAX_TOC,toc);
    if (size_toc<1) {
	uInternalError("Nothing to compile in WriteCompiledKbdDesc!!\n");
	return False;
    }
    out= fopen(name,"w");
    if (out==NULL) {
	uError("Couldn't open output file \"%s\"\n",name);
	uAction("Exiting\n");
	return False;
    }
    for (i=0;i<size_toc;i++) {
	toc[i].offset+= (8+(size_toc*SIZEOF(xkmSectionInfo)));;
    }
    hdr= (('x'<<24)|('k'<<16)|('m'<<8)|'0');
    xkmPutCARD32(out,hdr);
    xkmPutCARD8(out,type);
    xkmPutCARD8(out,size_toc);
    xkmPutCARD16(out,toc[size_toc-1].offset+toc[size_toc-1].size);
    fwrite(toc,SIZEOF(xkmSectionInfo),size_toc,out);
    ok= (*writer)(out,result,size_toc,toc,&info);
    fclose(out);
    return True;
}
