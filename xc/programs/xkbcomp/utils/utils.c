
  /*\
   * $XConsortium: utils.c,v 1.1 94/04/02 17:12:03 erik Exp $
   *
   *		              COPYRIGHT 1990
   *		        DIGITAL EQUIPMENT CORPORATION
   *		           MAYNARD, MASSACHUSETTS
   *			    ALL RIGHTS RESERVED.
   *
   * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
   * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
   * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE 
   * FOR ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED 
   * WARRANTY.
   *
   * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
   * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
   * ADDITION TO THAT SET FORTH ABOVE.
   *
   * Permission to use, copy, modify, and distribute this software and its
   * documentation for any purpose and without fee is hereby granted, provided
   * that the above copyright notice appear in all copies and that both that
   * copyright notice and this permission notice appear in supporting
   * documentation, and that the name of Digital Equipment Corporation not be
   * used in advertising or publicity pertaining to distribution of the 
   * software without specific, written prior permission.
  \*/

#include 	"utils.h"
#include	<ctype.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
char *malloc();
#endif

/***====================================================================***/

Opaque
uAlloc(size)
    unsigned	size;
{
    return((Opaque)malloc(size));
}

/***====================================================================***/

Opaque
uCalloc(n,size)
    unsigned	n;
    unsigned	size;
{
    return((Opaque)calloc(n,size));
}

/***====================================================================***/

Opaque
uRealloc(old,newSize)
    Opaque	old;
    unsigned	newSize;
{
    if (old==NULL)
	 return((Opaque)malloc(newSize));
    else return((Opaque)realloc((char *)old,newSize));
}

/***====================================================================***/

Opaque
uRecalloc(old,nOld,nNew,itemSize)
    Opaque	old;
    unsigned	nOld;
    unsigned	nNew;
    unsigned	itemSize;
{
char *rtrn;

    if (old==NULL)
	 rtrn= (char *)calloc(nNew,itemSize);
    else {
	rtrn= (char *)realloc((char *)old,nNew*itemSize);
   	if ((rtrn)&&(nNew>nOld)) {
	    bzero(&rtrn[nOld*itemSize],(nNew-nOld)*itemSize);
	}
    }
    return (Opaque)rtrn;
}

/***====================================================================***/

void
uFree(ptr)
    Opaque ptr;
{
    if (ptr!=(Opaque)NULL)
	free((char *)ptr);
    return;
}

/***====================================================================***/
/***                  FUNCTION ENTRY TRACKING                           ***/
/***====================================================================***/

static	FILE	*entryFile=	stderr;
	int	 uEntryLevel;

Boolean
uSetEntryFile(name)
    char *name;
{
    if ((entryFile!=NULL)&&(entryFile!=stderr)) {
	fprintf(entryFile,"switching to %s\n",name?name:"stderr");
	fclose(entryFile);
    }
    if (name!=NullString)	entryFile=	fopen(name,"w");
    else			entryFile=	stderr;
    if (entryFile==NULL) {
	entryFile=	stderr;
	return(False);
    }
    return(True);
}

