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

/***====================================================================***/

static	char		*dpyName = NULL;
static	int		 changeAll = 0;
static	int		 synch = 0;
static	unsigned	 full;
static	XKBMapChangesRec changes;
static	unsigned	whichNames = XKBAllNamesMask;
static	CARD8		firstNamesKeyType = 0;
static	CARD8		nNamesKeyTypes = 0;

int
parseArgs(int argc,char *argv[])
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
	else if ((strcmp(argv[i],"-t")==0) || (strcmp(argv[i],"-types")==0)){
	    if ((i+1<argc)&&(argv[i+1][0]!='-')) {
		int low,high;
		if (sscanf(argv[++i],"%i-%i",&low,&high)==2) {
		    full&= ~XKBKeyTypesMask;
		    changes.changed|= XKBKeyTypesMask;
		    changes.firstKeyType = low;
		    changes.nKeyTypes = high-low+1;
		    continue;
		}
		fprintf(stderr,"must specify a range of key types\n");
		return 0;
	    }
	    else full|= XKBKeyTypesMask;
	}
	else if ((strcmp(argv[i],"-s")==0) || (strcmp(argv[i],"-syms")==0)){
	    if ((i+1<argc)&&(argv[i+1][0]!='-')) {
		int low,high;
		if (sscanf(argv[++i],"%i-%i",&low,&high)!=2) {
		    full&= ~XKBKeySymsMask;
		    changes.changed|= XKBKeySymsMask;
		    changes.firstKeySym = low;
		    changes.nKeySyms = high-low+1;
		    continue;
		}
		fprintf(stderr,"must specify a range of key syms\n");
		return 0;
	    }
	    else full|= XKBKeySymsMask;
	}
	else if ((strcmp(argv[i],"-a")==0) || (strcmp(argv[i],"-actions")==0)){
	    if ((i+1<argc)&&(argv[i+1][0]!='-')) {
		int low,high;
		if (sscanf(argv[++i],"%i-%i",&low,&high)!=2) {
		    full&= ~XKBKeyActionsMask;
		    changes.changed|= XKBKeyActionsMask;
		    changes.firstKeyAction = low;
		    changes.nKeyActions = high-low+1;
		    continue;
		}
		fprintf(stderr,"must specify a range of actions\n");
		return 0;
	    }
	    else full|= XKBKeyActionsMask;
	}
	else if ((strcmp(argv[i],"-b")==0)||(strcmp(argv[i],"-behaviors")==0)){
	    if ((i+1<argc)&&(argv[i+1][0]!='-')) {
		int low,high;
		if (sscanf(argv[++i],"%i-%i",&low,&high)!=2) {
		    full&= ~XKBKeyBehaviorsMask;
		    changes.changed|= XKBKeyBehaviorsMask;
		    changes.firstKeyBehavior = low;
		    changes.nKeyBehaviors = high-low+1;
		    continue;
		}
		fprintf(stderr,"must specify a range of key behaviors\n");
		return 0;
	    }
	    else full|= XKBKeyBehaviorsMask;
	}
	else if ((strcmp(argv[i],"-x")==0)||(strcmp(argv[i],"-exclude")==0)) {
	    if ((i+1<argc) && (argv[i+1][0]!='-')) {
		char *subsets = argv[++i];
		unsigned which = 0;
		while (*subsets) {
		    switch (*subsets) {
			case 't':	which|= XKBKeyTypesMask; break;
			case 'a':	which|= XKBKeyActionsMask; break;
			case 's':	which|= XKBKeySymsMask; break;
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
		else {
		    changes.changed&= ~which;
		}
	    }
	    else {
		fprintf(stderr,"%s option requires an argument\n",argv[i]);
		return 0;
	    }
	}
	else if ((!strcmp(argv[i],"-nx"))||(!strcmp(argv[i],"-excludenames"))) {
	    if ((i+1<argc) && (argv[i+1][0]!='-')) {
		char *subsets = argv[++i];
		unsigned which = 0;
		while (*subsets) {
		    switch (*subsets) {
			case 's':	which|= XKBSymbolsNameMask; break;
			case 'c':	which|= XKBKeycodesNameMask; break;
			case 'g':	which|= XKBGeometryNameMask; break;
			case 'm':	which|= XKBModifierNamesMask; break;
			case 'l':	which|= XKBKTLevelNamesMask; break;
			default:
			    fprintf(stderr,"Unknown subset %c\n",*subsets);
			    return 0;
		    }
		    subsets++;
		} 
		whichNames&= which;
	    }
	    else {
		fprintf(stderr,"%s option requires an argument\n",argv[i]);
		return 0;
	    }
	}
	else if ((!strcmp(argv[i],"-ktn")) || (!strcmp(argv[i],"-KTlnames"))){
	    if ((i+1<argc)&&(argv[i+1][0]!='-')) {
		int low,high;
		if (sscanf(argv[++i],"%i-%i",&low,&high)!=2) {
		    firstNamesKeyType = low;
		    nNamesKeyTypes = high-low+1;
		    if (nNamesKeyTypes>0)
			continue;
		}
		fprintf(stderr,"must specify a legal range of key types\n");
		return 0;
	    }
	    else full|= XKBKeyActionsMask;
	}
	else if ( strcmp(argv[i],"-synch")==0 ) {
	    synch= 1;
	}
	else if ( strcmp(argv[i],"-all")==0 ) {
	    changeAll= 1;
	}
	else {
	    fprintf(stderr,"Unknown option %s\n",argv[i]);
	    return 0;
	}
    }
    return 1;
}

