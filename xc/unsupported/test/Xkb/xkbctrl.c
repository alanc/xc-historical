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
#include <string.h>
#include <ctype.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/extensions/XKBstr.h>
#include <X11/extensions/XKBlib.h>

static	char		*dpyName = NULL;
static	unsigned	 which;
static	XKBControlsRec	 newControls;
static	unsigned	 changeInternal;
static	unsigned	 internal;
static	unsigned	 changeIgnoreLocks;
static	unsigned	 ignoreLocks;
static	unsigned	 changeEnabled;
static	unsigned	 enabled;
static	int		 synch = 0;
static	int		 device = XKB_USE_CORE_KBD;

int
parseMods(modText,onOff,changes,mods)
    char *modText;
    int   onOff;
    unsigned *changes;
    unsigned *mods;
{
register char *tmp;

    for (tmp=modText;*tmp;tmp++) {
	if ((*tmp=='s')||(*tmp=='S')) {
	    *changes|= ShiftMask;
	    if (onOff)	*mods|= ShiftMask;
	    else	*mods&= ~ShiftMask;
	}
	if ((*tmp=='l')||(*tmp=='L')) {
	    *changes|= LockMask;
	    if (onOff)	*mods|= LockMask;
	    else	*mods&= ~LockMask;
	}
	if ((*tmp=='c')||(*tmp=='C')) {
	    *changes|= ControlMask;
	    if (onOff)	*mods|= ControlMask;
	    else	*mods&= ~ControlMask;
	}
	if ((*tmp=='a')||(*tmp=='A')||(*tmp=='m')||(*tmp=='M')||(*tmp=='1')) {
	    *changes|= Mod1Mask;
	    if (onOff)	*mods|= Mod1Mask;
	    else	*mods&= ~Mod1Mask;
	}
	if (*tmp=='2') {
	    *changes|= Mod2Mask;
	    if (onOff)	*mods|= Mod2Mask;
	    else	*mods&= ~Mod2Mask;
	}
	if (*tmp=='3') {
	    *changes|= Mod3Mask;
	    if (onOff)	*mods|= Mod3Mask;
	    else	*mods&= ~Mod3Mask;
	}
	if (*tmp=='4') {
	    *changes|= Mod4Mask;
	    if (onOff)	*mods|= Mod4Mask;
	    else	*mods&= ~Mod2Mask;
	}
	if (*tmp=='5') {
	    *changes|= Mod5Mask;
	    if (onOff)	*mods|= Mod5Mask;
	    else	*mods&= ~Mod5Mask;
	}
    }
    return 1;
}

#define	E(m)	fprintf(stderr,m)
void
usage(argc,argv)
    int argc;
    char *argv[];
{
    fprintf(stderr,"Usage: %s <options>\n",argv[0]);
    E("Where legal options are:\n");
    E("-display <dpy>                 specifies display to use\n");
    E("[+-]synch                      synchonize on/off\n");
    E("-device  <id>                  specifies device to use\n");
    E("[+-]wrap                       group wrap on/off\n");
    E("[+-]bell                       audible bell on/off\n");
    E("[+-]autoautorepeat             automatic determination of repeating keys on/off\n");
    E("[+-]internal <mods>            set/clear internal modifiers\n");
    E("[+-]ignorelock <mods>          set/clear internal modifiers\n");
    E("                               <mods> can contain one or more of:\n");
    E("                               s:         Shift\n");
    E("                               l:         Lock\n");
    E("                               c:         Control\n");
    E("                               [am]:      Alt/Meta (mod1)\n");
    E("                               [1-5]:     Mod1-Mod5\n");
    E("-repeat                        disable RepeatKeys\n");
    E("+repeat [ delay [ interval ] ] enable RepeatKeys with the specified delay\n");
    E("                               and interval\n");
    E("-slow                          disable SlowKeys\n");
    E("+slow [ delay ]                enable SlowKeys with the specified delay\n");
    E("-bounce                        disable BounceKeys\n");
    E("+bounce [timeout]              enable BounceKeys with the specified timeout\n");
    E("[+-]sticky                     enable/disable sticky keys\n");
    E("-mouse                         disable MouseKeys\n");
    E("+mouse [ delay [ interval [ time-to-max [ dflt-button ] ] ] ]\n");
    E("                               enable mouse keys with the specified behavior\n");
    E("-accessx                       disable AccessX hotkeys and timeout\n");
    E("+accessx [timeout]             enable AccessX hotkeys with the specified\n");
    E("                               timeout (0 indicates no timeout)\n");
}

