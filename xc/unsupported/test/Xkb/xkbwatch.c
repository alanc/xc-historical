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
#define NEED_EVENTS
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/extensions/XKBstr.h>
#include <X11/extensions/XKBlib.h>

static	char		*dpyName = NULL;
static	unsigned	 device = XKB_USE_CORE_KBD;
static	unsigned	 which = XKBAllStateComponentsMask;
static	int		 page_size = 24;

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
	else if ( strcmp(argv[i],"-device")==0 ) {
	    if ( ++i<argc ) {
		if (sscanf(argv[i],"%i",&device)!=1) {
		    fprintf(stderr,"Device specification must be an integer\n");
		    return 0;
		}
	    }
	    else {
		fprintf(stderr,"Must specify a device with -device option\n");
		return 0;
	    }
	}
	else if ( strcmp(argv[i],"-page")==0 ) {
	    if ( ++i<argc ) {
		if ((sscanf(argv[i],"%i",&page_size)!=1)||(i<1)) {
		    fprintf(stderr,"Device specification must be a positive integer\n");
		    return 0;
		}
	    }
	    else {
		fprintf(stderr,"Must specify a device with -device option\n");
		return 0;
	    }
	}
	else if (strlen(argv[i])==2) {
	    switch (argv[i][1]) {
		case 'a': case 'A':
		    if (argv[i][0]=='-')      which&=~XKBAllStateComponentsMask;
		    else if (argv[i][0]=='+') which|= XKBAllStateComponentsMask;
		    else			return 0;
		    break;
		case 's': case 'S':
		    if (argv[i][0]=='-')	which&= ~XKBModifierStateMask;
		    else if (argv[i][0]=='+')	which|= XKBModifierStateMask;
		    else			return 0;
		    break;
		case 'g': 
		    if (argv[i][0]=='-')	which&= ~XKBGroupStateMask;
		    else if (argv[i][0]=='+')	which|= XKBGroupStateMask;
		    else			return 0;
		    break;
		case 'G': 
		    if (argv[i][0]=='-')	which&= ~XKBGroupLatchMask;
		    else if (argv[i][0]=='+')	which|= XKBGroupLatchMask;
		    else			return 0;
		    break;
		case 'l':
		    if (argv[i][0]=='-')	which&= ~XKBModifierLockMask;
		    else if (argv[i][0]=='+')	which|= XKBModifierLockMask;
		    else			return 0;
		    break;
		case 'L':
		    if (argv[i][0]=='-')	which&= ~XKBModifierLatchMask;
		    else if (argv[i][0]=='+')	which|= XKBModifierLatchMask;
		    else			return 0;
		    break;
		case 'c': case 'C':
		    if (argv[i][0]=='-')	which&= ~XKBCompatStateMask;
		    else if (argv[i][0]=='+')	which|= XKBCompatStateMask;
		    else			return 0;
		    break;
	    }
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
int		ev_base,num_out;
XEvent		xev;
XKBStateNotifyEvent	*sn;

  
    if (!parseArgs(argc,argv)) {
	fprintf(stderr,"Usage: %s <options>\n",argv[0]);
	fprintf(stderr,"Where legal options are:\n");
	fprintf(stderr,"-display <dpy>     specifies display to use\n");
	fprintf(stderr,"-device <id>       specifies device to use\n");
	fprintf(stderr,"-page              specifies lines per output page\n");
	fprintf(stderr,"[+-][aA]           watch/ignore all components\n");
	fprintf(stderr,"[+-][cC]           watch/ignore compatibility state\n");
	fprintf(stderr,"[+-]g              watch/ignore keyboard group\n");
	fprintf(stderr,"[+-]G              watch/ignore latched group\n");
	fprintf(stderr,"[+-]l              watch/ignore latched modifiers\n");
	fprintf(stderr,"[+-]L              watch/ignore locked modifiers\n");
	fprintf(stderr,"[+-][sS]           watch/ignore modifier state\n");
	return 1;
    }
    if ( which==0 ) {
	fprintf(stderr,"You must specify at least one component to watch\n");
	exit(1);
    }
    dpy = XOpenDisplay(dpyName);
    if ( !dpy )
	return 1;
    if ( !XKBQueryExtension(dpy,&ev_base,&i2)>0 ) {
	fprintf(stderr,"query failed\n");
	goto BAIL;
    }
    if ( !XKBUseExtension(dpy,&i1,&i2) ) {
	fprintf(stderr,"use extension failed (%d,%d)\n",i1,i2);
	goto BAIL;
    }
    XSynchronize(dpy,1);
    XKBSelectEventDetails(dpy,device,XKBStateNotify,which,which);
    num_out = 0;
    printf("Watching the keyboard state...\n");
    while (1) {
	XNextEvent(dpy,&xev);
	if (xev.type==XKBStateNotify+ev_base) {
	    if ((num_out%page_size)==0) {
		printf("                ------- group --------  ------- modifiers -------------\n");
		printf("id key   event  eff  base  latch  lock   eff  base  latch  lock  compat\n");
	    }
	    num_out++;
	    sn = (XKBStateNotifyEvent *)&xev;
		printf("%2d  %2d",sn->device, sn->keycode);
		if (sn->keycode!=0)
		     printf(" %7s",(sn->eventType==KeyPress?"down":"up"));
		else {
		    char buf[30];
		    sprintf(buf,"%d/%d",sn->requestMajor,sn->requestMinor);
		    printf(" %7s",buf);
		}
		printf("   %2d%c   %2d%c    %2d%c   %2d%c",
				sn->group,
				(sn->changed&XKBGroupStateMask?'*':' '),
				sn->baseGroup,
				(sn->changed&XKBGroupBaseMask?'*':' '),
				sn->latchedGroup,
				(sn->changed&XKBGroupLatchMask?'*':' '),
				sn->lockedGroup,
				(sn->changed&XKBGroupLockMask?'*':' '));
		printf(" 0x%02x%c 0x%02x%c  0x%02x%c 0x%02x%c   0x%02x%c\n",
				sn->mods,
				(sn->changed&XKBModifierStateMask?'*':' '),
				sn->baseMods,
				(sn->changed&XKBModifierBaseMask?'*':' '),
				sn->latchedMods,
				(sn->changed&XKBModifierLatchMask?'*':' '),
				sn->lockedMods,
				(sn->changed&XKBModifierLockMask?'*':' '),
				sn->compatState,
				(sn->changed&XKBCompatStateMask?'*':' '));
	}
	else {
	    fprintf(stderr,"Unknown event type %d\n",xev.type);
	}
    }
    XCloseDisplay(dpy);
    return 0;
BAIL:
    XCloseDisplay(dpy);
    return 0;
}
