/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: man.c,v 1.1 88/08/31 22:52:42 jim Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   August 10, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: man.c,v 4.0 88/08/31 22:12:33 kit Exp $";
#endif

#include "globals.h"

static void SetSectionNames();
static int GetSectNumber();
static void CheckMandesc();

static char error_buf[BUFSIZ];		/* The buffer for error messages. */

/*	Function Name: CmpEntryLabel - used in qsort().
 *	Description: compares to elements by using their labels.
 *	Arguments: e1, e2 - two elements to compare by label.
 *	Returns: an integer >, < or = 0.
 */

static int 
CmpEntryLabel(e1, e2) 
struct entry *e1, *e2;
{
  return(strcmp(e1->label, e2->label));
}

/*	Function Name: Man
 *	Description: Builds a list of all manual directories and files.
 *	Arguments: none. 
 *	Returns: the number of manual sections.
 */

int
Man()
{
  char *man_nomatch[MAXSECT + 1]; /* allow NULL termination. */
  char *manptr, manualdir[BUFSIZ], *local_ptr;
  int i,current[MAXSECT],nsect;

  nsect = FIXEDSECT;

  for (i = 0 ; i < MAXSECT ; i ++) {
    current[i] = 0;		/* set current entries = 0. */
    if ( (manual[i].entries = 
	  (struct entry *) malloc(MAXENTRY * sizeof(struct entry))) == NULL )
      PrintError("Could not allocate memory, while building manpage list.");
    
    manual[i].blabel = "Dummy Section Label";
    manual[i].longest = 1;
  }

  SetManNames(manual);
  manptr = getenv("MANPATH");
  if (manptr == NULL || strcmp(manptr,"") == 0) {
#ifdef DEBUG  /* Since man does not complain we had better not either. */
    sprintf(error_buf,"Could not find MANPATH searching only, %s.",MANDIR);
    PrintWarning(error_buf);
#endif
    strcpy(manualdir,MANDIR);
  }
  else
    strcpy(manualdir,manptr);

  local_ptr = manualdir;
  do {
    char *man[MAXSECT];
    char *curmandir;
    if ( (manptr = index(local_ptr,':')) != NULL) {    
      *manptr = '\0';
      curmandir = local_ptr;
      local_ptr = manptr + 1;
    }
    else
      curmandir = local_ptr;

/*    if (!Preformatted(curmandir, manual, current, nsect)) { */
      if (GetEntry(curmandir,man_nomatch)) {
	MatchEntries(curmandir,manual,man_nomatch,man,&nsect);
	AddStruct(manual,curmandir,man,current,nsect);
      }
/*    } */

  } while (manptr != NULL);

  SetSectionNames(manual);
  SortAndRemove(manual, nsect);
#ifdef notdef
  DumpManual(nsect);
#endif
  return(nsect);		/* return the number of man sections. */
}    

/*	Function Name: GetEntry
 *	Description: This function gets the names of the manual page
 *                   directories, then closes the directory.
 *	Arguments: path - the path to this directory.
 *                 man - The directries of unformatted manual pages.
 *	Returns: FALSE if directory could not be opened.
 */

static Boolean
GetEntry(path, man)
char * path;
char * man[MAXSECT];
{
  DIR * dir;
  struct stat sb;
  register struct direct *dp;
  int numberman;
  char full_path[BUFSIZ];

  numberman = 0;

  /*
   * Read through this directory and save up any directory name
   * which begins with "man".
   */
  
  if((dir = opendir(path)) == NULL) {	
    sprintf(error_buf,"Can't open directory %s", path);
    PrintWarning(error_buf);
    return(FALSE);
  }

  while((dp = readdir(dir)) != NULL) {
    if((dp->d_namlen >= LSEARCHDIR) && 
       (strncmp(dp->d_name, SEARCHDIR, LSEARCHDIR) == 0)) {
      if(numberman == MAXSECT) 
	PrintError(
	  "Too many manual sections, recompile with larger allocations");
      sprintf(full_path, "%s/%s", path, dp->d_name);
      if((stat(full_path, &sb) >= 0) && (sb.st_mode & S_IFDIR))
	man[numberman++] = StrAlloc(dp->d_name);
    }
  }
  
  man[numberman] = NULL;	/* NULL terminate this list. */
  closedir(dir);
  return(TRUE);
}

/*	Function Name: MatchEntries
 *	Description: This fucntion gives the correct number to the 
 *                   directory, so that all manl entries will be 
 *                   in the same place.
 *	Arguments: path - path name if the current searched directory.
 *                 manual - a pointer to the manual structure.
 *                 manin - the unmatched states of the directories.
 *                 manout - the new wonderfully matched directories.
 *                 number - a pointer to the current number of sections.
 *	Returns: manout.
 */