int
parseArgs(argc,argv)
    int argc;
    char *argv[];
{
int i;
int onoff;

    for (i=1;i<argc;i++) {
	 if (argv[i][0]=='-')		onoff= 0;
	 else if (argv[i][0]=='+')	onoff= 1;
	 else {
	     fprintf(stderr,"Options must start with '+' or '-'\n");
	     return 0;
	 }
	 if ( strcmp(argv[i],"-display")==0 ) {
	    if ( ++i<argc )	dpyName= argv[i];
	    else {
		fprintf(stderr,"Must specify a display with -display option\n");
		return 0;
	    }
	}
	else if ( strcmp(argv[i],"-device")==0 ) {
	    if (( (i+1)<argc ) && isdigit(argv[i+1][0]))
		device= atoi(argv[++i]);
	    else {
		fprintf(stderr,"Must specify an id with -device option\n");
		return 0;
	    }
	}
	else if ( strcmp(&argv[i][1],"synch")==0 ) {
	    synch= onoff;
	}
	else if ( strcmp(&argv[i][1],"wrap")==0 ) {
	    which|= XKBControlsEnabledMask;
	    if (onoff)	enabled|= XKBGroupsWrapMask;
	    else	enabled&= ~XKBGroupsWrapMask;
	}
	else if ( strcmp(&argv[i][1],"bell")==0 ) {
	    which|= XKBControlsEnabledMask;
	    if (onoff)	enabled|= XKBAudibleBellMask;
	    else	enabled&= ~XKBAudibleBellMask;
	}
	else if ( strcmp(&argv[i][1],"autoautorepeat")==0 ) {
	    which|= XKBAutoAutorepeatMask;
	    if (onoff)	enabled|= XKBAutoAutorepeatMask;
	    else	enabled&= ~XKBAutoAutorepeatMask;
	}
	else if ( strcmp(&argv[i][1],"internal")==0 ) {
	    which|= XKBInternalModsMask;
	    if (((i+1)>=argc) ||
		(!parseMods(argv[i+1],onoff,&changeInternal,&internal))) {
		fprintf(stderr,"Must specify the internal modifiers\n");
		return 0;
	    }
	}
	else if ( strcmp(&argv[i][1],"ignorelock")==0 ) {
	    which|= XKBIgnoreLockModsMask;
	    if (((i+1)>=argc) || (!parseMods(argv[i+1],onoff,&changeIgnoreLocks,
							&ignoreLocks))) {
		fprintf(stderr,"Must specify the ignore lock modifiers\n");
		return 0;
	    }
	}
	else if ( strcmp(&argv[i][1],"repeat")==0 ) {
	    which|= XKBControlsEnabledMask;
	    changeEnabled|= XKBRepeatKeysMask;
	    enabled|= (onoff?XKBRepeatKeysMask:0);
	    if (onoff) {
		int ok= 0;
		if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		    which|= XKBRepeatKeysMask;
		    newControls.repeatDelay= atoi(argv[++i]);
		    ok= 1;
		}
		if (ok && ((i+1)<argc) && (isdigit(argv[i+1][0]))) {
		    newControls.repeatInterval= atoi(argv[++i]);
		}
	    }
	}
	else if ( strcmp(&argv[i][1],"slow")==0 ) {
	    which|= XKBControlsEnabledMask;
	    changeEnabled|= XKBSlowKeysMask;
	    enabled|= (onoff?XKBSlowKeysMask:0);
	    if (onoff) {
		if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		    which|= XKBSlowKeysMask;
		    newControls.slowKeysDelay= atoi(argv[++i]);
		}
	    }
	}
	else if ( strcmp(&argv[i][1],"bounce")==0 ) {
	    which|= XKBControlsEnabledMask;
	    changeEnabled|= XKBBounceKeysMask;
	    enabled|= (onoff?XKBBounceKeysMask:0);
	    if (onoff) {
		if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		    which|= XKBBounceKeysMask;
		    newControls.debounceDelay= atoi(argv[++i]);
		}
	    }
	}
	else if ( strcmp(&argv[i][1],"sticky")==0 ) {
	    which|= XKBControlsEnabledMask;
	    changeEnabled|= XKBStickyKeysMask;
	    enabled|= (onoff?XKBStickyKeysMask:0);
	}
	else if ( strcmp(&argv[i][1],"mouse")==0 ) {
	    which|= XKBControlsEnabledMask;
	    changeEnabled|= XKBMouseKeysMask;
	    enabled|= (onoff?XKBMouseKeysMask:0);
	    if (onoff) {
		int ok= 0;
		if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		    which|= XKBMouseKeysMask;
		    newControls.mouseKeysDelay= atoi(argv[++i]);
		    ok= 1;
		}
		if (ok && ((i+1)<argc) && (isdigit(argv[i+1][0]))) {
		    newControls.mouseKeysInterval= atoi(argv[++i]);
		}
		if (ok && ((i+1)<argc) && (isdigit(argv[i+1][0]))) {
		    newControls.mouseKeysTimeToMax= atoi(argv[++i]);
		}
		if (ok && ((i+1)<argc) && (isdigit(argv[i+1][0]))) {
		    newControls.mouseKeysCurve= atoi(argv[++i]);
		}
		if (ok && ((i+1)<argc) && (isdigit(argv[i+1][0]))) {
		    newControls.mouseKeysDfltBtn= atoi(argv[++i]);
		}
	    }
	}
	else if ( strcmp(&argv[i][1],"accessx")==0 ) {
	    which|= XKBControlsEnabledMask;
	    changeEnabled|= XKBAccessXKeysMask;
	    enabled|= (onoff?XKBAccessXKeysMask:0);
	    if (onoff) {
		if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		    which|= XKBAccessXKeysMask;
		    newControls.accessXTimeout= atoi(argv[++i]);
		}
	    }
	}
	else if ( strcmp(argv[i],"-help") ) {
	    return 0;
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
XKBControlsRec	*ctrls;
unsigned	 query;

  
    if (!parseArgs(argc,argv)) {
	usage(argc,argv);
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

    desc = XKBGetMap(dpy,0,device);
    if (desc) {
	if (!XKBGetControls(dpy,XKBAllControlsMask,desc)) {
	    fprintf(stderr,"XKBGetControls failed\n");
	    goto BAIL;
	}
	if (which) {
	    if (which&XKBRepeatKeysMask) {
		fprintf(stderr,"Changing RepeatKeys delay%s",
			newControls.repeatInterval>0?"and interval\n":"\n");
		desc->controls->repeatDelay= newControls.repeatDelay;
		if (newControls.repeatInterval>0)
		    desc->controls->repeatDelay= newControls.repeatInterval;
	    }
	    if (which&XKBSlowKeysMask) {
		fprintf(stderr,"Changing SlowKeys delay\n");
		desc->controls->slowKeysDelay= newControls.slowKeysDelay;
	    }
	    if (which&XKBBounceKeysMask) {
		fprintf(stderr,"Changing BounceKeys delay\n");
		desc->controls->debounceDelay= newControls.debounceDelay;
	    }
	    if (which&XKBMouseKeysMask) {
		fprintf(stderr,"Changing MouseKeys delay");
		desc->controls->mouseKeysDelay= newControls.mouseKeysDelay;
		if (newControls.mouseKeysInterval>0) {
		    fprintf(stderr,", interval");
		    desc->controls->mouseKeysInterval=
					newControls.mouseKeysInterval;
		}
		if (newControls.mouseKeysTimeToMax) {
		    fprintf(stderr,", time-to-max");
		    desc->controls->mouseKeysTimeToMax=
					newControls.mouseKeysTimeToMax;
		}
		if (newControls.mouseKeysTimeToMax) {
		    fprintf(stderr,", curve");
		    desc->controls->mouseKeysCurve= newControls.mouseKeysCurve;
		}
		if (newControls.mouseKeysDfltBtn) {
		    fprintf(stderr,", default button");
		    desc->controls->mouseKeysDfltBtn=
					newControls.mouseKeysDfltBtn;
		}
		fprintf(stderr,"\n");
	    }
	    if (which&XKBAccessXKeysMask) {
		fprintf(stderr,"Changing AccessX timeout\n");
		desc->controls->accessXTimeout= 
					newControls.accessXTimeout;
	    }
	    if (which&XKBControlsEnabledMask) {
		fprintf(stderr,"Changing enabled controls\n");
		desc->controls->enabledControls&= ~changeEnabled;
		desc->controls->enabledControls|= (changeEnabled&enabled);
	    }
	    if (which&XKBInternalModsMask) {
		fprintf(stderr,"Changing internal modifiers\n");
		desc->controls->internalMods&= ~changeInternal;
		desc->controls->internalMods|= (changeInternal|internal);
	    }
	    if (which&XKBIgnoreLockModsMask) {
		fprintf(stderr,"Changing ignore locks modifiers\n");
		desc->controls->ignoreLockMods&= ~changeIgnoreLocks;
		desc->controls->ignoreLockMods|=(changeIgnoreLocks|ignoreLocks);
	    }
	    XKBSetControls(dpy,which,desc);
	}
    }
    else {
	fprintf(stderr,"Get keyboard description request failed\n");
	return 1;
    }
    ctrls= desc->controls;
    printf("Device ID:        %d\n",desc->deviceSpec);
    printf("Groups:           %swrap\n",
		(ctrls->enabledControls&XKBGroupsWrapMask)?"":"don't ");
    printf("audible bell:     %s\n",
		(ctrls->enabledControls&XKBAudibleBellMask)?"on":"off");
    printf("auto autorepeat:  %s\n",
		(ctrls->enabledControls&XKBAutoAutorepeatMask)?"on":"off");
    printf("internal mods:    0x%x\n",ctrls->internalMods);
    printf("ignore lock mods: 0x%x\n",ctrls->ignoreLockMods);
    printf("repeat keys:      %s (%d/%d)\n",
		(ctrls->enabledControls&XKBRepeatKeysMask?"on":"off"),
		ctrls->repeatDelay,ctrls->repeatInterval);
    printf("slow keys:        %s (%d)\n",
		(ctrls->enabledControls&XKBSlowKeysMask?"on":"off"),
		ctrls->slowKeysDelay);
    printf("bounce keys:      %s (%d)\n",
		(ctrls->enabledControls&XKBBounceKeysMask?"on":"off"),
		ctrls->debounceDelay);
    printf("sticky keys:      %s\n",
		(ctrls->enabledControls&XKBStickyKeysMask?"on":"off"));
    printf("mouse keys:       %s (btn=%d,accel=%d/%d/%d/%d)\n",
		(ctrls->enabledControls&XKBMouseKeysMask?"on":"off"),
		ctrls->mouseKeysDfltBtn,
		ctrls->mouseKeysDelay,
		ctrls->mouseKeysInterval,
		ctrls->mouseKeysTimeToMax,
		ctrls->mouseKeysCurve);
    printf("access X keys:    %s (timeout=%d)\n",
		(ctrls->enabledControls&XKBAccessXKeysMask?"on":"off"),
		ctrls->accessXTimeout);
    XCloseDisplay(dpy);
    return 0;
BAIL:
    XCloseDisplay(dpy);
    return 0;
}
