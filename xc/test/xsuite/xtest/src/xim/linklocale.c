/* $XConsortium$ */
/*
 * Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.
 *
 *                   All Rights Reserved
 *
 * Permission  to  use,  copy,  modify,  and  distribute   this
 * software  and  its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright no-
 * tice  appear  in all copies and that both that copyright no-
 * tice and this permission notice appear in  supporting  docu-
 * mentation,  and  that the names of Sun or MIT not be used in
 * advertising or publicity pertaining to distribution  of  the
 * software  without specific prior written permission. Sun and
 * M.I.T. make no representations about the suitability of this
 * software for any purpose. It is provided "as is" without any
 * express or implied warranty.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
 * NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
 * ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
 * PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include		<stdio.h>
#include    <string.h>
#include    "xtest.h"
#include    "Xlib.h"
#include    "Xutil.h"
#include    "Xresource.h"
#include    "xtestlib.h"
#include    "tet_api.h"
#include    "pixval.h"
#include    "ximtest.h"

#define  BUF_LEN  512

extern   int   tet_thistest;
extern   struct   tet_testlist tet_testlist[];

int
linklocale(plocale)
char *plocale;
{
int ic;
char name1[128];
char name2[128];
FILE *fp1;
FILE *fp2;
char buf[BUF_LEN];

	if(config.debug_no_pixcheck)
		return(True);

   ic = tet_testlist[tet_thistest-1].icref;
   (void) sprintf(name1, "a%d.%s.dat", ic, plocale);
   (void) sprintf(name2, "a%d.dat", ic);

	fp1 = fopen(name1, "r");
	if (fp1 == NULL)
	{
		report("Could not open data file for locale %s", plocale);
		return(False);
	}
	fp2 = fopen(name2, "a+");
	if (fp2 == NULL)
	{
		report("Could not open data file %s", name2);
		return(False);
	}

	while (fgets(buf, BUF_LEN, fp1) != NULL) 
	{
		(void)fputs(buf, fp2);
	}
	
	fclose(fp1);
	fclose(fp2);
}

void
unlinklocales()
{
int ic;
char name[128];

	if(config.debug_no_pixcheck) return;

   ic = tet_testlist[tet_thistest-1].icref;
   (void) sprintf(name, "a%d.dat", ic);

	(void) unlink(name);
}
