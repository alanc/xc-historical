/* $XConsortium: xkbshow.c,v 1.2 93/09/28 23:52:06 rws Exp $ */
/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

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
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/XKBlib.h>

void
printSyms(map,sym,num)
    XkbKeyTypeRec *map;
    KeySym *sym;
    int num;
{
int	i,nsPending;
char	*name;

    nsPending= 0;
    for (i=0;i<num;i++) {
	if ( sym[i]==NoSymbol )
	    nsPending++;
	else {
	    while (nsPending>0) {
		printf("NoSymbol ");
		nsPending--;
	    }
	    name = XKeysymToString(sym[i]);
	    if ( name )	printf("%s ",name);
	    else	printf("unknown (0x%x)",sym[i]);
	}
    }
}

char *
atomText(dpy,atom)
    Display *dpy;
    Atom atom;
{
static char buf[256];
char	*name;

    if (atom==None) {
	sprintf(buf,"(None)");
	return buf;
    }
    name= XGetAtomName(dpy,atom);
    if ( name ) {
	strncpy(buf,name,255);
	buf[255]= '\0';
	XFree(name);
    }
    else {
	sprintf(buf,"(unknown)");
    }
    return buf;
}

char *
behaviorText(behavior)
    XkbBehavior behavior;
{
static char buf[30];

    switch (behavior.type) {
	case XkbRadioGroupKB:
		sprintf(buf,"radio group (%d)",behavior.data);
		break;
	case XkbDefaultKB:	  
		strcpy(buf,"default");
		break;
	case XkbLockKB:	 
		strcpy(buf,"lock");
		break;
	default:		 
		sprintf(buf,"(unknown 0x%x)",behavior.type);
		break;
    }
    return buf;
}

char *
stateText(state)
    CARD8 state;
{
static char buf[12];
char	*str;

    if ( state == 0 ) {
	strcpy(buf,"none");
	return buf;
    }

    str= buf;
    if ( state&ShiftMask )	*str++ = 'S';
    if ( state&ControlMask )	*str++ = 'C';
    if ( state&LockMask )	*str++ = 'L';
    if ( state&Mod1Mask )	*str++ = '1';
    if ( state&Mod2Mask )	*str++ = '2';
    if ( state&Mod3Mask )	*str++ = '3';
    if ( state&Mod4Mask )	*str++ = '4';
    if ( state&Mod5Mask )	*str++ = '5';
    *str++ = '\0';
    return buf;
}

char *
modsFlagsText(flags)
    CARD8 flags;
{
static char buf[48];
char *str= buf;

    if (flags&XkbSAClearLocks) {
	if (str!=buf)	*str++= '+';
	strcpy(str,"clear");
	str+= strlen("clear");
    }
    if (flags&XkbSALatchToLock) {
	if (str!=buf)	*str++= '+';
	strcpy(str,"latch->lock");
	str+= strlen("latch->lock");
    }
    *str++= '\0';
    return buf;
}
char *

isoAffectText(flags)
    CARD8 flags;
{
static char buf[48];
char *str= buf;

    if (flags&XkbSAISONoAffectMods) {
	if (str==buf)	*str++= '!';
	*str++= 'M';
    }
    if (flags&XkbSAISONoAffectGroup) {
	if (str==buf)	*str++= '!';
	*str++= 'G';
    }
    if (flags&XkbSAISONoAffectPtr) {
	if (str==buf)	*str++= '!';
	*str++= 'P';
    }
    if (flags&XkbSAISONoAffectCtrls) {
	if (str==buf)	*str++= '!';
	*str++= 'C';
    }
    if (str==buf)
	 strcpy(str,"all");
    else *str++= '\0';
    return buf;
}

