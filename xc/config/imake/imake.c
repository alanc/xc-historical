/*
 * $Header: imake.c,v 1.7 86/10/14 14:37:49 toddb Exp $
 * $Locker: toddb $
 *
 * imake: the include-make program.
 *
 * Usage: imake [-v] [-f imakefile ] [ make flags ]
 *
 * Imake takes a template makefile and runs cpp on it producing a temporary
 * makefile in /usr/tmp.  It then runs make on this pre-processed makefile.
 * Options:
 *		-v	verbose.  Show the make command line executed.
 *		-s	show.  Show the produced makefile on the standard
 *			output.  Make is not run is this case.
 *
 * Environment variables:
 *		
 *		IMAKEINCLUDE	Include directory to use in addition to
 *				"." and "/usr/lib/local/imake.include".
 *		IMAKECPP	Cpp to use instead of /lib/cpp
 *		IMAKEMAKE	make program to use other than what is
 *				found by searching the $PATH variable.
 * Other features:
 *	imake reads the entire cpp output into memory and then scans if
 *	for occurences of "@@".  If it encounters them, it replaces it with
 *	a newline.  It also trims any trailing white space on output lines
 *	(because make gets upset at them).  This helps when cpp expands
 *	multi-line macros but you want them to appear on multiple lines.
 *
 *	The macros MAKEFILE and MAKE are provided as commandline options
 *	to make.  MAKEFILE is set to imake's makefile (not the constructed,
 *	preprocessed one) and MAKE is set to argv[0], i.e. the name of
 *	the imake program.
 */
#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	<sys/wait.h>
#include	<sys/signal.h>
#include	<sys/stat.h>

#define	TRUE		1
#define	FALSE		0
#define	ARGUMENTS	50

typedef	u_char	boolean;

char	*tmpfile    = "/usr/tmp/imake.XXXXXX";
char	*cpp = "/lib/cpp";
char	*make_argv[ ARGUMENTS ] = { "make" };
char	*cpp_argv[ ARGUMENTS ] = {
	"cpp",
	"-I.",
	"-I/usr/lib/local/imake.includes",
	"-Uvax",
	"-Uunix",
};
int	make_argindex = 1;
int	cpp_argindex = 5;
char	*make = NULL;
char	*makefile = NULL;
char	*program;
char	*findmakefile();
char	*readline();
boolean	verbose = FALSE;
boolean	show = FALSE;
extern int	errno;
extern char	*malloc();
extern char	*realloc();
extern char	*getenv();
extern char	*mktemp();

main(argc, argv)
	int	argc;
	char	**argv;
{
	FILE	*tmpfd;

	init();
	setopts(argc, argv);

	makefile = findmakefile(makefile);
	tmpfile = mktemp(tmpfile);
	add_make_arg("-f");
	add_make_arg( tmpfile );

	if ((tmpfd = fopen(tmpfile, "w+")) == NULL)
		log_fatal("Cannot create temporary file %s.", tmpfile);

	cppit(makefile, tmpfd);

	if (show)
		showit(tmpfd);
	else
		makeit();
	wrapup();
}

showit(fd)
	FILE	*fd;
{
	char	buf[ BUFSIZ ];
	int	red;

	fseek(fd, 0, 0);
	while ((red = fread(buf, 1, BUFSIZ, fd)) > 0)
		fwrite(buf, red, 1, stdout);
	if (red < 0)
		log_fatal("Cannot write stdout.");
}

wrapup()
{
	unlink(tmpfile);
}

catch(sig)
	int	sig;
{
	errno = 0;
	log_fatal("Signal %d.", sig);
}

/*
 * Initialize some variables.
 */
init()
{
	char	*p;

	/*
	 * See if the standard include directory is different than
	 * the default.  Or if cpp is not the default.  Or if the make
	 * found by the PATH variable is not the default.
	 */
	if (p = getenv("IMAKEINCLUDE")) {
		add_cpp_arg(p);
		for (; *p; p++)
			if (*p == ' ') {
				*p++ = '\0';
				add_cpp_arg(p);
			}
	}
	if (p = getenv("IMAKECPP"))
		cpp = p;
	if (p = getenv("IMAKEMAKE"))
		make = p;

	signal(SIGINT, catch);
}

