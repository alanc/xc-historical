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

#include "Xlib.h"
#include "Xlibint.h"
#include "Xresource.h"
#include <stdio.h>

void XrmParseCommand(pdb, options, num_options, prefix, argc, argv)
    XrmDatabase		*pdb;		/* data base */
    register XrmOptionDescList options; /* pointer to table of valid options */
    int			num_options;	/* number of options		     */
    char		*prefix;	/* name to prefix resources with     */
    int			*argc;		/* address of argument count 	     */
    char		**argv;		/* argument list (command line)	     */
{
    Bool 		foundOption;
    char		**argsave;
    register int	i, j, myargc;
    XrmBinding		bindings[100];
    XrmQuark		quarks[100];
    XrmBinding		*start_bindings;
    XrmQuark		*start_quarks;

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

    for (--myargc; myargc > 0; --myargc, ++argv) {
	foundOption = False;
	for (i=0; (! foundOption) && i < num_options; ++i) {
	    foundOption = True;
	    for (j = 0; options[i].option[j] != NULL; ++j) {
		if (options[i].option[j] != (*argv)[j]) {
		    foundOption = False;
		    break;
		}
	    }

	    if (foundOption
	     && ((options[i].argKind == XrmoptionStickyArg)
	         || (options[i].argKind == XrmoptionIsArg)
		 || ((*argv)[j] == NULL))) {
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
		    PutCommandResource((*argv)+j);
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
		    _XReportParseError (options[i].argKind);
		    break;
		}
	    } else foundOption = False;

	}
	
	if (! foundOption) 
	    (*argsave++) = (*argv);  /*compress arglist*/ 
    }

    (*argsave)=NULL; /* put NULL terminator on compressed argv */
}

void exit();
int _XReportParseError(arg)
    char arg[];
{
	(void) fprintf(stderr, "Error parsing argument %s", arg);
	exit(1);
}
