#ifndef lint
static char rcsid[] = "$Header: ParseCmd.c,v 1.3 87/11/23 12:40:47 swick Locked $";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


/* XrmParseCommand.c 
   Parse command line and store argument values into resource database

   Allows any un-ambiguous abbreviation for an option name, but requires
   that the table be ordered with any options that are prefixes of
   other options appearing before the longer version in the table. */


#include "Xlibint.h"
#include "Xresource.h"
#include "Quarks.h"
#include <stdio.h>
#include <sys/types.h>


static void _XReportParseError(arg, msg)
    XrmOptionDescRec *arg;
    char *msg;
{
    (void) fprintf(stderr, "Error parsing argument \"%s\" (%s); %s\n",
		   arg->option, arg->resourceName, msg);
    exit(1);
}


void XrmParseCommand(rdb, table, tableCount, prependName, argc, argv)
    XrmResourceDataBase *rdb;		/* data base */
    XrmOptionDescList	table;	 	/* pointer to table of valid options */
    int			tableCount;	/* number of options		     */
    XrmAtom		prependName;	/* application name to prepend 	     */
    int			*argc;		/* address of argument count 	     */
    char		**argv;		/* argument list (command line)	     */
{
    int 		foundOption;
    static XrmQuark 	fullName[100];
    XrmQuark		*resourceName;
    char		**argsave;
    XrmValue	 	resourceValue;
    int 		i, myargc;
    char		*optP, *argP, optchar, argchar;
    int			matches;
    enum {DontCare, Check, NotSorted, Sorted} table_is_sorted;

    myargc = (*argc); 
    argsave = ++argv;
    if (prependName == NULLATOM) {
        resourceName = fullName;
    } else {
    	fullName[0] = XrmAtomToQuark(prependName);
	resourceName = &fullName[1];
    }

    table_is_sorted = (myargc > 2) ? Check : DontCare;
    for (--myargc; myargc>0;--myargc, ++argv) {
	foundOption = False;
	matches = 0;
	for (i=0; i < tableCount; ++i) {
	    /* checking the sort order first insures we don't have to
	       re-do the check if the arg hits on the last entry in
	       the table.  Useful because usually '=' is the last entry
	       and users frequently specify geometry early in the command */
	    if (table_is_sorted == Check && i > 0 &&
		strcmp(table[i].option, table[i-1].option) < 0) {
		table_is_sorted = NotSorted;
	    }
	    for (argP=*argv, optP=table[i].option;
		 (optchar = *optP++) != NULL &&
		 (argchar = *argP++) != NULL &&
		 argchar == optchar;);
	    if (optchar == NULL) {
		if (*argP == NULL ||
		    table[i].argKind == XrmoptionStickyArg ||
		    table[i].argKind == XrmoptionIsArg) {
		    /* give preference to exact matches, StickyArg and IsArg */
		    matches = 1;
		    foundOption = i;
		    break;
		}
	    }
	    else if (argchar == NULL) {
		/* may be an abbreviation for this option */
		matches++;
		foundOption = i;
	    }
	    else if (table_is_sorted == Sorted && optchar > argchar) {
		break;
	    }
	    if (table_is_sorted == Check && i > 0 &&
		strcmp(table[i].option, table[i-1].option) < 0) {
		table_is_sorted = NotSorted;
	    }
	}
	if (table_is_sorted == Check && i >= (tableCount-1))
	    table_is_sorted = Sorted;
	if (matches == 1) {
		i = foundOption;

		switch (table[i].argKind){
		case XrmoptionNoArg:
		    resourceValue.addr = table[i].value;
		    --(*argc);
		    resourceValue.size = strlen(resourceValue.addr)+1;
		    XrmStringToQuarkList(table[i].resourceName, resourceName);
		    XrmPutResource(rdb, fullName, XrmQString, &resourceValue);
		    break;
			    
		case XrmoptionIsArg:
		    resourceValue.addr = (caddr_t)((*argv));
		    --(*argc);
		    resourceValue.size = strlen(resourceValue.addr)+1;
		    XrmStringToQuarkList(table[i].resourceName, resourceName);
		    XrmPutResource(rdb, fullName, XrmQString, &resourceValue);
		    break;

		case XrmoptionStickyArg:
		    resourceValue.addr = (caddr_t)argP;
		    --(*argc);
		    resourceValue.size = strlen(resourceValue.addr)+1;
		    XrmStringToQuarkList(table[i].resourceName, resourceName);
		    XrmPutResource(rdb, fullName, XrmQString, &resourceValue);
		    break;

		case XrmoptionSepArg:
		    ++argv; --myargc; --(*argc); --(*argc);
		    resourceValue.addr = (caddr_t)(*argv);
		    resourceValue.size = strlen(resourceValue.addr)+1;
		    XrmStringToQuarkList(table[i].resourceName, resourceName);
		    XrmPutResource(rdb, fullName, XrmQString, &resourceValue);
		    break;
		
		case XrmoptionSkipArg:
		    --myargc;
		    (*argsave++) = (*argv++);
		    (*argsave++) = (*argv); 
		    break;

		case XrmoptionSkipLine:
		    for (;myargc>0;myargc--)
			(*argsave++) = (*argv++);
		    break;

		default:
		    _XReportParseError (&table[i], "unknown kind");
		    break;
		}
	}
	else
	    (*argsave++) = (*argv);  /*compress arglist*/ 
    }

    (*argsave)=NULL; /* put NULL terminator on compressed argv */
}
