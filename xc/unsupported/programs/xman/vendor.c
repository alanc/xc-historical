/*
 * $XConsortium: vendor.c,v 1.8 92/11/21 13:41:38 rws Exp $
 *
 * Copyright 1992 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* Vendor-specific data structures and operations */

#include "globals.h"
#include "vendor.h"

typedef struct sectionName {
    char *	name;
    char *	suffix;
} SectionNameRec;

#if defined(SYSV) || defined(__osf__)

static SectionNameRec SectionNames[] = {
    {"(1) User Commands",		"1"},
    {"(1m) Sys, Administration",	"1m"},
    {"(2) System Calls",		"2"},
    {"(3) Subroutines",			"3"},
    {"(4) File Formats",		"4"},
    {"(5) Miscellaneous",		"5"},
    {"(6) Games",			"6"},
    {"(7) Devices",			"7"},
    {"(8) Sys. Administration",		"8"},
    {"(l) Local",			"l"},
    {"(n) New",				"n"},
    {"(o) Old",				"o"}
};

#else

#if defined(sun) && defined(SVR4)
/* losing man.cf is useless because it has no section names */

static SectionNameRec SectionNames[] = {
    {"(1)  User Commands",			"1"},
    {"(1b) SunOS/BSD Compatibility Commands",	"1b"},
    {"(1c) Communication Commands",		"1c"},
    {"(1f) FMLI Commands",			"1f"},
    {"(1m) Maintenance Commands",		"1m"},
    {"(1s) SunOS Specific Commands",		"1s"},
    {"(2)  System Calls",			"2"},
    {"(3)  C Library Functions",		"3"},
    {"(3b) SunOS/BSD Compatibility Functions",	"3b"},
    {"(3c) C Library Functions",		"3c"},
    {"(3e) ELF Library Functions",		"3e"},
    {"(3g) C Library Functions",		"3g"},
    {"(3i) Wid Character Functions",		"3i"},
    {"(3k) Kernel VM Library Functions",	"3k"},
    {"(3m) Mathematical Library",		"3m"},
    {"(3n) Network Functions",			"3n"},
    {"(3r) RPC Services Library",		"3r"},
    {"(3s) Standard I/O Functions",		"3s"},
    {"(3x) Miscellaneous Library Functions",	"3x"},
    {"(4)  File Formats",			"4"},
    {"(4b) Misc. Reference Manual Pages",	"4b"},
    {"(5)  Environments, Tables, and TROFF Macros", "5"},
    {"(6)  Games and Demos",			"6"},
    {"(7)  Special Files",			"7"},
    {"(9)  Device Driver Information",		"9"},
    {"(9e) DDI and DKI Driver Entry Points",	"9e"},
    {"(9f) DDI and DKI Kernel Functions",	"9f"},
    {"(9s) DDI and DKI Data Structures",	"9s"},
    {"(l)  Local",				"l"},
    {"(n)  New",				"n"},
};

#else

static SectionNameRec SectionNames[] = {
    {"(1) User Commands",		"1"},
    {"(2) System Calls",		"2"},
    {"(3) Subroutines",			"3"},
    {"(4) Devices",			"4"},
    {"(5) File Formats",		"5"},
    {"(6) Games",			"6"},
    {"(7) Miscellaneous",		"7"},
    {"(8) Sys. Administration",		"8"},
    {"(l) Local",			"l"},
    {"(n) New",				"n"},
    {"(o) Old",				"o"},
#ifdef sony
    {"(p) Public Domain",		"p"},
    {"(s) Sony Specific",		"s"},
#endif
};

#endif
#endif

/*	Function Name: AddStandardSections
 *	Description: Adds all the standard sections to the list for this path.
 *	Arguments: list - a pointer to the section list.
 *                 path - the path to these standard sections.
 *	Returns: none.
 */

void AddStandardSections(list, path)
SectionList **list;
char * path;
{
#ifdef CRAY
    AddStandardCraySections(list, path);
    return;
#else
  register int i;
  char file[BUFSIZ];
  int numSections = sizeof(SectionNames) / sizeof(SectionNames[0]);

  for (i=0; i < numSections; i++) {
    sprintf(file, "%s%s", SEARCHDIR, SectionNames[i].suffix);
    AddNewSection(list, path, file, SectionNames[i].name, TRUE);
#ifdef SEARCHOTHER
    sprintf(file, "%s%s", SEARCHOTHER, SectionNames[i].suffix);
    AddNewSection(list, path, file, SectionNames[i].name, TRUE);
#endif
  }
#endif
}

