#define SUFFIX "suffix"
#define FOLD "fold"
#define FOLDSUFFIX "foldsuffix"
#define MNULL 0
#define MSUFFIX 1
#define MFOLD 2
#define MFOLDSUFFIX 3

/* Vendor-specific definitions */

/*
 * The directories to search.  Assume that the manual directories are more 
 * complete than the cat directories.
 */

#if ( defined(UTEK) || defined(apollo) )
#  define SEARCHDIR  CAT
#  define LSEARCHDIR LCAT
#else
#  define SEARCHDIR  MAN
#  define LSEARCHDIR LMAN
#endif

/*
 * The default manual page directory. 
 *
 * The MANPATH enviornment variable will override this.
 */

#ifndef DEFAULTMANPATH

#ifdef macII
#  define MANDIR "/usr/catman/u_man:/usr/catman/a_man"	
#else /* macII */
#ifdef SVR4
#  define MANDIR "/usr/share/man"
#else /* SVR4 */
#ifdef hcx
#  define MANDIR "/usr/catman/local_man:/usr/catman/u_man:/usr/catman/a_man:/usr/catman/p_man:/usr/catman/ada_man"
#else
#  define MANDIR "/usr/man"
#endif /* hcx else */
#endif /* SVR4 else */
#endif /* macII else */

#else
#define MANDIR DEFAULTMANPATH
#endif

/*
 * Compression Definitions.
 */

#if defined( macII ) || defined( hcx )
#  define COMPRESSION_EXTENSION   "z"
#  define UNCOMPRESS_FORMAT       "pcat %s > %s"
#  define NO_COMPRESS		/* mac can't handle using pack as a filter and
				   xman needs it to be done that way. */
#else 
#  if defined ( UTEK )
#    define COMPRESSION_EXTENSION "C"
#    define UNCOMPRESS_FORMAT     "ccat < %s > %s"
#    define COMPRESS              "compact"
#  else
#    define COMPRESSION_EXTENSION "Z"
#    define UNCOMPRESS_FORMAT     "zcat < %s > %s"
#    define COMPRESS              "compress"
#  endif /* UTEK */
#endif /* macII */



/*
 * The command filters for the manual and apropos searches.
 */

#if ( defined(hpux) || defined(macII) || defined(CRAY) || defined(ultrix) || defined(hcx) )
#  define NO_MANPATH_SUPPORT
#endif

#ifdef NO_MANPATH_SUPPORT
#  define APROPOS_FORMAT ("man -k %s | pr -h Apropos >> %s")
#else
#  define APROPOS_FORMAT ("man -M %s -k %s | pr -h Apropos > %s")
#endif

#if defined( ultrix )
#  define FORMAT "| nroff -man"             /* The format command. */
#else
#  define FORMAT "| neqn | nroff -man"      /* The format command. */
#endif

/*
 * Names of the man and cat dirs. 
 */

#define MAN "man"

#if ( defined(macII) || defined(CRAY) || defined(hcx) )
/*
 * The Apple, Cray, and HCX folks put the preformatted pages in the "man" 
 * directories.
 */
#  define CAT MAN		
#else
#  define CAT "cat"
#endif

extern void AddStandardSections();
extern void AddNewSection();

typedef struct _SectionList {
  struct _SectionList * next;
  char * label;			/* section label */
  char * directory;		/* section directory */
  int flags;
} SectionList;