char *
actionText(sa)
    XkbAction sa;
{
static char buf[100];
char	*str1,*str2;

    switch (sa.type) {
	case XkbSANoAction:
	    strcpy(buf,"NoAction");
	    break;
	case XkbSASetMods:
	    str1= (sa.mods.flags?modsFlagsText(sa.mods.flags):NULL);
	    str2= (sa.mods.suppressLocks?stateText(sa.mods.suppressLocks):NULL);
	    sprintf(buf,"SetMods(%s%s%s%s%s)",stateText(sa.mods.mods),
					(str1?",":""),(str1?str1:""),
					(str2?",suppress=":""),(str2?str2:""));
	    break;
	case XkbSAISOLock:
	    str1= isoAffectText(sa.iso.affect);
	    if (sa.iso.flags&XkbSAISODfltIsGroup)
		 sprintf(buf,"ISOLock(group=%d,affect=%s)",sa.iso.group,str1);
	    else sprintf(buf,"ISOLock(mods=%s,affect=%s)",
				stateText(sa.iso.mods),str1);
	    break;
	case XkbSALockMods:
	    sprintf(buf,"LockMods(%s)",stateText(sa.mods.mods));
	    break;
	case XkbSALatchMods:
	    str1= (sa.mods.flags?modsFlagsText(sa.mods.flags):NULL);
	    str2= (sa.mods.suppressLocks?stateText(sa.mods.suppressLocks):NULL);
	    sprintf(buf,"LatchMods(%s%s%s%s%s)",stateText(sa.mods.mods),
					(str1?",":""),(str1?str1:""),
					(str2?",":""),(str2?str2:""));
	    break;
	case XkbSASetGroup:
	    str1= (sa.group.flags?modsFlagsText(sa.group.flags):NULL);
	    sprintf(buf,"SetGroup(%d(%s)%s%s%s)",sa.group.group,
		(sa.group.flags&XkbSAGroupAbsolute?"absolute":"relative"),
		(str1?",":""),(str1?str1:""),
		(sa.group.suppressLocks?",suppress=group":""));
	    break;
	case XkbSALatchGroup:
	    str1= (sa.group.flags?modsFlagsText(sa.group.flags):NULL);
	    sprintf(buf,"LatchGroup(%d(%s)%s%s%s)",sa.group.group,
		(sa.group.flags&XkbSAGroupAbsolute?"absolute":"relative"),
		(str1?",":""),(str1?str1:""),
		(sa.group.suppressLocks?",suppress=group":""));
	    break;
	case XkbSALockGroup:
	    sprintf(buf,"LockGroup(%d(%s))",sa.group.group,
		(sa.group.flags&XkbSAGroupAbsolute?"absolute":"relative"));
	    break;
	case XkbSAMovePtr:
	    sprintf(buf,"MovePtr=(%d,%d)",XkbPtrActionX(&sa.ptr),
						XkbPtrActionY(&sa.ptr));
	    break;
	case XkbSAAccelPtr:
	    sprintf(buf,"AccelPtr=(%d,%d)",XkbPtrActionX(&sa.ptr),
						XkbPtrActionY(&sa.ptr));
	    break;
	case XkbSAPtrBtn:
	    if (sa.btn.button==XkbSAUseDfltButton)
		 sprintf(buf,"PtrBtn(dflt)");
	    else sprintf(buf,"PtrBtn(%d)",sa.btn.button);
	    break;
	case XkbSAClickPtrBtn:
	    if (sa.btn.button==XkbSAUseDfltButton)
		 sprintf(buf,"ClickPtrBtn(%d,default)",sa.btn.count);
	    else sprintf(buf,"ClickPtrBtn(%d,%d)",sa.btn.count,sa.btn.button);
	    break;
	case XkbSALockPtrBtn:
	    if (sa.btn.button==XkbSAUseDfltButton)
		 sprintf(buf,"LockPtrBtn(default)");
	    else sprintf(buf,"LockPtrBtn(%d)",sa.btn.button);
	    break;
	case XkbSASetPtrDflt:
	    sprintf(buf,"SetPtrDflt(%s,%d)",
		(sa.dflt.flags==XkbSASetDfltBtn?"setDfltBtn":
			(sa.dflt.flags==XkbSAIncrDfltBtn?"IncrDfltBtn":
							 "Unknown")),
		sa.dflt.value);
	    break; 
	case XkbSATerminate:
	    sprintf(buf,"Terminate");
	    break;
	case XkbSASwitchScreen:
	    sprintf(buf,"SwitchScreen(0x%x,%d)",sa.screen.flags,
						sa.screen.screen);
	    break;
	case XkbSASetControls:
	    sprintf(buf,"SetControls(0x%x)",XkbActionCtrls(&sa.ctrls));
	    break;
	case XkbSALockControls:
	    sprintf(buf,"LockControls(0x%x)",XkbActionCtrls(&sa.ctrls));
	    break;
	default:
	    sprintf(buf,"Unknown(0x%x)",sa);
	    break;
    }
    return buf;
}