void
uEntry(l,s,a1,a2,a3,a4,a5,a6,a7,a8)
int	l;
char	*s;
Opaque	a1,a2,a3,a4,a5,a6,a7,a8;
{
int	i;

    for (i=0;i<uEntryLevel;i++) {
	putc(' ',entryFile);
    }
    fprintf(entryFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    uEntryLevel+= l;
    return;
}

void
uExit(l,rtVal)
    int		l;
    char *	rtVal;
{
int	i;

    uEntryLevel-= l;
    if (uEntryLevel<0)	uEntryLevel=	0;
    for (i=0;i<uEntryLevel;i++) {
	putc(' ',entryFile);
    }
    fprintf(entryFile,"---> 0x%x\n",rtVal);
    return;
}

/***====================================================================***/
/***			PRINT FUNCTIONS					***/
/***====================================================================***/

	FILE	*uDebugFile=		stderr;
	int	 uDebugIndentLevel=	0;
	int	 uDebugIndentSize=	4;

Boolean
uSetDebugFile(name)
    char *name;
{
    if ((uDebugFile!=NULL)&&(uDebugFile!=stderr)) {
	fprintf(uDebugFile,"switching to %s\n",name?name:"stderr");
	fclose(uDebugFile);
    }
    if (name!=NullString)	uDebugFile=	fopen(name,"w");
    else			uDebugFile=	stderr;
    if (uDebugFile==NULL) {
	uDebugFile=	stderr;
	return(False);
    }
    return(True);
}

void
uDebug(s,a1,a2,a3,a4,a5,a6,a7,a8)
char *s;
Opaque a1,a2,a3,a4,a5,a6,a7,a8;
{
int	i;

    for (i=(uDebugIndentLevel*uDebugIndentSize);i>0;i--) {
	putc(' ',uDebugFile);
    }
    fprintf(uDebugFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    fflush(uDebugFile);
    return;
}

void
uDebugNOI(s,a1,a2,a3,a4,a5,a6,a7,a8)
char *s;
Opaque a1,a2,a3,a4,a5,a6,a7,a8;
{
int	i;

    fprintf(uDebugFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    fflush(uDebugFile);
    return;
}

/***====================================================================***/

static	FILE	*errorFile=	stderr;

Boolean
uSetErrorFile(name)
    char *name;
{
    if ((errorFile!=NULL)&&(errorFile!=stderr)) {
	fprintf(errorFile,"switching to %s\n",name?name:"stderr");
	fclose(errorFile);
    }
    if (name!=NullString)	errorFile=	fopen(name,"w");
    else			errorFile=	stderr;
    if (errorFile==NULL) {
	errorFile=	stderr;
	return(False);
    }
    return(True);
}

void
uInformation(s,a1,a2,a3,a4,a5,a6,a7,a8)
char *s;
Opaque a1,a2,a3,a4,a5,a6,a7,a8;
{
    fprintf(errorFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    fflush(errorFile);
    return;
}

/***====================================================================***/

void
uAction(s,a1,a2,a3,a4,a5,a6,a7,a8)
char *s;
Opaque a1,a2,a3,a4,a5,a6,a7,a8;
{
    fprintf(errorFile,"                  ");
    fprintf(errorFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    fflush(errorFile);
    return;
}

/***====================================================================***/

void
uWarning(s,a1,a2,a3,a4,a5,a6,a7,a8)
char *s;
Opaque a1,a2,a3,a4,a5,a6,a7,a8;
{
    fprintf(errorFile,"Warning:          ");
    fprintf(errorFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    fflush(errorFile);
    return;
}

/***====================================================================***/

void
uError(s,a1,a2,a3,a4,a5,a6,a7,a8)
char *s;
Opaque a1,a2,a3,a4,a5,a6,a7,a8;
{
    fprintf(errorFile,"Error:            ");
    fprintf(errorFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    fflush(errorFile);
    return;
}

/***====================================================================***/

void
uFatalError(s,a1,a2,a3,a4,a5,a6,a7,a8)
char *s;
Opaque a1,a2,a3,a4,a5,a6,a7,a8;
{
    fprintf(errorFile,"Fatal Error:      ");
    fprintf(errorFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    fprintf(errorFile,"                  Exiting\n");
    fflush(errorFile);
    exit(1);
    /* NOTREACHED */
}

/***====================================================================***/

void
uInternalError(s,a1,a2,a3,a4,a5,a6,a7,a8)
char *s;
Opaque a1,a2,a3,a4,a5,a6,a7,a8;
{
    fprintf(errorFile,"Internal error:   ");
    fprintf(errorFile,s,a1,a2,a3,a4,a5,a6,a7,a8);
    fflush(errorFile);
    return;
}

/***====================================================================***/

#ifndef HAVE_STRDUP
char *
uStringDup(str)
    char *str;
{
char *rtrn;

    if (str==NULL)
	return NULL;
    rtrn= (char *)uAlloc(strlen(str)+1);
    strcpy(rtrn,str);
    return rtrn;
}
#endif

#ifndef HAVE_STRCASECMP
int
uStrCaseCmp(str1, str2)
    char *str1, *str2;
{
    char str[512];
    char c, *s;

    for (s = str; c = *str1++; ) {
	if (isupper(c))
	    c = tolower(c);
	*s++ = c;
    }
    *s = '\0';
    return (strcmp(str, str2));
}
#endif
