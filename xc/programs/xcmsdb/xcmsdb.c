/* $XConsortium: xcmsdb.c,v 1.3 91/02/17 14:49:12 dave Exp $ */

/*
 * (c) Copyright 1990 Tektronix Inc.
 * 	All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Tektronix not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * Tektronix disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness, in no event shall
 * Tektronix be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of this
 * software.
 *
 *
 *	NAME
 *		xcmsdb.c
 *
 *	DESCRIPTION
 *		Program to load, query or remove the Screen Color 
 *		Characterization Data from the root window of the screen.
 *
 */

/*
 *      INCLUDES
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <ctype.h>

#include "Xcmsint.h"
#include "SCCDFile.h"

char *ProgramName;

void Syntax ()
{
    fprintf (stderr, 
	     "usage:  %s [-options ...] [filename]\n\n",
	     ProgramName);
    fprintf (stderr, 
	     "where options include:\n");
    fprintf (stderr, 
	     "    -display host:dpy[.scrn]            display to use\n");
    fprintf (stderr, 
	     "    -query                       query Screen Color Characterization Data\n");
    fprintf (stderr, 
	     "    -remove                      remove Screen Color Characterization Data\n");
    fprintf (stderr, 
	     "    -color                       use color as def<ault\n");
    fprintf (stderr, 
	     "    -gray                        use gray-scale as default\n");
    fprintf (stderr, 
	     "\n");
    exit (1);
}

static Bool optionmatch (opt, arg, minlen)
    char *opt;
    char *arg;
    int minlen;
{
    int arglen;

    if (strcmp(opt, arg) == 0) {
	return(True);
    }

    if ((arglen = strlen(arg)) >= (int)strlen(opt) || arglen < minlen) {
	return(False);
    }

    if (strncmp (opt, arg, arglen) == 0) {
	return(True);
    }
    
    return(False);
}

main (argc, argv)
    int argc;
    char **argv;
{
    Display *dpy;
    int i;
    char *displayname = NULL;
    char *filename = NULL;
    int query = 0;
    int remove = 0;
    int load = 0;
    int color = -1;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    if (arg[1] == '\0') {
		filename = NULL;
		continue;
	    } else if (optionmatch ("-help", arg, 1)) {
		Syntax ();
		/* doesn't return */
	    } else if (optionmatch ("-display", arg, 1)) {
		if (++i >= argc) Syntax ();
		displayname = argv[i];
		continue;
	    } else if (optionmatch ("-query", arg, 1)) {
		query = 1;
		continue;
	    } else if (optionmatch ("-remove", arg, 1)) {
		remove = 1;
		continue;
	    } else if (optionmatch ("-color", arg, 1)) {
		color = 1;
		continue;
	    } else if (optionmatch ("-gray", arg, 1)) {
		color = 0;
		continue;
	    }
	    Syntax ();
	} else {
	    load = 1;
	    filename = arg;
	}
    }

    /* Open display  */ 
    if (!(dpy = XOpenDisplay (displayname))) {
      fprintf (stderr, "%s:  Can't open display '%s'\n",
	       ProgramName, XDisplayName(displayname));
      exit (1);
    }

    if (load && query) {
	load = 0;
    }
    if (load && remove) {
	load = 0;
    }

    if (!query && !remove) {
	LoadSCCData(dpy, DefaultScreen(dpy), filename);
    }

    if (query) {
	if (color != 0)
	    QuerySCCDataRGB(dpy, RootWindow(dpy, DefaultScreen(dpy)));
	if (color != 1)
	    QuerySCCDataGray(dpy, RootWindow(dpy, DefaultScreen(dpy)));
    }

    if (remove) {
	RemoveSCCData(dpy, RootWindow(dpy, DefaultScreen(dpy)), color);
    }

    XCloseDisplay(dpy);
    exit (0);
}


Atom
ParseAtom (dpy, name, only_flag)
    Display *dpy;
    char    *name;
    int     only_flag;
{
    return(XInternAtom(dpy, name, only_flag));
}

/*
 *      NAME
 *		QuerySCCData - Query for the SCC data on the root window
 *
 *      SYNOPSIS
 */
int
QuerySCCDataRGB(dpy, root)
    Display *dpy;
    Window  root;
