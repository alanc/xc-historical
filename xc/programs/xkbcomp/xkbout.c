/* $XConsortium: xkbout.c,v 1.2 94/04/04 15:28:43 rws Exp $ */
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


#define	DEBUG_VAR_NOT_LOCAL
#define	DEBUG_VAR debugFlags
#include "xkbcomp.h"
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif
#include "misc.h"
#include "tokens.h"

#include <X11/extensions/XKB.h>
#include <X11/XKBlib.h>
#include "expr.h"
#include "vmod.h"

static char *
VModText(xkb,vmod)
    XkbDescPtr	xkb;
    unsigned	vmod;
{
static char buf[8];

    if (vmod>=XkbNumVirtualMods) {
	sprintf(buf,"ILLEGAL");
    }
    else if ((xkb!=NULL)&&(xkb->names!=NULL)&&
	(((StringToken)xkb->names->vmods[vmod])!=NullStringToken)) {
	return stGetString(((StringToken)xkb->names->vmods[vmod]));
    }
    sprintf(buf,"%d",vmod);
    return buf;
}

static Bool
WriteXKBVModDecl(file,xkb)
    FILE *	file;
    XkbDescPtr	xkb;
{
register int 	i,nMods;
StringToken *	vmodNames;
unsigned char *	vmodDefs;

    if (xkb==NULL)
	return False;
    if (xkb->names!=NULL)
	 vmodNames= (StringToken *)xkb->names->vmods;
    else vmodNames= NULL;
    if (xkb->server!=NULL)
	 vmodDefs= xkb->server->vmods;
    else vmodDefs= NULL;

    for (i=nMods=0;i<XkbNumVirtualMods;i++) {
	if ((vmodNames!=NULL)&&(vmodNames[i]!=NullStringToken)) {
	    if (nMods==0)	fprintf(file,"virtual_modifiers ");
	    else		fprintf(file,",");
	    fprintf(file,"%s",stGetString(vmodNames[i]));
	    if ((vmodDefs!=NULL)&&(vmodDefs[i]!=XkbNoModifier))
		fprintf(file,"=%s",modIndexText(vmodDefs[i],XkbXKBFile));
	    nMods++;
	}
    }
    if (nMods>0)
	fprintf(file,";\n\n");
}

/***====================================================================***/

static Bool
PrintNoActionArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
    return True;
}

static Bool
PrintModActionArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
XkbModAction *	act;
unsigned	tmp;
char *		str;

    act= (XkbModAction *)action;
    tmp= XkbModActionVMods(act);
    fprintf(file,"modifiers=");
    if (act->flags&XkbSA_UseModMapMods)
	 fprintf(file,"modMapMods");
    else if (act->real_mods || tmp) {
	if (act->real_mods)
	    fprintf(file,"%s%s",modMaskText(act->real_mods,XkbXKBFile),
							(tmp?"+":""));
	if (tmp)
	    fprintf(file,"%s",vmodMaskText(xkb,0,tmp,XkbXKBFile));
    }
    else fprintf(file,"none");
    if (action->type==XkbSA_LockMods)
	return True;
    if (act->flags&XkbSA_ClearLocks)
	fprintf(file,",clearLocks");
    if (act->flags&XkbSA_LatchToLock)
	fprintf(file,",latchToLock");
    return True;
}

static Bool
PrintGroupActionArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
XkbGroupAction *	act;
unsigned		tmp;

    act= (XkbGroupAction *)action;
    fprintf(file,"group=");
    if (act->flags&XkbSA_GroupAbsolute)
	 fprintf(file,"%d",XkbSAGroup(act)+1);
    else if (XkbSAGroup(act)<0)
	 fprintf(file,"%d",XkbSAGroup(act));
    else fprintf(file,"+%d",XkbSAGroup(act));
    if (action->type==XkbSA_LockGroup)
	return True;
    if (act->flags&XkbSA_ClearLocks)
	fprintf(file,",clearLocks");
    if (act->flags&XkbSA_LatchToLock)
	fprintf(file,",latchToLock");
    return True;
}

