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

char *
atomText(Display *dpy,Atom atom)
{
static char buf[256];
char	*name = XGetAtomName(dpy,atom);

    if ( atom==None ) {
	buf[0]= '\0';
    }
    else {
	if ( name ) {
	    strncpy(buf,name,255);
	    buf[255]= '\0';
	    XFree(name);
	}
	else {
	    sprintf(buf,"(unknown)");
	}
    }
    return buf;
}

char *
indComponentText(CARD8 which)
{
register int i;
static char buf[8];
char *str;

    if (which) {
	str= buf;
	if (which&XKB_IMUseBase)
	    *str++= 'b';
	if (which&XKB_IMUseLatched)
	    *str++= 'l';
	if (which&XKB_IMUseLocked)
	    *str++= 'L';
	if (which&XKB_IMUseEffective)
	    *str++= 'e';
	if (which&XKB_IMUseCompat)
	    *str++= 'c';
	*str++= '\0';
    }
    else strcpy(buf,"none");
    return buf;
}

/***====================================================================***/

static	char		*dpyName = NULL;
static	int		 showMapping;
static	int		 synch = 0;

static	CARD32			changed;
static	XKBIndicatorMapRec	map[XKB_NUM_INDICATORS];

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
	else if ( strcmp(argv[i],"-synch")==0 ) {
	    synch= 1;
	}
	else if ( strcmp(argv[i],"-v")==0 ) {
	    showMapping = 1;
	}
	else if ( isdigit(argv[i][0]) ) {
	    char buf[20],*tmp;
	    int  led,mods,group,ctrls,which;
	    int	   got;
	    led= mods= group= ctrls= which= 0;
	    got= sscanf(argv[i],"%i %s %i %i %i",&led,buf,&mods,&group,&ctrls);
	    if (got<2) {
		fprintf(stderr,"Couldn't parse map for indicator %d\n",led);
		return 0;
	    }
	    if ((led<0)||(led>=XKB_NUM_INDICATORS)) {
		fprintf(stderr,"Illegal led %d (must be 0..%d)\n",led,
							XKB_NUM_INDICATORS-1);
		return 0;
	    }
	    tmp= buf;
	    while (*tmp) {
		if (*tmp=='b')		which|= XKB_IMUseBase;
		else if (*tmp=='l')	which|= XKB_IMUseLatched;
		else if (*tmp=='L')	which|= XKB_IMUseLocked;
		else if (*tmp=='e')	which|= XKB_IMUseEffective;
		else if (*tmp=='c')	which|= XKB_IMUseCompat;
		else if (*tmp=='x')	which= 0;
		else {
		    fprintf(stderr,"Unknown specifier '%c' ignored\n",*tmp);
		}
		tmp++;
	    }
	    changed|= (1<<led);
	    map[led].whichMods= which;
	    map[led].mods= mods;
	    map[led].whichGroups= which;
	    map[led].groups= group;
	    map[led].controls= ctrls;
	}
	else {
	    fprintf(stderr,"Unknown option %s\n",argv[i]);
	    return 0;
	}
    }
    return 1;
}

void
showMaps(Display *dpy,XKBDescRec *desc)
{
register int i;
CARD8	*action;
char	*name;
XKBIndicatorRec *leds;
XKBNamesRec	*names;

    leds= desc->indicators;
    names= desc->names;
    printf("%d physical indicators\n",leds->nRealIndicators);
    for (i=0;i<XKB_NUM_INDICATORS;i++) {
	if ((leds->maps[i].whichMods==0)&&(leds->maps[i].whichGroups==0)&&
					  (leds->maps[i].controls==0))
	    continue;
	printf("Indicator %d",i);
	if (names&&(name=atomText(dpy,names->indicators[i]))&&(name[0]))
	    printf(" (%s)",name);
	printf(":\n");
	printf("   mods:     0x%x (%s)\n",leds->maps[i].mods,
				indComponentText(leds->maps[i].whichMods));
	printf("   groups:   0x%x (%s)\n",leds->maps[i].groups,
				indComponentText(leds->maps[i].whichGroups));
	printf("   controls: 0x%x\n",leds->maps[i].controls);
    }
    return;
}

int
main(int argc,char *argv[])
{
Display	*dpy;
int	i1,i2;
extern	Bool	XKBQueryExtension(Display *,int *,int *);
XKBDescRec	*desc;
unsigned	 	 query;

  
    if (!parseArgs(argc,argv)) {
	fprintf(stderr,"Usage: %s <options>\n",argv[0]);
	fprintf(stderr,"Where legal options are:\n");
	fprintf(stderr,"-display <dpy>     specifies display to use\n");
	fprintf(stderr,"-v        show current indicator mapping\n");
	fprintf(stderr,"-synch    turn on synchronization\n");
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
    desc = XKBGetMap(dpy,0,XKB_USE_CORE_KBD);
    if (!desc) {
	fprintf(stderr,"XKBGetMap failed\n");
	goto BAIL;
    }

    if (!XKBGetIndicatorMap(dpy,0xFFFFFFFF,desc)) {
	fprintf(stderr,"GetIndicatorMapping failed\n");
	goto BAIL;
    }
    if ( showMapping )
	showMaps(dpy,desc);
    if (changed) {
	for (i1=0,i2=1;i1<XKB_NUM_INDICATORS;i1++,i2<<=1) {
	    if (changed&i2)
		desc->indicators->maps[i1]= map[i1];
	}
	XKBSetIndicatorMap(dpy,changed,desc);
	XSync(dpy,0);
    }
BAIL:
    XCloseDisplay(dpy);
    return 0;
}