/*
 *      DESCRIPTION
 *
 *      RETURNS
 *		None
 */
{
    char *ret_prop, *pChar;
    int  i, j, hValue;
    int  ret_format, ret_len, ret_nbytes, cType, nTables, nElements;
    Atom MatricesAtom, CorrectAtom;
    XcmsFloat fValue;

    MatricesAtom = ParseAtom (dpy, XDCCC_MATRIX_ATOM_NAME, True);
    if (MatricesAtom != None) {
	if (_XcmsGetProperty (dpy, root, MatricesAtom, &ret_format, &ret_len,
			  &ret_nbytes, &ret_prop) == XCMS_FAILURE) {
	    ret_format = 0;
	} else if (ret_len != 18) {
	    printf ("Property %s had invalid length of %d\n",
		    XDCCC_MATRIX_ATOM_NAME, ret_len);
	    if (ret_prop) {
		XFree (ret_prop);
	    }
	    return;
	}
    } 
    if (MatricesAtom == None || !ret_format) {
	printf ("Could not find property %s\n", XDCCC_MATRIX_ATOM_NAME);
    } else {
	pChar = ret_prop;
	printf ("Querying property %s\n", XDCCC_MATRIX_ATOM_NAME);
	printf ("\tXYZtoRGB matrix :\n");
	for (i = 0; i < 3; i++) {
	    printf ("\t");
	    for (j = 0; j < 3; j++) {
		printf ("\t%8.5lf", 
			(XcmsFloat) _XcmsGetElement(ret_format, &pChar) / 
			(XcmsFloat) XDCCC_NUMBER);
	    }
	    printf ("\n");
	}
	printf ("\tRGBtoXYZ matrix :\n");
	for (i = 0; i < 3; i++) {
	    printf ("\t");
	    for (j = 0; j < 3; j++) {
		printf ("\t%8.5lf", 
			(XcmsFloat) _XcmsGetElement(ret_format, &pChar) / 
			(XcmsFloat) XDCCC_NUMBER);
	    }
	    printf ("\n");
	}
	XFree (ret_prop);
    }

    CorrectAtom = XInternAtom (dpy, XDCCC_CORRECT_ATOM_NAME, True);
    if (CorrectAtom != None) {
	if (_XcmsGetProperty (dpy, root, CorrectAtom, &ret_format, &ret_len,
			  &ret_nbytes, &ret_prop) == XCMS_FAILURE) {
	    ret_format = 0;
	} else if (ret_len <= 0) {
            printf ("Property %s had invalid length of %d\n",
		    XDCCC_CORRECT_ATOM_NAME, ret_len);
	    if (ret_prop) {
		XFree (ret_prop);
	    }
	    return;
	}
    }
    if (CorrectAtom == None || !ret_format) {
	printf ("Could not find property %s\n", XDCCC_CORRECT_ATOM_NAME);
    } else {
	printf ("Querying property %s\n", XDCCC_CORRECT_ATOM_NAME);
	pChar = ret_prop;
	cType = (int)_XcmsGetElement(ret_format, &pChar);
	nTables = (int)_XcmsGetElement(ret_format, &pChar);
	nElements = (int)_XcmsGetElement(ret_format, &pChar);

	if (cType == 0) {
	    /* Red Table should always exist */
	    printf ("\tRed Conversion Table:\n");
	    for (i = 0; i < nElements; i++) {
		hValue = (int) _XcmsGetElement(ret_format, &pChar);
		fValue = (XcmsFloat) _XcmsGetElement(ret_format, &pChar) /
			 (XcmsFloat) XDCCC_NUMBER;
		printf ("\t\t0x%x\t%8.5lf\n", hValue, fValue);
	    }
	    if (nTables > 1) {
		printf ("\tGreen Conversion Table:\n");
		for (i = 0; i < nElements; i++) {
		    hValue = (int) _XcmsGetElement(ret_format, &pChar);
		    fValue = (XcmsFloat)_XcmsGetElement(ret_format, &pChar)/
			     (XcmsFloat) XDCCC_NUMBER;
		    printf ("\t\t0x%4x\t%8.5lf\n", hValue, fValue);
		}
		printf ("\tBlue Conversion Table:\n");
		for (i = 0; i < nElements; i++) {
		    hValue = (int) _XcmsGetElement(ret_format, &pChar);
		    fValue = (XcmsFloat)_XcmsGetElement(ret_format, &pChar)/
			     (XcmsFloat) XDCCC_NUMBER;
		    printf ("\t\t0x%x\t%8.5lf\n", hValue, fValue);
		}
	    }
	} else {
	    /* Red Table should always exist */
	    printf ("\tRed Conversion Table:\n");
	    for (i = 0; i < nElements; i++) {
		fValue = (XcmsFloat) _XcmsGetElement(ret_format, &pChar) /
			     (XcmsFloat) XDCCC_NUMBER;
		printf ("\t\t%d\t%8.5lf\n", i, fValue);
	    }
	    if (nTables > 1) {
		printf ("\tGreen Conversion Table:\n");
		for (i = 0; i < nElements; i++) {
		    fValue = (XcmsFloat)_XcmsGetElement(ret_format, &pChar)/
			     (XcmsFloat) XDCCC_NUMBER;
		    printf ("\t\t%d\t%8.5lf\n", i, fValue);
		}
		printf ("\tBlue Conversion Table:\n");
		for (i = 0; i < nElements; i++) {
		    fValue = (XcmsFloat)_XcmsGetElement(ret_format, &pChar)/
			     (XcmsFloat) XDCCC_NUMBER;
		    printf ("\t\t%d\t%8.5lf\n", i, fValue);
		}
	    }
	}
	XFree (ret_prop);
    }    
}