int
main(int argc,char *argv[])
{
Display	*dpy;
int	i1,i2;
extern	Bool	XKBQueryExtension(Display *,int *,int *);
XKBDescRec	*map;
XKBStateRec	 state;
unsigned	 query;

  
    if (!parseArgs(argc,argv)) {
	fprintf(stderr,"Usage: %s <options>\n",argv[0]);
	fprintf(stderr,"Where legal options are:\n");
	fprintf(stderr,"-display <dpy>         specifies display to use\n");
	fprintf(stderr,"-t[ypes] [range]       specifies a range of key types to refresh\n");
	fprintf(stderr,"-s[yms] [range]        specifies a range of key syms to refresh\n");
	fprintf(stderr,"-a[ctions] [range]     specifies a range of actions to refresh\n");
	fprintf(stderr,"-b[ehaviors] [range]   specifies a range of key behaviors to refresh\n");
	fprintf(stderr,"-x <subsets>           specifies unchanged subset of keyboard mapping\n");
	fprintf(stderr,"                       Legal subsets are:\n");
	fprintf(stderr,"		       t:  Key Types\n");
	fprintf(stderr,"                       a:  Key Actions\n");
	fprintf(stderr,"                       s:  Key Syms\n");
	fprintf(stderr,"                       b:  Key Behavior\n");
	fprintf(stderr,"-xn <name subsets>     specifies unchanges subset of keyboard names\n");
	fprintf(stderr,"                       legal name subsets are:\n");
	fprintf(stderr,"                       c:  Keycode name\n");
	fprintf(stderr,"                       s:  Symbols name\n");
	fprintf(stderr,"                       g:  Geometry name\n");
	fprintf(stderr,"                       i:  Indicator names\n");
	fprintf(stderr,"                       m:  Modifier names\n");
	fprintf(stderr,"                       l:  Key type level names\n");
	fprintf(stderr,"-cmn [range]           specifies a range of column map names to refresh\n");
	return 1;
    }
    dpy = XOpenDisplay(dpyName);
    if ( !dpy )
	return 1;
    XSynchronize(dpy,1);
    if ( !XKBQueryExtension(dpy,&i1,&i2)>0 ) {
	fprintf(stderr,"query failed\n");
	goto BAIL;
    }
    if ( !XKBUseExtension(dpy,&i1,&i2) ) {
	fprintf(stderr,"use extension failed (%d,%d)\n",i1,i2);
	goto BAIL;
    }

    map = XKBGetMap(dpy,XKBAllMapComponentsMask,XKB_USE_CORE_KBD);
    if (!map) {
	fprintf(stderr,"XKBGetMap failed\n");
	goto BAIL;
    }
    if (!XKBGetNames(dpy,XKBAllNamesMask,map)) {

	fprintf(stderr,"XKBGetNames failed\n");
	goto BAIL;
    }
    if ((!changeAll)&&(!changes.changed)) {
	changeAll = 1;
	full = XKBAllMapComponentsMask;
    }
    if (changeAll) {
	printf("Refreshing entire mapping...");
	if (XKBSetMap(dpy,full,map))
	     printf("done\n");
	else printf("XKBSetMap failed!\n");
    }
    else {
	printf("Refreshing:");
	if (changes.changed&XKBKeyTypesMask) {
	    if ((changes.firstKeyType==0)&&(changes.nKeyTypes==0))
	    	changes.nKeyTypes = map->map->nKeyTypes;
	    printf("   column maps:      %d-%d\n",changes.firstKeyType,
	    						changes.nKeyTypes);
	}
	if (changes.changed&XKBKeySymsMask) {
	    if ((changes.firstKeySym==0)&&(changes.nKeySyms==0)) {
		changes.firstKeySym = map->minKeyCode;
	    	changes.nKeySyms = map->maxKeyCode-map->minKeyCode+1;
	    }
	    printf("   key syms:         %d-%d\n",changes.firstKeySym,
	    						changes.nKeySyms);
	}
	if (changes.changed&XKBKeyActionsMask) {
	    if ((changes.firstKeyAction==0)&&(changes.nKeyActions==0)) {
		changes.firstKeyAction = map->minKeyCode;
	    	changes.nKeyActions = map->maxKeyCode-map->minKeyCode+1;
	    }
	    printf("   key actions:      %d-%d\n",changes.firstKeyAction,
	    						changes.nKeyActions);
	}
	if (changes.changed&XKBKeyBehaviorsMask) {
	    if ((changes.firstKeyBehavior==0)&&(changes.nKeyBehaviors==0)) {
		changes.firstKeyBehavior = map->minKeyCode;
	    	changes.nKeyBehaviors = map->maxKeyCode-map->minKeyCode+1;
	    }
	    printf("   key behaviors:    %d-%d\n",changes.firstKeyBehavior,
	    						changes.nKeyBehaviors);
	}
	if (XKBChangeMap(dpy,map,&changes))
	     printf("Map refreshed\n");
	else printf("XKBChangeMap failed!\n");
    }
    if (whichNames) {
	printf("Refreshing names...");
	if (nNamesKeyTypes==0) {
	    firstNamesKeyType = 0;
	    nNamesKeyTypes= map->map->nKeyTypes;
	}
	XKBSetNames(dpy,whichNames,firstNamesKeyType,nNamesKeyTypes,map);
	printf("done\n");
    }
    return 0;
BAIL:
    XCloseDisplay(dpy);
    return 0;
}
