#ifndef lint
static char rcsid[] = "$Header$";
#endif lint

/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**/

/* XrmParseCommand()

   Parse command line and store argument values into resource database

   Allows any un-ambiguous abbreviation for an option name, but requires
   that the table be ordered with any options that are prefixes of
   other options appearing before the longer version in the table.
*/

#include <X/Xlib.h>
#include "Xlibint.h"
#include <X/Xresource.h>
#include <stdio.h>


static void _XReportParseError(arg, msg)
    XrmOptionDescRec *arg;
    char *msg;
{
    (void) fprintf(stderr, "Error parsing argument \"%s\" (%s); %s\n",
		   arg->option, arg->specifier, msg);
    exit(1);
}

void XrmParseCommand(pdb, options, num_options, prefix, argc, argv)
    XrmDatabase		*pdb;		/* data base */
    register XrmOptionDescList options; /* pointer to table of valid options */
    int			num_options;	/* number of options		     */
    char		*prefix;	/* name to prefix resources with     */
    int			*argc;		/* address of argument count 	     */
    char		**argv;		/* argument list (command line)	     */
{
    int 		foundOption;
    char		**argsave;
    register int	i, myargc;
    XrmBinding		bindings[100];
    XrmQuark		quarks[100];
    XrmBinding		*start_bindings;
    XrmQuark		*start_quarks;
    char		*optP, *argP, optchar, argchar;
    int			matches;
    enum {DontCare, Check, NotSorted, Sorted} table_is_sorted;

#define PutCommandResource(value_str)				\
{								\
    XrmStringToBindingQuarkList(				\
	options[i].specifier, start_bindings, start_quarks);    \
    XrmQPutStringResource(pdb, bindings, quarks, value_str);    \
} /* PutCommandResource */

    myargc = (*argc); 
    argsave = ++argv;

    /* Parse prefix into bindings and quark list */
    XrmStringToBindingQuarkList(prefix, bindings, quarks);
    for (start_bindings = bindings, start_quarks = quarks;
	 *start_quarks != NULLQUARK;
	 start_bindings++, start_quarks++) {};

    table_is_sorted = (myargc > 2) ? Check : DontCare;
    for (--myargc; myargc > 0; --myargc, ++argv) {
	foundOption = False;
	matches = 0;
	for (i=0; i < num_options; ++i) {
	    /* checking the sort order first insures we don't have to
	       re-do the check if the arg hits on the last entry in
	       the table.  Useful because usually '=' is the last entry
	       and users frequently specify geometry early in the command */
	    if (table_is_sorted == Check && i > 0 &&
		strcmp(options[i].option, options[i-1].option) < 0) {
		table_is_sorted = NotSorted;
	    }
	    for (argP = *argv, optP = options[i].option;
		 (optchar = *optP++) != NULL &&
		 (argchar = *argP++) != NULL &&
		 argchar == optchar;);
	    if (optchar == NULL) {
		if (*argP == NULL ||
		    options[i].argKind == XrmoptionStickyArg ||
		    options[i].argKind == XrmoptionIsArg) {
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
		strcmp(options[i].option, options[i-1].option) < 0) {
		table_is_sorted = NotSorted;
	    }
	}
	if (table_is_sorted == Check && i >= (num_options-1))
	    table_is_sorted = Sorted;
	if (matches == 1) {
		i = foundOption;
		switch (options[i].argKind){
		case XrmoptionNoArg:
		    --(*argc);
		    PutCommandResource(options[i].value);
		    break;
			    
		case XrmoptionIsArg:
		    --(*argc);
		    PutCommandResource(*argv);
		    break;

		case XrmoptionStickyArg:
		    --(*argc);
		    PutCommandResource(argP);
		    break;

		case XrmoptionSepArg:
		    --(*argc);
		    ++argv; --myargc; --(*argc);
		    PutCommandResource(*argv);
		    break;
		
		case XrmoptionResArg:
		    ++argv; --myargc; --(*argc); --(*argc);
		    XrmPutLineResource(pdb, *argv);
		    break;
		
		case XrmoptionSkipArg:
		    --myargc;
		    (*argsave++) = (*argv++);
		    (*argsave++) = (*argv); 
		    break;

		case XrmoptionSkipLine:
		    for (; myargc > 0; myargc--)
			(*argsave++) = (*argv++);
		    break;

		default:
		    _XReportParseError (&options[i], "unknown kind");
		    break;
		}
	}
	else
	    (*argsave++) = (*argv);  /*compress arglist*/ 
    }

    (*argsave)=NULL; /* put NULL terminator on compressed argv */
}