void
MatchEntries(path,manual,manin,manout,number)
char * path;
Manual * manual;
char **manin;
char **manout;
int *number;
{
  int sect;

  bzero((char *) manout, MAXSECT * sizeof(char *));

  for ( ; *manin != NULL ; manin++) {
    if ( (sect = GetSectNumber( (*manin)[LMAN] )) >= 0) 
      manout[sect] = *manin;
  }

  CheckMandesc(path, manout, manual, number);
}

/*	Function Name: CheckMandesc
 *	Description: Reads the mandesc file, and adds more sections as 
 *                   nescessary.
 *	Arguments: path - path name if the current searched directory.
 *                 manual - a pointer to the manual structure.
 *                 man - directories, now sorted by order.
 *                 number - a pointer to the current number of sections.
 *	Returns: none
 */
  
static void
CheckMandesc(path, manout, manual, number)
int *number;
char * path;
char ** manout;
Manual * manual;
{
  char mandesc_file[BUFSIZ];	/* full path to the mandesc file. */
  FILE * descfile;
  char character;
  char string[BUFSIZ];
  int sectnum;
  register int j;

  sprintf(mandesc_file, "%s/%s", path, MANDESC);
  if ( (descfile = fopen(mandesc_file, "r")) == NULL) 
    return;			/* if no description file we are done. */
  while ( (character = getc(descfile)) != EOF) {
    Boolean flag = TRUE;

    fgets(string, 100, descfile);
    string[strlen(string)-1] = '\0';        /* Strip off the CR. */
    if ( (sectnum = GetSectNumber(character)) >= 0) {
      if (manout[sectnum] == NULL) {
	sprintf(error_buf, "Error in file: %s.", mandesc_file);
	PrintWarning(error_buf);
	sprintf(error_buf, "Could not find the directory %s/man%c.", 
		path, character);
	PrintWarning(error_buf);
	continue;
      }
      for (j = FIXEDSECT ; j < *number ; j++) {
/*
 * If this section exists then do not create a new one for it.
 */
	if ( (!strcmp(manual[j].blabel , string)) ) {
	  manout[j] = manout[sectnum];
	  flag = FALSE;
	}
      }
      if (flag) {
	manual[*number].blabel = StrAlloc(string);
	manual[*number].sect = StrAlloc(manout[sectnum]);
	manout[*number] = StrAlloc(manout[sectnum]);
	if ( ++(*number) >= MAXSECT) {
	  sprintf(error_buf,"Number of sections exceeded %d recompile %s",
		  MAXSECT,"with larger MAXSECT.");
	    PrintError(error_buf);
	}
      } 
      manout[sectnum][0] = '\0'; /* remove it's name for the prev place */
    }
    else { /* (sectnum = GetSectNumber(character)) >= 0) */
      sprintf(error_buf, "Unknown man directory 'man%s'.", character);
      PrintWarning(error_buf);
    }
  }
  fclose(descfile);
}

/*	Function Name: AddStruct
 *	Description: add the new entries to the manual structure.
 *	Arguments: manual - the manual structure to add them to.
 *                 path   - the man path for these entires.
 *                 man  - a pointer to the man directory names. 
 *                 current - a pointer to the number of the current entry.
 *	Returns: current - new current pointer numbers and changes manual
 */

void
AddStruct(manual,path,man,current,number)
Manual * manual;
char * path;
char * man[MAXSECT];
int current[MAXSECT];
int number;
{
  int i;
  DIR * dir;
  register struct direct *dp;
  char section[BUFSIZ];

  /*
   * Go through each manual directory saving up the individual
   * page entries.
   * Demand that a page entry not start with a '.' but have
   * a dot somewhere.
   */

  for(i=0; i < number; i++,man++) {
    register int j, k;
    int longest, lpixels;

    longest = lpixels = 1;	/* prevents division by zero. */
    k = current[i];

    path = StrAlloc(path);	/* allocate a space to save the path. */

    /* 
     * Use the man directories to get the file names 'cause they should be 
     * more complete. 
     */ 

    if (*man == NULL)
      continue;
    sprintf(section,"%s/%s",path, *man);
    if ( (dir = opendir(section)) == NULL) {
      sprintf(error_buf, "Could not open directory %s.", section);
      PrintWarning(error_buf);
      continue;
    }

    while((dp = readdir(dir)) != NULL) {
      if( k >= MAXENTRY) {
	sprintf(error_buf,"Too many manual pages in %s %s", section,
		"Try recompiling with larger allocations");
	PrintError(error_buf);
      }
      /* starts with a dot? no dot in name? */
      if( (dp->d_name[0] == '.') || (rindex(dp->d_name,'.') == NULL) )
	continue;
      manual[i].entries[k].label = StrAlloc(dp->d_name);
      manual[i].entries[k].path = path;
      k++;
/* 
 * This sets the value of the longest string in characters and pixels, I have
 * to play a few games since the longest string in characters is not 
 * nescessarily the longest string in pixels, and XTextWidth is not very fast.
 */
      if ((j = strlen(dp->d_name)) >= longest - 2) {
	longest = j;
	if  ( (j = XTextWidth(fonts.directory,dp->d_name,strlen(dp->d_name))) >
	     lpixels)
	lpixels = j;
      }
    }
    closedir(dir);
/*
 * Yes, this check is needed, because we may add to the structure more 
 * than once, depending on the MANPATH.
 */
    if ( (lpixels + 5) > manual[i].longest)
      manual[i].longest = lpixels+5;
    current[i] = manual[i].nentries = k;
  }
}