/***====================================================================***/

void
showKeys(dpy,xkb,which)
    Display *dpy;
    XkbDescRec *xkb;
    unsigned which;
{
int	i,key,nKeys;

    if ( !(which & (XkbKeySymsMask|XkbKeyBehaviorsMask|XkbKeyActionsMask)) )
	return;
    nKeys = xkb->maxKeyCode-xkb->minKeyCode+1;
    for (i=0,key=xkb->minKeyCode;i<nKeys;i++,key++) {
	printf("Key %d:\n",key);
	if ( which & XkbKeySymsMask ) {
	    int n = XkbKeyNumSyms(xkb,key);
	    int ng= XkbKeyNumGroups(xkb,key);
	    int tmp;
	    printf("    %d symbol%s in %d %swrapping group%s",
				n,(n>1?"s":""),ng,
				(XkbKeyGroupsWrap(xkb,key)?"":"non-"),
				(ng>1?"s":""));
	    if (xkb->names && xkb->names->keyTypes) 
		printf(" (%s)",atomText(dpy,
		xkb->names->keyTypes[xkb->map->keySymMap[key].ktIndex]));
	    for (tmp=0;tmp<ng;tmp++) {
		printf("\n    [ ");
		printSyms(XkbKeyKeyType(xkb,key),
				XkbKeySymsPtr(xkb,key)+(tmp*n/ng),n/ng);
		printf("]");
	    }
	    printf("\n");
	}
	if ( which & XkbKeyActionsMask ) {
	    int nActs = XkbKeyNumActions(xkb,key);
	    XkbAction *acts=XkbKeyActionsPtr(xkb,key);
	    if ((nActs>1)||(acts[0].type!=XkbSANoAction)) {
		int nGroups= XkbKeyNumGroups(xkb,key);
		int nLevels= nActs/nGroups;
		int g,l;
		printf("    Actions: ");
		for (g=0;g<nGroups;g++) {
		    if (g==0)
			printf("[ ");
		    else printf("              [ ");
		    for (l=0;l<nLevels;l++) {
			printf("%s ",actionText(acts[(g*nGroups)+l]));
		    }
		    printf("]\n");
		}
	    }
	}
	if ( which & XkbKeyBehaviorsMask ) {
	    printf("    Behavior: %s\n",
				behaviorText(xkb->server->keyBehaviors[key]));
	}
    }
}

void
showKeyTypes(dpy,desc)
    Display *dpy;
    XkbDescRec *desc;
{
XkbClientMapRec *map = desc->map;
int	i,m;

    for (i=0;i<map->nKeyTypes;i++) {
	XkbKeyTypeRec *type= &map->keyTypes[i];
	printf("Key Type %d",i);
	if (desc->names && desc->names->keyTypes)
	     printf(" (%s):\n",atomText(dpy,desc->names->keyTypes[i]));
	else printf("\n");
	printf("    mask:         0x%x\n",type->mask);
	printf("    groupWidth:   %d\n",type->groupWidth);
	printf("    map:\n");
	printf("       no modifiers:  %d",type->map[0]);
	if (desc->names && desc->names->levels)
	    printf("  \"%s\"  ",atomText(dpy,desc->names->levels[i][0]));
	if ( type->preserve ) {
	    printf("(preserve %s)",stateText(type->preserve[0]));
	}
	printf("\n");
	for (m=1;m<255;m++) {
	    if ( (m&type->mask)==m ) {
		printf("       %12s:  %d",stateText(m),type->map[m]);
		if (desc->names && desc->names->levels) 
		    printf("  \"%s\"  ",atomText(dpy,
					 desc->names->levels[i][type->map[m]]));
		if ( type->preserve ) {
		    printf("(preserve %s)",stateText(type->preserve[m]));
		}
		printf("\n");
	    }
	}
    }
}

/***====================================================================***/

static	char		*dpyName = NULL;
static	int		 getState = -1;
static	int		 getMap = -1;
static	unsigned	 which = XkbAllMapComponentsMask;
static	int		 usePartialQueries = 0;
static	int		 synch = 0;

