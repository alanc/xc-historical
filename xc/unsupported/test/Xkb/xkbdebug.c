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

static	char		*dpyName = NULL;
static	unsigned	 flags = 0;

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
	else if ( strcmp(argv[i],"-flags")==0 ) {
	    if ( ++i<argc ) {
		if (sscanf(argv[i],"%i",&flags)!=1) {
		    fprintf(stderr,"Flags specification must be an integer\n");
		    return 0;
		}
	    }
	    else {
		fprintf(stderr,"Must specify a flags with -device option\n");
		return 0;
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
unsigned	query;

  
    if (!parseArgs(argc,argv)) {
	fprintf(stderr,"Usage: %s <options>\n",argv[0]);
	fprintf(stderr,"Where legal options are:\n");
	fprintf(stderr,"-display <dpy>     specifies display to use\n");
	fprintf(stderr,"-flags   <flags>   specifies new debugging flags\n");
	return 1;
    }
    dpy = XOpenDisplay(dpyName);
    if ( !dpy )
	return 1;
    if ( !XKBQueryExtension(dpy,&i1,&i2)>0 ) {
	fprintf(stderr,"query failed\n");
	goto BAIL;
    }
    if ( !XKBUseExtension(dpy,&i1,&i2) ) {
	fprintf(stderr,"use extension failed (%d,%d)\n",i1,i2);
	goto BAIL;
    }
    XSynchronize(dpy,1);
    XKBSetDebuggingFlags(dpy,flags);
    XCloseDisplay(dpy);
    return 0;
BAIL:
    XCloseDisplay(dpy);
    return 0;
}
