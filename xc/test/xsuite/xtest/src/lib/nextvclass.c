/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */
/*
 * Functions to cycle through all the visual classes that are supposed to
 * supported by the display/screen that is being tested.
 * Note that these functions are only used in the tests for 
 * XMatchVisualInfo and XGetVisualInfo.
 * Other tests call functions in nextvinf.c, which use XGetVisualInfo, and 
 * rely on XGetVisualInfo to work properly.
 */

#include "xtest.h"
#include "stdlib.h"
#include "string.h"
#include "tet_api.h"
#include "Xlib.h"
#include "Xutil.h"
#include "xtestlib.h"
#include "pixval.h"

/*
 * Table and index into it.
 */
static	int 	*Vclass;
static	int 	*Vdepth;
static	int 	VCindex;
static	int 	Nclass;

/*
 * Items are separated by spaces.
 */
#define SEPS		" "
#define LEFT_BRACKET	'('
#define RIGHT_BRACKET	')'
#define COMMA		','

#define	CLASSBUFLEN	40

static char classbuf[CLASSBUFLEN+1];

static struct valname S_displayclass[] = {
	StaticGray, "StaticGray",
	GrayScale, "GrayScale",
	StaticColor, "StaticColor",
	PseudoColor, "PseudoColor",
	TrueColor, "TrueColor",
	DirectColor, "DirectColor",
	0, (char*)0,
};

/*
 * Initialise the visual class table.  The visual classes that are
 * supported are supplied by the test suite user in the
 * variable XT_VISUAL_CLASSES, together with the depths at which 
 * they are supported.
 * Returns -1 on failure, 0 on success.
 */
int
initvclass()
{
struct	valname *vp;
char	*dp;
char	*tok, *s, *left_bracket, *right_bracket, *comma;
int	Lclass;

	dp = config.visual_classes;
	if (dp == (char*)0) {
		report("XT_VISUAL_CLASSES not set");
		return(-1);
	}

	/*
	 * The number of visual classes in the string is less than the
	 * length of the string.
	 */
	Vclass = (int*)malloc(strlen(dp) * sizeof(int));
	if (Vclass == (int*)0) {
		report("Could not allocate memory for visual class table");
		return(-1);
	}
	Vdepth = (int*)malloc(strlen(dp) * sizeof(int));
	if (Vdepth == (int*)0) {
		report("Could not allocate memory for visual class table");
		return(-1);
	}

	Nclass = 0;
	for (tok = strtok(dp, SEPS); tok; tok = strtok((char*)0, SEPS)) {

		strncpy(classbuf, tok, CLASSBUFLEN);
		classbuf[CLASSBUFLEN] = '\0';
		if (!(left_bracket = strchr(classbuf, LEFT_BRACKET))) {
			report("Display class %s does not contain a '('", tok);
			return(-1);
		} else
			*left_bracket = '\0';
		if (!(right_bracket = strchr(left_bracket+1, RIGHT_BRACKET))) {
			report("Display class %s does not contain a ')'", tok);
			return(-1);
		} else
			*right_bracket = '\0';
		for (vp = S_displayclass; vp->name; vp++) {
			if (strcmp(vp->name, classbuf) == 0)
				break;
		}
		if (vp->name == 0) {
			report("Bad display class %s in config file", tok);
			return(-1);
		}
		Lclass = Nclass;
		for (s = left_bracket; s; s = comma) {
			if (comma = strchr(++s, COMMA))
				*comma = '\0';
			if (*s == '\0')
				break;
			Vclass[Nclass] = vp->val;
			Vdepth[Nclass++]   = atov(s);
		}
		if (Nclass <= Lclass) {
			report("Display class %s specifies no depths", tok);
			return(-1);
		}
	}

	return(0);

}

/*
 * Start again at the beginning of the list of visual classes.
 */
void
resetvclass()
{
	VCindex = 0;
}

/*
 * Get the next visual class and depth.  
 * Function returns False if there are no more in the list, 
 * otherwise True.
 */
int
nextvclass(vp, dp)
int 	*vp;
int 	*dp;
{
	if (Vclass == 0) {
		report("initvclass has not been called");
		tet_result(TET_UNRESOLVED);
		return(False);
	}

	if (VCindex >= Nclass)
		return(False);

	*vp = Vclass[VCindex];
	*dp = Vdepth[VCindex++];

	return(True);
}

int
nvclass()
{
	return(Nclass);
}