int
parseArgs(argc,argv)
    int argc;
    char *argv[];
{
int i;

    for (i=1;i<argc;i++) {
	 if ( strcmp(argv[i],"-display")==0 ) {
	    if ( ++i<argc )	dpyName= argv[i];
	    else {
		fprintf(stderr,"Must specify a display with -display option\n");
		return 0;
	    }
	}
	else if ( strcmp(argv[i],"-m")==0 ) {
	    if ((i+1<argc) && (argv[i+1][0]!='-')) {
		char *subsets = argv[++i];
		which = 0;
		while (*subsets) {
		    switch (*subsets) {
			case 't':	which|= XkbKeyTypesMask; break;
			case 's':	which|= XkbKeySymsMask; break;
			case 'a':	which|= XkbKeyActionsMask; break;
			case 'b':	which|= XkbKeyBehaviorsMask; break;
			default:
			    fprintf(stderr,"Unknown subset %c\n",*subsets);
			    return 0;
		    }
		    subsets++;
		}
		if ( !which ) {
		    fprintf(stderr,"no subsets specified\n");
		    return 0;
		}
	    }
	    else getMap = 1;
	}
	else if ( strcmp(argv[i],"-s")==0 ) {
	    getState = 1;
	}
	else if ( strcmp(argv[i],"-1")==0 ) {
	    usePartialQueries = 1;
	}
	else if ( strcmp(argv[i],"-synch")==0 ) {
	    synch= 1;
	}
	else {
	    fprintf(stderr,"Unknown option %s\n",argv[i]);
	    return 0;
	}
    }
    return 1;
}