add_make_arg(arg)
	char	*arg;
{
	errno = 0;
	if (make_argindex >= ARGUMENTS-1)
		log_fatal("Out of internal storage.");
	make_argv[ make_argindex++ ] = arg;
	make_argv[ make_argindex ] = NULL;
}

add_cpp_arg(arg)
	char	*arg;
{
	errno = 0;
	if (cpp_argindex >= ARGUMENTS-1)
		log_fatal("Out of internal storage.");
	cpp_argv[ cpp_argindex++ ] = arg;
	cpp_argv[ cpp_argindex ] = NULL;
}

setopts(argc, argv)
	int	argc;
	char	**argv;
{
	errno = 0;
	/*
	 * Now gather the arguments for make
	 */
	program = argv[0];
	for(argc--, argv++; argc; argc--, argv++) {
		/*
		 * We intercept these flags.
		 */
		if (argv[0][0] == '-' && argv[0][1] == 'f') {
			argc--, argv++;
			if (! argc)
				log_fatal("No description argument after %s",
					"-f flag.");
			makefile = argv[0];
			continue;
		}
		if (argv[0][0] == '-' && argv[0][1] == 'v') {
			verbose = TRUE;
			continue;
		}
		if (argv[0][0] == '-' && argv[0][1] == 's') {
			show = TRUE;
			continue;
		}
		add_make_arg(argv[0]);
	}
}

char *findmakefile(makefile)
	char	*makefile;
{
	int	fd;

	if (makefile) {
		if ((fd = open(makefile, O_RDONLY)) < 0)
			log_fatal("Cannot open %s.", makefile);
	} else {
		if ((fd = open("Imakefile", O_RDONLY)) < 0)
			if ((fd = open("imakefile", O_RDONLY)) < 0)
				log_fatal("No description file.");
			else
				makefile = "imakefile";
		else
			makefile = "Imakefile";
	}
	close (fd);
	return(makefile);
}