static Bool
PrintMoveAccelPtrArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
XkbPtrAction *	act;
int		x,y;

    act= (XkbPtrAction *)action;
    x= XkbPtrActionX(act);
    y= XkbPtrActionY(act);
    fprintf(file,"x=%d,y=%d",x,y);
    return True;
}

static Bool
PrintPtrBtnArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
XkbPtrBtnAction *	act;

    act= (XkbPtrBtnAction *)action;
    fprintf(file,"button=");
    if ((act->button>0)&&(act->button<6))
	 fprintf(file,"%d",act->button);
    else fprintf(file,"default");
    if (action->type==XkbSA_ClickPtrBtn) {
	fprintf(file,",count=%d",act->count);
    }
    return True;
}

static Bool
PrintSetPtrDfltArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
XkbPtrDfltAction *	act;

    act= (XkbPtrDfltAction *)action;
    if (act->affect==XkbSA_AffectDfltBtn) {
	fprintf(file,"affect=button,button=");
	if ((act->flags&XkbSA_DfltBtnAbsolute)||(XkbSAPtrDfltValue(act)<0))
	     fprintf(file,"%d",XkbSAPtrDfltValue(act));
	else fprintf(file,"+%d",XkbSAPtrDfltValue(act));
    }
    return True;
}

static Bool
PrintISOLockArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
XkbISOAction *	act;

    act= (XkbISOAction *)action;
    if (act->flags&XkbSA_ISODfltIsGroup) {
	fprintf(file,"group=");
	if (act->flags&XkbSA_GroupAbsolute)
	     fprintf(file,"%d",XkbSAGroup(act)+1);
	else if (XkbSAGroup(act)<0)
	     fprintf(file,"%d",XkbSAGroup(act));
	else fprintf(file,"+%d",XkbSAGroup(act));
    }
    else {
	unsigned tmp;
	tmp= XkbModActionVMods(act);
	fprintf(file,"modifiers=");
	if (act->flags&XkbSA_UseModMapMods)
	     fprintf(file,"modMapMods");
	else if (act->real_mods || tmp) {
	    if (act->real_mods)
		fprintf(file,"%s%s",modMaskText(act->real_mods,XkbXKBFile),
							(tmp?"+":""));
	    if (tmp)
		fprintf(file,"%s",vmodMaskText(xkb,0,tmp,XkbXKBFile));
	}
	else fprintf(file,"none");
    }
    fprintf(file,",affect=");
    if ((act->affect&XkbSA_ISOAffectMask)==0)
	fprintf(file,"all");
    else {
	int nOut= 0;
	if ((act->affect&XkbSA_ISONoAffectMods)==0) {
	    fprintf(file,"mods");
	    nOut++;
	}
	if ((act->affect&XkbSA_ISONoAffectGroup)==0) {
	    fprintf(file,"%sgroups",(nOut>0?"+":""));
	    nOut++;
	}
	if ((act->affect&XkbSA_ISONoAffectPtr)==0) {
	    fprintf(file,"%spointer",(nOut>0?"+":""));
	    nOut++;
	}
	if ((act->affect&XkbSA_ISONoAffectCtrls)==0) {
	    fprintf(file,"%scontrols",(nOut>0?"+":""));
	    nOut++;
	}
    }
    return True;
}

static Bool
PrintSwitchScreenArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
XkbSwitchScreenAction *	act;

    act= (XkbSwitchScreenAction *)action;
    if ((act->flags&XkbSA_SwitchAbsolute)||(XkbSAScreen(act)<0))
	 fprintf(file,"screen=%d",XkbSAScreen(act));
    else fprintf(file,"screen=+%d",XkbSAScreen(act));
    if (act->flags&XkbSA_SwitchApplication)
	 fprintf(file,",!same");
    else fprintf(file,",same");
    return True;
}

