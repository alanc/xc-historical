/* $XConsortium$ */
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
#include <X11/extensions/XKBstr.h>
#include <X11/extensions/XKBlib.h>

void
printSyms(map,sym,num)
    XKBKeyTypeRec *map;
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
    XKBAction behavior;
{
static char buf[30];

    switch (XKBActionType(behavior)) {
	case XKB_KB_RADIO_GROUP:
		sprintf(buf,"radio group (%d)",XKBActionData(behavior));
		break;
	case XKB_KB_DEFAULT:	  
		strcpy(buf,"default");
		break;
	case XKB_KB_LOCK:	 
		strcpy(buf,"lock");
		break;
	default:		 
		sprintf(buf,"(unknown 0x%x)",behavior);
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

    str= (CARD8 *)buf;
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
actionText(sa)
    XKBAction sa;
{
static char buf[60];

    switch (XKBActionType(sa)) {
	case XKB_SA_NO_ACTION:
	    strcpy(buf,"NO_ACTION");
	    break;
	case XKB_SA_SET_MODS:
	    sprintf(buf,"SET_MODS(%s)",stateText(XKBActionDataLow(sa)));
	    break;
	case XKB_SA_ISO_LOCK:
	    sprintf(buf,"ISO_LOCK(%s)",stateText(XKBActionDataLow(sa)));
	    break;
	case XKB_SA_LOCK_MODS:
	    sprintf(buf,"LOCK_MODS(%s)",stateText(XKBActionDataLow(sa)));
	    break;
	case XKB_SA_LATCH_MODS:
	    sprintf(buf,"LATCH_MODS(%s)",stateText(XKBActionDataLow(sa)));
	    break;
	case XKB_SA_SET_GROUP:
	    sprintf(buf,"SET_GROUP(%s=%d)",
			(sa.flags&XKB_SA_GROUP_ABSOLUTE?"absolute":"relative"),
			XKBActionDataLow(sa));
	    break;
	case XKB_SA_LATCH_GROUP:
	    sprintf(buf,"LATCH_GROUP(%s=%d)",
			(sa.flags&XKB_SA_GROUP_ABSOLUTE?"absolute":"relative"),
			XKBActionDataLow(sa));
	    break;
	case XKB_SA_LOCK_GROUP:
	    sprintf(buf,"LOCK_GROUP(%s=%d)",
			(sa.flags&XKB_SA_GROUP_ABSOLUTE?"absolute":"relative"),
			XKBActionDataLow(sa));
	    break;
	case XKB_SA_MOVE_PTR:
	    sprintf(buf,"MOVE_POINTER=(%d,%d)",XKBActionData(sa)>>8,
						XKBActionData(sa)&0xff);
	    break;
	case XKB_SA_ACCEL_PTR:
	    sprintf(buf,"ACCEL_POINTER=(%d,%d)",XKBActionData(sa)>>8,
						XKBActionData(sa)&0xff);
	    break;
	case XKB_SA_PTR_BTN:
	    sprintf(buf,"POINTER_BUTTON(%d)",XKBActionData(sa));
	    break;
	case XKB_SA_CLICK_PTR_BTN:
	    sprintf(buf,"CLICK_POINTER_BUTTON(%d,%d)",
				XKBActionDataHigh(sa),XKBActionDataLow(sa));
	    break;
	case XKB_SA_LOCK_PTR_BTN:
	    sprintf(buf,"LOCK_POINTER_BUTTON(%d)",XKBActionData(sa));
	    break;
	case XKB_SA_SET_PTR_DFLT:
	    sprintf(buf,"SET_POINTER_DFLT(%d,%d)",XKBActionDataHigh(sa),
							XKBActionDataLow(sa)); 
	    break; 
	case XKB_SA_TERMINATE:
	    sprintf(buf,"TERMINATE_SERVER");
	    break;
	case XKB_SA_SWITCH_SCREEN:
	    sprintf(buf,"SWITCH_TO_SCREEN(0x%x,%d)",XKBActionDataHigh(sa),
							XKBActionDataLow(sa));
	    break;
	case XKB_SA_SET_CONTROLS:
	    sprintf(buf,"SET_CONTROLS(0x%x)",XKBActionData(sa));
	    break;
	case XKB_SA_LOCK_CONTROLS:
	    sprintf(buf,"LOCK_CONTROLS(0x%x)",XKBActionData(sa));
	    break;
	default:
	    sprintf(buf,"UNKNOWN(0x%x)",sa);
	    break;
    }
    return buf;
}

/***====================================================================***/

void
showKeys(dpy,xkb,which)
    Display *dpy;
    XKBDescRec *xkb;
    unsigned which;
{
int	i,key,nKeys;

    if ( !(which & (XKBKeySymsMask|XKBKeyBehaviorsMask|XKBKeyActionsMask)) )
	return;
    nKeys = xkb->maxKeyCode-xkb->minKeyCode+1;
    for (i=0,key=xkb->minKeyCode;i<nKeys;i++,key++) {
	printf("Key %d:\n",key);
	if ( which & XKBKeySymsMask ) {
	    int n = XKBKeyNumSyms(xkb,key);
	    int ng= XKBKeyNumGroups(xkb,key);
	    int tmp;
	    printf("    %d symbol%s in %d %swrapping group%s",
				n,(n>1?"s":""),ng,
				(XKBKeyGroupsWrap(xkb,key)?"":"non-"),
				(ng>1?"s":""));
	    if (xkb->names && xkb->names->keyTypes) 
		printf(" (%s)",atomText(dpy,
		xkb->names->keyTypes[xkb->map->keySymMap[key].ktIndex]));
	    for (tmp=0;tmp<ng;tmp++) {
		printf("\n    [ ");
		printSyms(XKBKeyKeyType(xkb,key),
				XKBKeySymsPtr(xkb,key)+(tmp*n/ng),n/ng);
		printf("]");
	    }
	    printf("\n");
	}
	if ( which & XKBKeyActionsMask ) {
	    int nActs = XKBKeyNumActions(xkb,key);
	    XKBAction *acts=XKBKeyActionsPtr(xkb,key);
	    printf("    Actions:  ");
	    if (nActs==1) {
		if (acts[0].type==XKB_SA_NO_ACTION) 
		     printf("None\n");
		else printf("%s\n",actionText(acts[0]));
	    }
	    else {
		int nGroups= XKBKeyNumGroups(xkb,key);
		int nLevels= nActs/nGroups;
		int g,l;
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
	if ( which & XKBKeyBehaviorsMask ) {
	    printf("    Behavior: %s\n",
				behaviorText(xkb->server->keyBehaviors[key]));
	}
    }
}

void
showKeyTypes(dpy,desc)
    Display *dpy;
    XKBDescRec *desc;
{
XKBClientMapRec *map = desc->map;
int	i,m;

    for (i=0;i<map->nKeyTypes;i++) {
	XKBKeyTypeRec *type= &map->keyTypes[i];
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
static	unsigned	 which = XKBAllMapComponentsMask;
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
			case 't':	which|= XKBKeyTypesMask; break;
			case 's':	which|= XKBKeySymsMask; break;
			case 'a':	which|= XKBKeyActionsMask; break;
			case 'b':	which|= XKBKeyBehaviorsMask; break;
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
int	i1,i2;
extern	Bool	XKBQueryExtension(Display *,int *,int *);
XKBDescRec	*desc;
XKBClientMapRec	*map;
XKBStateRec	 state;
XKBControlsRec	*ctrls;
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
    if ( !XKBQueryExtension(dpy,&i1,&i2)>0 ) {
	fprintf(stderr,"query failed\n");
	goto BAIL;
    }
    if ( !XKBUseExtension(dpy,&i1,&i2) ) {
	fprintf(stderr,"use extension failed (%d,%d)\n",i1,i2);
	goto BAIL;
    }
    if ((getState>0) && (getMap<0))		getMap = 0;
    else if ((getMap>0) && (getState<0))	getState = 0;
    else if ((getMap<0) && (getState<0))	getMap = getState = 1;

    if ( getState ) {
	if (!XKBGetState(dpy,XKB_USE_CORE_KBD,&state)) {
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
	if (query&(XKBKeySymsMask|XKBKeyActionsMask))
	    query|= XKBKeyTypesMask;
	desc = XKBGetMap(dpy,0,XKB_USE_CORE_KBD);
	map= desc->map;
	if ( !usePartialQueries ) {
	    if ( !XKBGetUpdatedMap(dpy,query,desc) ) {
		fprintf(stderr,"get keyboard desc request failed\n");
		goto BAIL;
	    }
	}
	else {
	    if ( query & XKBKeyTypesMask )  {
		if ( !XKBGetKeyTypes(dpy,0,3,map) ) {
		    fprintf(stderr,"XKBGetKeyTypes failed\n");
		    goto BAIL;
		}
	    }
	}

 	if (!XKBGetControls(dpy,XKBAllControlsMask,desc)) {
	    fprintf(stderr,"XKBGetControls failed\n");
	    goto BAIL;
	}
	if (!XKBGetNames(dpy,XKBAllNamesMask,desc)) {
	    fprintf(stderr,"XKBGetNames failed\n");
	    goto BAIL;
	}
	ctrls= desc->controls;
	printf("Device ID:     %d\n",desc->deviceSpec);
	printf("keycodes type: %s\n",atomText(dpy,desc->names->keycodes));
	printf("geometry type: %s\n",atomText(dpy,desc->names->geometry));
	printf("keycode range: %d-%d\n",desc->minKeyCode,desc->maxKeyCode);
	printf("audible bell:     %s\n",
		((ctrls->enabledControls&XKBAudibleBellMask)?"on":"off"));
	printf("auto autorepeat:  %s\n",
		((ctrls->enabledControls&XKBAutoAutorepeatMask)?"on":"off"));
	printf("internal mods:    0x%x\n",ctrls->internalMods);
	printf("ignore lock mods: 0x%x\n",ctrls->ignoreLockMods);
	printf("repeat keys:      %s (%d/%d)\n",
		((ctrls->enabledControls&XKBRepeatKeysMask)?"on":"off"),
		ctrls->repeatDelay,ctrls->repeatInterval);
	printf("slow keys:        %s (%d)\n",
		(ctrls->enabledControls&XKBSlowKeysMask?"on":"off"),
		ctrls->slowKeysDelay);
	printf("bounce keys:      %s (%d)\n",
		(ctrls->enabledControls&XKBBounceKeysMask?"on":"off"),
		ctrls->debounceDelay);
	printf("sticky keys:      %s\n",
		(ctrls->enabledControls&XKBStickyKeysMask?"on":"off"));
	printf("mouse keys:      %s (btn=%d,accel=%d/%d/%d/%d)\n",
		(ctrls->enabledControls&XKBMouseKeysMask?"on":"off"),
		ctrls->mouseKeysDfltBtn,
		ctrls->mouseKeysDelay, ctrls->mouseKeysInterval,
		ctrls->mouseKeysTimeToMax, ctrls->mouseKeysCurve);
	printf("access X keys:   %s (timeout=%d)\n",
		(ctrls->enabledControls&XKBAccessXKeysMask?"on":"off"),
		ctrls->accessXTimeout);
	printf("modifier names:\n");
	for (i1=0;i1<8;i1++) {
	    printf("    %s\n",atomText(dpy,desc->names->modifiers[i1]));
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
		(ctrls->enabledControls&XKBGroupsWrapMask)?"wrap":"don't wrap");
	showKeys(dpy,desc,which);
	if ( which & XKBKeyTypesMask )
	    showKeyTypes(dpy,desc);
	XCloseDisplay(dpy);
    }
    return 0;
BAIL:
    XCloseDisplay(dpy);
    return 0;
}