/*	Function Name: SortAndRemove
 *	Description: This function sorts all the entry names and
 *                   then removes all the duplicate entries.
 *	Arguments: manual - a pointer to the manual structure.
 *                 number - the number of manual sections.
 *	Returns: an improved manual stucure
 */

void
SortAndRemove(manual, number)
Manual *manual;
int number;
{
  int i;

  for ( i = 0; i < number; i++) { /* sort each section */
    register int j = 0;
    Manual * man = &(manual[i]);

    qsort(man->entries, man->nentries, sizeof(struct entry), CmpEntryLabel);

    while (j < (man->nentries - 1) ) {
      if (!strcmp(man->entries[j].label, man->entries[j+1].label)) {
	register int k = 0;
	for( k = j; k < (man->nentries -1); k++)
	  man->entries[j] = man->entries[j+1];
	(man->nentries)--;
      }
      j++;
    }
  }
}

/*	Function Name: StrAlloc
 *	Description: this function allocates memory for a character string
 *      pointed to by sp and returns its new pointer.
 *	Arguments: sp - a pointer to the string that needs memory.
 *	Returns: a pointer to this string, that is now safely allocated.
 */

char *
StrAlloc(sp) char *sp;
{
  char *ret;

  if((ret = (char *) malloc(strlen(sp)+1)) == NULL) {
    sprintf(error_buf,"Out of memory");
    PrintError(error_buf);
  }
  strcpy(ret,sp);
  return(ret);
}

/*	Function Name:  SetManNames
 *	Description: This function give a name to manual.sect
 *	Arguments: manual - pointer to the manual structure.
 *	Returns: NONE.
 */

void
SetManNames(manual)
Manual *manual;
{
  int i;
  char string[40];

  for (i = 0; i < 9; i++) {
    sprintf(string,"%s%d",MAN,i);
    manual[i].sect = StrAlloc(string);
  }
  sprintf(string,"%s%c",MAN,'l');
  manual[9].sect = StrAlloc(string);
  sprintf(string,"%s%c",MAN,'n');
  manual[10].sect = StrAlloc(string);
}

/*	Function Name: GetSectNumber
 *	Description: this gets a section number of a fixed section.
 *	Arguments: character - the character representing the section.
 *	Returns: the number of that section.
 */

static int
GetSectNumber(c)
char c;
{

  switch (c) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
    return(c - '0');
  case 'l':
    return(9);
  case 'n':
    return(10);
  default:
    break;			/* ignore others, and return -1. */
  }
  return(-1);
}

/*	Function Name: SetSectNames
 *	Description: Sets the section names.
 *	Arguments: manual - the manual structure.
 *	Returns: none.
 */

static void
SetSectionNames(manual)
Manual * manual;
{

  /* These are the names for the first 10 sections. */

  static char * names[] = {
    "foo bar (0)",
    "User Commands (1)",
    "System Calls (2)",
    "Subroutines(3)",
    "Devices (4)",
    "File Formats (5)",
    "Games (6)",
    "Miscellaneous (7)",
    "Sys. Administration (8)",
    "Local (l)",
    "New (n)"
    };
  register int i;

  for (i = 1; i < FIXEDSECT; i++)
    manual[i].blabel = names[i];
}

#if defined(DEBUG)

/*	Function Name: DumpManual
 *	Description: Debugging function that dumps the entire manual page
 *                   structure.
 *	Arguments: number - the number of sections.
 *	Returns: none.
 */

DumpManual(number)
{
  register int i,j;
  
  for ( i = 0; i < number; i++) {
    for (j = 0; j < manual[i].nentries; j++) 
      printf("path %-10s label %-20s\n", manual[i].entries[j].path,
	     manual[i].entries[j].label);
  }
}

#endif DEBUG