/*
 *      NAME
 *		QuerySCCData - Query for the SCC data on the root window
 *
 *      SYNOPSIS
 */
int
QuerySCCDataGray(dpy, root)
    Display *dpy;
    Window  root;
/*
 *      DESCRIPTION
 *
 *      RETURNS
 *		None
 */
{
    char *ret_prop, *pChar;
    int  i, j, hValue;
    int  ret_format, ret_len, ret_nbytes, cType, nElements;
    Atom MatricesAtom, CorrectAtom;
    XcmsFloat fValue;

    MatricesAtom = ParseAtom (dpy, XDCCC_SCREENWHITEPT_ATOM_NAME, True);
    if (MatricesAtom != None) {
	if (_XcmsGetProperty (dpy, root, MatricesAtom, &ret_format, &ret_len,
			  &ret_nbytes, &ret_prop)  == XCMS_FAILURE) {
	    ret_format = 0;
	} else if (ret_len != 3) {
	    printf ("Property %s had invalid length of %d\n",
		    XDCCC_SCREENWHITEPT_ATOM_NAME, ret_len);
	    if (ret_prop) {
		XFree (ret_prop);
	    }
	    return;
	}
    } 
    if (MatricesAtom == None || !ret_format) {
	printf ("Could not find property %s\n", XDCCC_SCREENWHITEPT_ATOM_NAME);
    } else {
	pChar = ret_prop;
	printf ("Querying property %s\n", XDCCC_SCREENWHITEPT_ATOM_NAME);
	printf ("\tWhite Point XYZ :\n");
	printf ("\t");
	for (j = 0; j < 3; j++) {
	    printf ("\t%8.5lf", 
			(XcmsFloat) _XcmsGetElement(ret_format, &pChar) / 
			(XcmsFloat) XDCCC_NUMBER);
	}
	printf ("\n");
	XFree (ret_prop);
    }

    CorrectAtom = XInternAtom (dpy, XDCCC_GRAY_CORRECT_ATOM_NAME, True);
    if (CorrectAtom != None) {
	if (_XcmsGetProperty (dpy, root, CorrectAtom, &ret_format, &ret_len,
			  &ret_nbytes, &ret_prop) == XCMS_FAILURE) {
	    ret_format = 0;
	} else if (ret_len <= 0) {
            printf ("Property %s had invalid length of %d\n",
		    XDCCC_GRAY_CORRECT_ATOM_NAME, ret_len);
	    if (ret_prop) {
		XFree (ret_prop);
	    }
	    return;
	}
    }
    if (CorrectAtom == None || !ret_format) {
	printf ("Could not find property %s\n", XDCCC_GRAY_CORRECT_ATOM_NAME);
    } else {
	printf ("Querying property %s\n", XDCCC_GRAY_CORRECT_ATOM_NAME);
	pChar = ret_prop;
	cType = (int)_XcmsGetElement(ret_format, &pChar);
	nElements = (int)_XcmsGetElement(ret_format, &pChar);

	if (cType == 0) {
	    /* One Table should always exist */
	    printf ("\tGray Conversion Table:\n");
	    for (i = 0; i < nElements; i++) {
		hValue = (int) _XcmsGetElement(ret_format, &pChar);
		fValue = (XcmsFloat) _XcmsGetElement(ret_format, &pChar) /
			 (XcmsFloat) XDCCC_NUMBER;
		printf ("\t\t0x%x\t%8.5lf\n", hValue, fValue);
	    }
	} else {
	    /* One Table should always exist */
	    printf ("\tGray Conversion Table:\n");
	    for (i = 0; i < nElements; i++) {
		fValue = (XcmsFloat) _XcmsGetElement(ret_format, &pChar) /
			     (XcmsFloat) XDCCC_NUMBER;
		printf ("\t\t%d\t%8.5lf\n", i, fValue);
	    }
	}
	XFree (ret_prop);
    }    
}