static Bool
PrintSetLockControlsArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
XkbCtrlsAction *	act;
unsigned		tmp;

    act= (XkbCtrlsAction *)action;
    tmp= XkbActionCtrls(act);
    fprintf(file,"controls=");
    if (tmp==0)
	fprintf(file,"none");
    else if ((tmp&XkbAllAccessXMask)==XkbAllAccessXMask)
	fprintf(file,"all");
    else {
	int nOut= 0;
	if (tmp&XkbRepeatKeysMask) {
	    fprintf(file,"%sRepeatKeys",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbSlowKeysMask) {
	    fprintf(file,"%sSlowKeys",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbBounceKeysMask) {
	    fprintf(file,"%sBounceKeys",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbStickyKeysMask) {
	    fprintf(file,"%sStickyKeys",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbMouseKeysMask) {
	    fprintf(file,"%sMouseKeys",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbMouseKeysAccelMask) {
	    fprintf(file,"%sMouseKeysAccel",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbAccessXKeysMask) {
	    fprintf(file,"%sAccessXKeys",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbAccessXTimeoutMask) {
	    fprintf(file,"%sAccessXTimeout",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbAccessXFeedbackMask) {
	    fprintf(file,"%sAccessXFeedback",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbGroupsWrapMask) {
	    fprintf(file,"%sGroupsWrap",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbAudibleBellMask) {
	    fprintf(file,"%sAudibleBell",(nOut>0?"+":""));
	    nOut++;
	}
	if (tmp&XkbAutoAutorepeatMask) {
	    fprintf(file,"%sAutoAutorepeat",(nOut>0?"+":""));
	    nOut++;
	}
    }
    return True;
}

static Bool
PrintActionMessageArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
int			nOut= 0;
XkbMessageAction *	act;
unsigned		all;

    act= (XkbMessageAction *)action;
    all= XkbSA_MessageOnPress|XkbSA_MessageOnRelease;
    fprintf(file,"report=");
    if ((act->flags&all)==0)
	fprintf(file,"none");
    else if ((act->flags&all)==all)
	fprintf(file,"all");
    else if (act->flags&XkbSA_MessageOnPress)
	 fprintf(file,"KeyPress");
    else fprintf(file,"KeyRelease");
    fprintf(file,",data[0]=0x%02x",act->message[0]);
    fprintf(file,",data[1]=0x%02x",act->message[1]);
    fprintf(file,",data[2]=0x%02x",act->message[2]);
    fprintf(file,",data[3]=0x%02x",act->message[3]);
    fprintf(file,",data[4]=0x%02x",act->message[4]);
    fprintf(file,",data[5]=0x%02x",act->message[5]);
    return True;
}

static Bool
PrintOtherArgs(file,indent,xkb,action)
    FILE *		file;
    unsigned		indent;
    XkbDescPtr		xkb;
    XkbAnyAction *	action;
{
    fprintf(file,"type=0x%02x",action->type);
    fprintf(file,",data[0]=0x%02x",action->data[0]);
    fprintf(file,",data[1]=0x%02x",action->data[1]);
    fprintf(file,",data[2]=0x%02x",action->data[2]);
    fprintf(file,",data[3]=0x%02x",action->data[3]);
    fprintf(file,",data[4]=0x%02x",action->data[4]);
    fprintf(file,",data[5]=0x%02x",action->data[5]);
    fprintf(file,",data[6]=0x%02x",action->data[6]);
    return True;
}

typedef	Bool	(*actionPrint)();
static actionPrint	printActionArgs[XkbSA_NumActions] = {
	PrintNoActionArgs		/* NoAction	*/,
	PrintModActionArgs		/* SetMods	*/,
	PrintModActionArgs		/* LatchMods	*/,
	PrintModActionArgs		/* LockMods	*/,
	PrintGroupActionArgs		/* SetGroup	*/,
	PrintGroupActionArgs		/* LatchGroup	*/,
	PrintGroupActionArgs		/* LockGroup	*/,
	PrintMoveAccelPtrArgs		/* MovePtr	*/,
	PrintMoveAccelPtrArgs		/* AccelPtr	*/,
	PrintPtrBtnArgs			/* PtrBtn	*/,
	PrintPtrBtnArgs			/* ClickPtrBtn	*/,
	PrintPtrBtnArgs			/* LockPtrBtn	*/,
	PrintSetPtrDfltArgs		/* SetPtrDflt	*/,
	PrintISOLockArgs		/* ISOLock	*/,
	PrintNoActionArgs		/* Terminate	*/,
	PrintSwitchScreenArgs		/* SwitchScreen	*/,
	PrintSetLockControlsArgs	/* SetControls	*/,
	PrintSetLockControlsArgs	/* LockControls	*/,
	PrintActionMessageArgs		/* ActionMessage*/,
};

static Bool
WriteXKBAction(file,result,action)
    FILE *		file;
    XkbFileResult *	result;
    XkbAnyAction *	action;
{
XkbDescPtr	xkb;

    xkb= &result->xkb;
    fprintf(file,"%s(",actionTypeText(action->type,XkbXKBFile));
    if (action->type<XkbSA_NumActions)
	 (*printActionArgs[action->type])(file,20,xkb,action);
    else PrintOtherArgs(file,20,xkb,action);
    fprintf(file,")");
    return True;
}

/***====================================================================***/

static Bool
WriteXKBKeycodes(file,result,topLevel)
    FILE *		file;
    XkbFileResult *	result;
    Bool		topLevel;
{
StringToken	kcName;
register int 	i;
char	buf[8],outBuf[8];
XkbDescPtr	xkb;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->names)||(!xkb->names->keys)) {
	uInternalError("Key names not defined in WriteXKBKeycodes\n");
	return False;
    }
    kcName= (StringToken)xkb->names->keycodes;
    if (kcName!=NullStringToken)
	 fprintf(file,"xkb_keycodes \"%s\"%s\n",stText(kcName),
							(topLevel?"":" {"));
    else fprintf(file,"xkb_keycodes%s\n",(topLevel?"":" {"));
    buf[4]= '\0';
    for (i=xkb->min_key_code;i<=xkb->max_key_code;i++) {
	memcpy(buf,xkb->names->keys[i].name,4);
	if (strlen(buf)>0) {
	    sprintf(outBuf,"<%s>",buf);
	    fprintf(file,"%6s = %d;\n",outBuf,i);
	}
    }
    if (!topLevel)
	fprintf(file,"};\n\n");
    return True;
}

static Bool
WriteXKBKeyTypes(file,result,topLevel)
    FILE *		file;
    XkbFileResult *	result;
    Bool		topLevel;
{
StringToken		iName;
register int 		i,n;
XkbKeyTypePtr		type;
XkbKTMapEntryPtr	entry;
XkbDescPtr	xkb;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->map)||(!xkb->map->types)) {
	uInternalError("Key types not defined in WriteXKBKeyTypes\n");
	return False;
    }
    if ((xkb->names==NULL)||(xkb->names->semantics==NullStringToken))
	 fprintf(file,"xkb_types%s\n\n",(topLevel?"":" {"));
    else fprintf(file,"xkb_types \"%s\"%s\n\n",stText(xkb->names->semantics),
						(topLevel?"":" {"));
    WriteXKBVModDecl(file,xkb);

    type= xkb->map->types;
    for (i=0;i<xkb->map->num_types;i++,type++) {
	fprintf(file,"type \"%s\" {\n",stText((StringToken)type->name));
	fprintf(file,"    modifiers=%s;\n",
		vmodMaskText(xkb,type->real_mods,type->vmods,XkbXKBFile));
	entry= type->map;
	for (n=0;n<type->map_count;n++,entry++) {
	    char *str;	
	    str=vmodMaskText(xkb,entry->real_mods,entry->vmods,XkbXKBFile);
	    fprintf(file,"    map[%s]=%d;\n",str,entry->level+1);
	    if ((type->preserve)&&((type->preserve[n].real_mods)||
				   (type->preserve[n].vmods))) {
		fprintf(file,"    preserve[%s]= ",str);
		fprintf(file,"%s;\n",vmodMaskText(xkb,
					type->preserve[n].real_mods,
					type->preserve[n].vmods,
					XkbXKBFile));
	    }
	}
	if (type->lvl_names!=NULL) {
	    StringToken *name= (StringToken *)type->lvl_names;
	    for (n=0;n<type->group_width;n++,name++) {
		if ((*name)==NullStringToken)
		    continue;
		fprintf(file,"    level_name[%d]=\"%s\";\n",n+1,stText(*name));
	    }
	}
	fprintf(file,"};\n");
    }
    if (!topLevel)
	fprintf(file,"};\n\n");
    return True;
}

static Bool
WriteXKBCompatMap(file,result,topLevel)
    FILE *		file;
    XkbFileResult *	result;
    Bool		topLevel;
{
StringToken		iName;
register int 		i;
XkbSymInterpretPtr	interp;
XkbDescPtr	xkb;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->compat)||(!xkb->compat->sym_interpret)) {
	uInternalError("Compat map not defined in WriteXKBCompatMap\n");
	return False;
    }
    if ((xkb->names==NULL)||(xkb->names->semantics==NullStringToken))
	 fprintf(file,"xkb_compatibility%s\n\n",(topLevel?"":" {"));
    else fprintf(file,"xkb_compatibility \"%s\"%s\n\n",
						stText(xkb->names->semantics),
						(topLevel?"":" {"));
    WriteXKBVModDecl(file,xkb);

    fprintf(file,"interpret.useModMapMods= AnyLevel;\n");
    fprintf(file,"interpret.repeat= False;\n");
    fprintf(file,"interpret.locking= False;\n");
    interp= xkb->compat->sym_interpret;
    for (i=0;i<xkb->compat->num_si;i++,interp++) {
	fprintf(file,"interpret %s+%s(%s) {\n",	
				((interp->sym==NoSymbol)?"Any":
					keysymText(interp->sym,XkbXKBFile)),
				SIMatchText(interp->match,XkbXKBFile),
				modMaskText(interp->mods,XkbXKBFile));
	if (interp->virtual_mod!=XkbNoModifier) {
	    fprintf(file,"    virtualModifier= %s;\n",
				VModText(xkb,interp->virtual_mod));
	}
	if (interp->match&XkbSI_LevelOneOnly)
	    fprintf(file,"    useModMapMods=level1;\n");
	if (interp->match&XkbSI_LockingKey)
	    fprintf(file,"    locking= True;\n");
	if (interp->match&XkbSI_Autorepeat)
	fprintf(file,"    repeat= True;\n");
	fprintf(file,"    action= ");
	WriteXKBAction(file,result,&interp->act);
	fprintf(file,";\n");
	fprintf(file,"};\n");
    }
    for (i=0;i<XkbNumModifiers;i++) {
	XkbModCompatPtr	mc;
	int		nOut;

	nOut= 0;
	mc= &xkb->compat->real_mod_compat[i];
	if ((mc->mods==(1<<i))&&(mc->groups==0))
	    continue;
	fprintf(file,"modifier %s = { ",modIndexText(i,XkbXKBFile));
	if (mc->mods!=(1<<i)) {
	    fprintf(file,"mods= %s;",modMaskText(mc->mods,XkbXKBFile));
	    nOut++;
	}
	if (mc->groups!=0) {
	    fprintf(file,"%sgroups= 0x%x;",(nOut>0?", ":""),mc->groups);
	}
	fprintf(file," };\n");
    }
    for (i=0;i<XkbNumVirtualMods;i++) {
	XkbModCompatPtr	mc;
	int		nOut;

	nOut= 0;
	mc= &xkb->compat->vmod_compat[i];
	if ((mc->mods==0)&&(mc->groups==0))
	    continue;
	fprintf(file,"modifier %s = { ",vmodIndexText(xkb,i,XkbXKBFile));
	if (mc->mods!=0) {
	    fprintf(file,"mods= %s;",modMaskText(mc->mods,XkbXKBFile));
	    nOut++;
	}
	if (mc->groups!=0) {
	    fprintf(file,"%sgroups= 0x%x;",(nOut>0?", ":""),mc->groups);
	}
	fprintf(file," };\n");
    }
    if (!topLevel)
	fprintf(file,"};\n\n");
    return True;
}

static Bool
WriteXKBSymbols(file,result,topLevel)
    FILE *		file;
    XkbFileResult *	result;
    Bool		topLevel;
{
register int i;
char	buf[8],buf2[8];
XkbDescPtr	xkb;

    xkb= &result->xkb;
    if ((!xkb)||(!xkb->map)||(!xkb->map->syms)||(!xkb->map->key_sym_map)) {
	uInternalError("Symbols not defined in WriteXKBSymbols\n");
	return False;
    }
    if ((!xkb->names)||(!xkb->names->keys)) {
	uInternalError("Key names not defined in WriteXKBSymbols\n");
	return False;
    }
    if ((xkb->names==NULL)||(xkb->names->symbols==NullStringToken))
	 fprintf(file,"xkb_symbols%s\n\n",(topLevel?"":" {"));
    else fprintf(file,"xkb_symbols \"%s\"%s\n\n",stText(xkb->names->symbols),
						 (topLevel?"":" {"));
    for (i=xkb->min_key_code;i<=xkb->max_key_code;i++) {
	Bool	simple;
	if (XkbKeyNumSyms(xkb,i)<1)
	    continue;
	simple= True;
	LongToKeyName(xkb->names->keys[i],buf);
	buf[4]= '\0';
	sprintf(buf2,"<%s>",buf);
	fprintf(file,"key %6s = {",buf2);
	if (xkb->server->explicit) {
	    if ((xkb->server->explicit[i]&XkbExplicitKeyTypeMask)!=0) {
		int typeNdx= xkb->map->key_sym_map[i].kt_index;
		fprintf(file,"\n    type= \"%s\",",
				stText(xkb->map->types[typeNdx].name));
		simple= False;
	    }
	    if (result->defined&PerKeyRepeatDefined) {
		if ((xkb->server->explicit[i]&XkbExplicitAutorepeatMask)!=0) {
		    if (result->repeat[i/8]&(1<<(i%8)))
			 fprintf(file,"\n    repeat= Yes,");
		    else fprintf(file,"\n    repeat= No,");
		    simple= False;
		}
	    }
	}
	if (xkb->server->behaviors!=NULL) {
	    unsigned type,permanent;
	    type= xkb->server->behaviors[i].type&XkbKB_OpMask;
	    permanent=((xkb->server->behaviors[i].type&XkbKB_Permanent)!=0);

	    if (type==XkbKB_Lock) {
		simple= False;
		fprintf(file,"\n    lock= %s,",(permanent?"Permanent":"True"));
	    }
	    else if (type==XkbKB_RadioGroup) {
		simple= False;
		if (permanent)
		     fprintf(file,"\n    permanentRadioGroup= ");
		else fprintf(file,"\n    radioGroup= ");
		fprintf(file,"%d,",xkb->server->behaviors[i].data+1);
	    }
	    else if (type!=XkbKB_Default) {
		uInternalError("Unknown key behavior %d encountered\n",type);
		uAction("Generated XKB description of <%s> might be wrong\n",
					longText(xkb->names->keys[i]));
	    }
	}
	if ((XkbKeyNumGroups(xkb,i)>1)||(XkbKeyHasActions(xkb,i)))
	    simple= False;
	if (simple) {
	    KeySym *syms;
	    unsigned s;

	    syms= XkbKeySymsPtr(xkb,i);
	    fprintf(file,"       [ ");
	    for (s=0;s<XkbKeyGroupWidth(xkb,i);s++) {
		if (s!=0)
		    fprintf(file,", ");
		fprintf(file,"%15s",keysymText(*syms++,XkbXKBFile));
	    }
	    fprintf(file," ] };\n");
	}
	else {
	    unsigned g,s;
	    KeySym *syms;
	    XkbAction *acts;
	    syms= XkbKeySymsPtr(xkb,i);
	    acts= XkbKeyActionsPtr(xkb,i);
	    for (g=0;g<XkbKeyNumGroups(xkb,i);g++) {
		if (g!=0)
		    fprintf(file,",");
		fprintf(file,"\n    symbols[Group%d]= [ ",g+1);
		for (s=0;s<XkbKeyGroupWidth(xkb,i);s++) {
		    if (s!=0)
			fprintf(file,", ");
		    fprintf(file,"%15s",keysymText(*syms++,XkbXKBFile));
		}
		fprintf(file," ]");
		if (XkbKeyHasActions(xkb,i)) {
		    fprintf(file,",\n    actions[Group%d]= [ ",g+1);
		    for (s=0;s<XkbKeyGroupWidth(xkb,i);s++) {
			if (s!=0)
			    fprintf(file,", ");
			WriteXKBAction(file,result,(XkbAnyAction *)acts++);
		    }
		    fprintf(file," ]");
		}
	    }
	    fprintf(file,"\n};\n");
	}
    }
    if (result->defined&ModMapDefined) {
	for (i=xkb->min_key_code;i<=xkb->max_key_code;i++) {
	    if (result->modmap[i]!=0) {
		register int n,bit;
		for (bit=1,n=0;n<XkbNumModifiers;n++,bit<<=1) {
		    if (result->modmap[i]&bit) {
			char buf[5];
			memcpy(buf,xkb->names->keys[i].name,4);
			buf[4]= '\0';
			fprintf(file,"modifier_map %s = { <%s> };\n",
					modIndexText(n,XkbXKBFile),buf);
		    }
		}
	    }
	}
    }
    if (!topLevel)
	fprintf(file,"};\n\n");
    return True;
}

static Bool
WriteXKBKeymap(file,result,topLevel)
    FILE *		file;
    XkbFileResult *	result;
    Bool		topLevel;
{
Bool	ok;
XkbDescPtr	xkb;

    xkb= &result->xkb;
    fprintf(file,"xkb_keymap\n");
    ok= WriteXKBKeycodes(file,result,False);
    ok= ok&&WriteXKBKeyTypes(file,result,False);
    ok= ok&&WriteXKBCompatMap(file,result,False);
    ok= ok&&WriteXKBSymbols(file,result,False);
#ifdef NOTDEF
    ok= ok&&WriteCHdrGeometry(file,result,False);
#endif
    return ok;
}

Bool
WriteSourceKbdDesc(name,type,result)
    char *		name;
    unsigned		type;
    XkbFileResult *	result;
{
FILE	*		out;
Bool	 		ok;
Bool			(*func)();

    switch (type) {
	case XkmSemanticsFile:
	case XkmLayoutFile:
	    uInternalError("Can't create an XKB file containing %s yet\n",
							configText(type));
	    return False;
	case XkmKeymapFile:
	    func= WriteXKBKeymap;
	    break;
	case XkmTypesIndex:
	    func= WriteXKBKeyTypes;
	    break;
	case XkmCompatMapIndex:
	    func= WriteXKBCompatMap;
	    break;
	case XkmSymbolsIndex:
	    func= WriteXKBSymbols;
	    break;
	case XkmKeyNamesIndex:
	    func= WriteXKBKeycodes;
	    break;
	case XkmGeometryFile:
	case XkmGeometryIndex:
	case XkmAlternateSymsFile:
	case XkmVirtualModsIndex:
	case XkmIndicatorsIndex:
	    uInternalError("Can't create an XKB file containing %s yet\n",
							configText(type));
	    return False;
    }
    out= fopen(name,"w");
    if (out==NULL) {
	uError("Couldn't open output file \"%s\"\n",name);
	ok= False;
    }
    else {
	ok= (*func)(out,result,True);
	fclose(out);
    }

    if (!ok) {
	uAction("Key Map source not written\n");
	return False;
    }
    return True;
}