log_fatal(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
{
	extern char	*sys_errlist[];
	static boolean	entered = FALSE;

	if (entered)
		return;
	entered = TRUE;

	fprintf(stderr, "%s: ", program);
	if (errno)
		fprintf(stderr, "%s: ", sys_errlist[ errno ]);
	fprintf(stderr, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	fprintf(stderr, "  Stop.\n");
	wrapup();
	exit(1);
}

showargs(argv)
	char	**argv;
{
	for (; *argv; argv++)
		fprintf(stderr, "%s ", *argv);
	fprintf(stderr, "\n");
}

cppit(makefile, outfd)
	char	*makefile;
	FILE	*outfd;
{
	int	pid, pipefd[2];
	union wait	status;

	/*
	 * Get a pipe.
	 */
	if (pipe(pipefd) < 0)
		log_fatal("Cannot make a pipe.");

	/*
	 * Fork and exec cpp
	 */
	pid = vfork();
	if (pid < 0)
		log_fatal("Cannot fork.");
	if (pid) {	/* parent */
		close(pipefd[0]);
		clean_cpp_input(makefile, pipefd[1]);
		while (wait(&status) > 0) {
			errno = 0;
			if (status.w_termsig)
				log_fatal("Signal %d.", status.w_termsig);
			if (status.w_retcode)
				log_fatal("Exit code %d.", status.w_retcode);
		}
		clean_cpp_output(outfd);
	} else {	/* child... dup and exec cpp */
		if (verbose)
			showargs(cpp_argv);
		dup2(pipefd[0], 0);
		dup2(fileno(outfd), 1);
		close(pipefd[1]);
		execv(cpp, cpp_argv);
		log_fatal("Cannot exec %s.", cpp);
	}
}

makeit()
{
	int	pid;
	union wait	status;

	/*
	 * Fork and exec make
	 */
	pid = vfork();
	if (pid < 0)
		log_fatal("Cannot fork.");
	if (pid) {	/* parent... simply wait */
		while (wait(&status) > 0) {
			errno = 0;
			if (status.w_termsig)
				log_fatal("Signal %d.", status.w_termsig);
			if (status.w_retcode)
				log_fatal("Exit code %d.", status.w_retcode);
		}
	} else {	/* child... dup and exec cpp */
		if (verbose)
			showargs(make_argv);
		if (make)
			execv(make, make_argv);
		else
			execvp("make", make_argv);
		log_fatal("Cannot exec %s.", cpp);
	}
}

clean_cpp_input(makefile, outfd)
	char	*makefile;
	FILE	*outfd;
{
	FILE	*in, *out;
	char	buf[ BUFSIZ ], *ptoken, *pend, savec;

	if ((out = fdopen(outfd, "w")) == NULL)
		log_fatal("Cannot fdopen fd %d for output.", outfd);
	if ((in = fopen(makefile, "r")) == NULL)
		log_fatal("Cannot open %s for input.", makefile);

	fprintf(out, "IMAKE=%s\n", program);
	fprintf(out, "IMAKEFILE=%s\n", makefile);

	while (fgets(buf, BUFSIZ, in)) {
		if (*buf == '#') {	/* pad make comments for cpp */
			ptoken = buf+1;
			while (*ptoken == ' ' || *ptoken == '\t')
				ptoken++;
			pend = ptoken;
			while (*pend && *pend != ' ' && *pend != '\t')
				pend++;
			savec = *pend;
			*pend = '\0';
			if (strcmp(ptoken, "include")
			 && strcmp(ptoken, "define")
			 && strcmp(ptoken, "undef")
			 && strcmp(ptoken, "ifdef")
			 && strcmp(ptoken, "else")
			 && strcmp(ptoken, "endif")
			 && strcmp(ptoken, "if"))
				fputs("/**/", out);
			*pend = savec;
		}
		fputs(buf, out);
	}

	/*
	 * Insist on a trailing include file: "depend".
	 */
	fprintf(out, "#include\t<depend>\n", makefile);

	fclose(in);
	fclose(out); /* also closes the pipe */
}

clean_cpp_output(tmpfd)
	FILE	*tmpfd;
{
	char	*input;
	int	blankline = 0;

	while(input = readline(tmpfd)) {
		if (isempty(input)) {
			if (blankline++)
				continue;
		} else {
			blankline = 0;
			fputs(input, tmpfd);
		}
		putc('\n', tmpfd);
	}
	fflush(tmpfd);
}

/*
 * Determine of a line has nothing in it.  As a side effect, we trim white
 * space from the end of the line.  Cpp magic cookies are also thrown away.
 */
isempty(line)
	char	*line;
{
	char	*pend;

	/*
	 * Check for lines of the form
	 *	# n "...
	 * or
	 *	# line n "...
	 */
	if (*line == '#') {
		pend = line+1;
		if (*pend == ' ')
			pend++;
		if (strncmp(pend, "line ", 5) == 0)
			pend += 5;
		if (isdigit(*pend)) {
			while (isdigit(*pend))
				pend++;
			if (*pend++ == ' ' && *pend == '"')
				return(TRUE);
		}
	}

	/*
	 * Find the end of the line and then walk back.
	 */
	for (pend=line; *pend; pend++) ;

	pend--;
	while (pend >= line && (*pend == ' ' || *pend == '\t'))
		pend--;
	*++pend = '\0';
	return (*line == '\0');
}

char *readline(tmpfd)
	FILE	*tmpfd;
{
	static boolean	initialized = FALSE;
	static char	*buf, *pline, *end;
	char	*p1, *p2;

	if (! initialized) {
		int	total_red;
		struct stat	st;

		/*
		 * Slurp it all up.
		 */
		fseek(tmpfd, 0, 0);
		fstat(fileno(tmpfd), &st);
		pline = buf = malloc(st.st_size+1);
		total_red = read(fileno(tmpfd), buf, st.st_size);
		if (total_red != st.st_size)
			log_fatal("cannot read %s\n", tmpfile);
		end = buf + st.st_size;
		*end = '\0';
		lseek(fileno(tmpfd), 0, 0);
		ftruncate(fileno(tmpfd), 0);
		initialized = TRUE;
	}

	for (p1 = pline; p1 < end; p1++) {
		if (*p1 == '@' && *(p1+1) == '@') { /* soft EOL */
			*p1++ = '\0';
			p1++; /* skip over second @ */
			break;
		}
		else if (*p1 == '\n') { /* real EOL */
			*p1++ = '\0';
			break;
		}
	}

	/*
	 * return NULL at the end of the file.
	 */
	p2 = (pline == p1 ? NULL : pline);
	pline = p1;
	return(p2);
}

writetmpfile(fd, buf, cnt)
	FILE	*fd;
	int	cnt;
	char	*buf;
{
	errno = 0;
	if (fwrite(buf, cnt, 1, fd) != 1)
		log_fatal("Cannot write to %s.", tmpfile);
}