/*
 *      NAME
 *		RemoveSCCData - Remove for the SCC data on the root window
 *
 *      SYNOPSIS
 */
int
RemoveSCCData(dpy, root, colorFlag)
    Display *dpy;
    Window  root;
    int     colorFlag;
/*
 *      DESCRIPTION
 *
 *      RETURNS
 *		None
 */
{
    unsigned char *ret_prop;
    unsigned long ret_len, ret_after;
    int  ret_format;
    Atom MatricesAtom, CorrectAtom, ret_atom;

    if (colorFlag != 0) {
	MatricesAtom = ParseAtom (dpy, XDCCC_MATRIX_ATOM_NAME, True);
	if (MatricesAtom != None) {
	    XGetWindowProperty (dpy, root, MatricesAtom, 0, 8192, False,
			XA_INTEGER, &ret_atom, &ret_format, &ret_len, 
			&ret_after, &ret_prop);
	} 
	if (MatricesAtom == None || !ret_format) {
	    printf ("Could not find property %s\n", XDCCC_MATRIX_ATOM_NAME);
	} else {
	    printf ("Deleting property %s\n", XDCCC_MATRIX_ATOM_NAME);
	    XDeleteProperty (dpy, root, MatricesAtom);
	    XFree ((char *)ret_prop);
	}

	CorrectAtom = XInternAtom (dpy, XDCCC_CORRECT_ATOM_NAME, True);
	if (CorrectAtom != None) {
	    XGetWindowProperty (dpy, root, CorrectAtom, 0, 8192, False, 
			XA_INTEGER, &ret_atom, &ret_format, &ret_len, 
			&ret_after, &ret_prop);
	}
	if (CorrectAtom == None || !ret_format) {
	    printf ("Could not find property %s\n", XDCCC_CORRECT_ATOM_NAME);
	} else {
	    printf ("Deleting property %s\n", XDCCC_CORRECT_ATOM_NAME);
	    XDeleteProperty (dpy, root, CorrectAtom);
	    XFree ((char *)ret_prop);
	}
    }
    if (colorFlag != 1) {
	MatricesAtom = ParseAtom (dpy, XDCCC_SCREENWHITEPT_ATOM_NAME, True);
	if (MatricesAtom != None) {
	    XGetWindowProperty (dpy, root, MatricesAtom, 0, 8192, False,
			XA_INTEGER, &ret_atom, &ret_format, &ret_len, 
			&ret_after, &ret_prop);
	} 
	if (MatricesAtom == None || !ret_format) {
	    printf ("Could not find property %s\n", XDCCC_SCREENWHITEPT_ATOM_NAME);
	} else {
	    printf ("Deleting property %s\n", XDCCC_SCREENWHITEPT_ATOM_NAME);
	    XDeleteProperty (dpy, root, MatricesAtom);
	    XFree ((char *)ret_prop);
	}

	CorrectAtom = XInternAtom (dpy, XDCCC_GRAY_CORRECT_ATOM_NAME, True);
	if (CorrectAtom != None) {
	    XGetWindowProperty (dpy, root, CorrectAtom, 0, 8192, False, 
			XA_INTEGER, &ret_atom, &ret_format, &ret_len, 
			&ret_after, &ret_prop);
	}
	if (CorrectAtom == None || !ret_format) {
	    printf ("Could not find property %s\n", XDCCC_GRAY_CORRECT_ATOM_NAME);
	} else {
	    printf ("Deleting property %s\n", XDCCC_GRAY_CORRECT_ATOM_NAME);
	    XDeleteProperty (dpy, root, CorrectAtom);
	    XFree ((char *)ret_prop);
	}
    }
}