int
main(argc,argv)
    int argc;
    char *argv[];
{
Display	*dpy;
int	i1,i2,i3,i4,i5;
XkbDescRec	*desc;
XkbClientMapRec	*map;
XkbStateRec	 state;
XkbControlsRec	*ctrls;
unsigned	 query;

  
    if (!parseArgs(argc,argv)) {
	fprintf(stderr,"Usage: %s <options>\n",argv[0]);
	fprintf(stderr,"Where legal options are:\n");
	fprintf(stderr,"-display <dpy>     specifies display to use\n");
	fprintf(stderr,"-s                 report keyboard state\n");
	fprintf(stderr,"-m <subsets>       specifies subset of keyboard mapping to display\n");
	fprintf(stderr,"                   Legal subsets are:\n");
	fprintf(stderr,"		   t:  Key Types\n");
	fprintf(stderr,"                   s:  Key Syms\n");
	fprintf(stderr,"                   a:  Key Actions\n");
	fprintf(stderr,"                   b:  Key Behaviors\n");
	fprintf(stderr,"                   r:  Radio Groups\n");
	fprintf(stderr,"-1                 specifies use of partial queries\n");
	return 1;
    }
    dpy = XOpenDisplay(dpyName);
    if ( !dpy )
	return 1;
    if (synch)
	XSynchronize(dpy,1);
    if ( !XkbQueryExtension(dpy,&i1,&i2,&i3,&i4,&i5)>0 ) {
	fprintf(stderr,"query failed\n");
	goto BAIL;
    }
    if ((getState>0) && (getMap<0))		getMap = 0;
    else if ((getMap>0) && (getState<0))	getState = 0;
    else if ((getMap<0) && (getState<0))	getMap = getState = 1;

    if ( getState ) {
	if (!XkbGetState(dpy,XkbUseCoreKbd,&state)) {
	    fprintf(stderr,"get keyboard state request failed\n");
	    goto BAIL;
	}
        printf("group:         %d\n",state.group);
        printf("latched group: %d\n",state.latchedGroup);
	printf("mods:          %s\n",stateText(state.mods));
	printf("latched:       %s\n",stateText(state.latchedMods));
	printf("locked:        %s\n",stateText(state.lockedMods));
	printf("compatibility state: %s\n",stateText(state.compatState));
    }
    if ( getMap ) {
	query = which;
	if (query&(XkbKeySymsMask|XkbKeyActionsMask))
	    query|= XkbKeyTypesMask;
	desc = XkbGetMap(dpy,0,XkbUseCoreKbd);
	map= desc->map;
	if ( !usePartialQueries ) {
	    if ( !XkbGetUpdatedMap(dpy,query,desc) ) {
		fprintf(stderr,"get keyboard desc request failed\n");
		goto BAIL;
	    }
	}
	else {
	    if ( query & XkbKeyTypesMask )  {
		if ( !XkbGetKeyTypes(dpy,0,3,map) ) {
		    fprintf(stderr,"XkbGetKeyTypes failed\n");
		    goto BAIL;
		}
	    }
	}

 	if (!XkbGetControls(dpy,XkbAllControlsMask,desc)) {
	    fprintf(stderr,"XkbGetControls failed\n");
	    goto BAIL;
	}
	if (!XkbGetNames(dpy,XkbAllNamesMask,desc)) {
	    fprintf(stderr,"XkbGetNames failed\n");
	    goto BAIL;
	}
	ctrls= desc->controls;
	printf("Device ID:     %d\n",desc->deviceSpec);
	printf("keycodes type: %s\n",atomText(dpy,desc->names->keycodes));
	printf("geometry type: %s\n",atomText(dpy,desc->names->geometry));
	printf("keycode range: %d-%d\n",desc->minKeyCode,desc->maxKeyCode);
	printf("audible bell:     %s\n",
		((ctrls->enabledControls&XkbAudibleBellMask)?"on":"off"));
	printf("auto autorepeat:  %s\n",
		((ctrls->enabledControls&XkbAutoAutorepeatMask)?"on":"off"));
	printf("internal mods:    0x%x\n",ctrls->internalMods);
	printf("ignore lock mods: 0x%x\n",ctrls->ignoreLockMods);
	printf("repeat keys:      %s (%d/%d)\n",
		((ctrls->enabledControls&XkbRepeatKeysMask)?"on":"off"),
		ctrls->repeatDelay,ctrls->repeatInterval);
	printf("slow keys:        %s (%d)\n",
		(ctrls->enabledControls&XkbSlowKeysMask?"on":"off"),
		ctrls->slowKeysDelay);
	printf("bounce keys:      %s (%d)\n",
		(ctrls->enabledControls&XkbBounceKeysMask?"on":"off"),
		ctrls->debounceDelay);
	printf("sticky keys:      %s\n",
		(ctrls->enabledControls&XkbStickyKeysMask?"on":"off"));
	printf("mouse keys:      %s (btn=%d,accel=%d/%d/%d/%d)\n",
		(ctrls->enabledControls&XkbMouseKeysMask?"on":"off"),
		ctrls->mouseKeysDfltBtn,
		ctrls->mouseKeysDelay, ctrls->mouseKeysInterval,
		ctrls->mouseKeysTimeToMax, ctrls->mouseKeysCurve);
	printf("access X keys:   %s (timeout=%d)\n",
		(ctrls->enabledControls&XkbAccessXKeysMask?"on":"off"),
		ctrls->accessXTimeout);
	printf("modifier names:\n");
	for (i1=0;i1<8;i1++) {
	    printf("    %s\n",atomText(dpy,desc->names->modifiers[i1]));
	}
	for (i1=0;i1<XkbNumIndicators;i1++) {
	    if (desc->names->indicators[i1]!=None) {
		static int been_here=0;
		if (!been_here) {
		    been_here=1;
		    printf("indicator names:\n");
		}
		printf("%2d: %s\n",i1,
				   atomText(dpy,desc->names->indicators[i1]));
	    }
	}
	printf("symbols name:  %s\n",atomText(dpy,desc->names->symbols));
	if (desc->names->nCharSets) {
	    printf("character sets: ");
	    for (i1=0;i1<desc->names->nCharSets;i1++) {
		if (i1==0)
		     printf("%s",atomText(dpy,desc->names->charSets[i1]));
		else printf(", %s",atomText(dpy,desc->names->charSets[i1]));
	    }
	    printf("\n");
	}
	else printf("No character sets defined\n");
	printf("%d keyboard groups\n",ctrls->numGroups);
	printf("keyboard groups %s\n",
		(ctrls->enabledControls&XkbGroupsWrapMask)?"wrap":"don't wrap");
	showKeys(dpy,desc,which);
	if ( which & XkbKeyTypesMask )
	    showKeyTypes(dpy,desc);
	XCloseDisplay(dpy);
    }
    return 0;
BAIL:
    XCloseDisplay(dpy);
    return 0;
}