#ifdef CRAY
/*	Function Name: AddStandardCraySections
 *	Description: Add sections specific to the Cray.
 *	Arguments: list - a pointer to the section list.
 *                 path - the path to these standard sections.
 *                 names - standard section names.
 *	Returns: none.
 */

AddStandardCraySections(list, path)
SectionList **list;
char *path;
{
  char file[BUFSIZ];
  int i;
#define NoSuf  (char *)0
#define Suffix (char *)1
#define Fold (char *)2
#define FoldSuffix (char *)3
  static char *cname[] = {
    "(1) User Commands",	Suffix,	"1",	"1bsd",	NULL,
    "(1) User Commands (instant)",	NoSuf,	"1r",	"1rb",	NULL,
    "(1m) System Administration",	NoSuf,	"1m",	NULL,
    "(2) System Calls",		Suffix,	"2",	NULL,
    "(3) Subroutines",		FoldSuffix,	"3",	"3bsd",	"3c",	"3m",	"3mt",	"3s",	"3sl",	"3z",	NULL,
    "(3) Subroutines (FORTRAN)", Fold,	"3f",	NULL,
    "(3) Subroutines (I/O)",	Fold,	"3io",	NULL,
    "(3) Subroutines (X11)",	NoSuf,	"3X11",	NULL,
    "(3) Subroutines (Xt)",	NoSuf,	"3Xt",	NULL,
    "(3) Subroutines (misc.)",	Suffix,	"3q",	NULL,
    "(3) Subroutines (misc.)",	Fold,	"3x",	NULL,
    "(3) Subroutines (networking)",	Suffix,	"3n",	"3rpc",	"3svc",	"3w",	"3yp",	NULL,
    "(3) Subroutines (scientific)",	Fold,	"3sci",	NULL,
    "(3) Subroutines (utilities)",	FoldSuffix,	"3db",	"3u",	NULL,
    "(4) Devices",		Suffix,	"4",	"4d",	"4f",	"4n",	"4p",	"4s",	NULL,
    "(5) File Formats",		Suffix,	"5",	NULL,
    "(6) Games",		Suffix,	"6",	NULL,
    "(7) Miscellaneous",	Suffix,	"7",	NULL,
    "(8) Sys. Administration",	NoSuf,	"8",	NULL,
    "(l) Local",		Suffix,	"l",	NULL,
    "(n) New",			Suffix,	"n",	NULL,
    "(o) Old",			Suffix,	"o",	NULL,
    "(info) Information",	NoSuf,	"info",	NULL,
    "(osi) Miscellaneous",	NoSuf,	"osi",	NULL,
    "(sl) Miscellaneous",	NoSuf,	"sl",	NULL,
    "(ultra) Miscellaneous",	NoSuf,	"ultra",	NULL,
    NULL
  };
  char **p = cname;

  while (*p != NULL) {
    char *message = *p++;
    int flags = (int) *p++;
    while (*p != NULL) {
      sprintf(file, "%s%s", SEARCHDIR, *p++);
      AddNewSection(list, path, file, message, flags);
    }
    p++;
  }
}
#endif /* CRAY */


/*	Function Name: CreateManpageName
 *	Description: Creates the manual page name for a given item.
 *	Arguments: entry - the entry to convert.
 *	Returns: the manual page properly allocated.
 */

/*
 * If the filename is foo.3     - Create an entry of the form:  foo
 * If the filename is foo.3X11  - Create an entry of the form:  foo(X11)
 * IF the filename is a.out.1   - Create an entry of the form:  a.out
 */

char *
CreateManpageName(entry, section, flags)
char * entry;
int section;
int flags;
{
  char * cp;
  char *p;
  char page[BUFSIZ];
  char sect[BUFSIZ];

  ParseEntry(entry, NULL, sect, page);

#ifndef CRAY
  if ( (cp = rindex(page, '.')) != NULL)
    if ( (int)strlen(cp) > 2 ) {
      *cp++ = '(';
      while( (cp[1] != '\0') ) {
	*cp = *(cp + 1); 
	cp++;
      }
      *cp++ = ')';
      *cp = '\0';
    }
    else
      *cp = '\0';

#else	/* CRAY	- pick up the Cray name from the section */

  if ( (cp = rindex(page, '.')) == NULL)
    cp = page + strlen(page);
  if ((flags & MSUFFIX) && strlen(sect) > 4) {
    p = sect + 4;
    *cp++ = '(';
    while (*p)
      *cp++ = *p++;
    *cp++ = ')';
  }
  *cp = '\0';  

#endif	/* CRAY */
  
  return(StrAlloc(page));
}
