#ifndef lint
static char rcs_id[] = "$XConsortium: command.c,v 2.18 89/02/02 13:04:35 swick Exp $";
#endif lint
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

/* command.c -- interface to exec mh commands. */

#include "xmh.h"
#include <sys/ioctl.h>
#include <sys/signal.h>
#ifndef SYSV
#include <sys/wait.h>
#endif	/* SYSV */
#include <sys/resource.h>

#ifdef macII
#define vfork() fork()
#endif /* macII */

#if defined(SYSV) && !defined(hpux)
#define vfork() fork()
#endif /* SYSV and not hpux */


#ifndef FD_SET
#define NFDBITS         (8*sizeof(fd_set))
#define FD_SETSIZE      NFDBITS
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))
#endif FD_SET


typedef struct _CommandStatus {
    Widget	popup;		 /* must be first; see PopupStatus */
    struct _LastInput lastInput; /* must be second; ditto */
    int		child_pid;
    XtInputId	error_inputId;
    int		error_pipe[2];
    char*	error_output;
    int		buf_size;
} CommandStatusRec, *CommandStatus;

typedef char* Pointer;
static void FreeStatus();

static void SystemError(text)
    char* text;
{
    extern int sys_nerr;
    extern char* sys_errlist[];
    char msg[BUFSIZ];
    sprintf( msg, "%s; errno = %d %s", text, errno, 
	     (errno < sys_nerr) ? sys_errlist[errno] : NULL );
    XtWarning( msg );
}


/* Return the full path name of the given mh command. */

static char *FullPathOfCommand(str)
  char *str;
{
    static char result[100];
    (void) sprintf(result, "%s/%s", app_resources.defMhPath, str);
    return result;
}


static int childdone;		/* Gets nonzero when the child process
				   finishes. */
ChildDone()
{
    childdone++;
}

/* Execute the given command, and wait until it has finished.  While the
   command is executing, watch the X socket and cause Xlib to read in any
   incoming data.  This will prevent the socket from overflowing during
   long commands. */

DoCommand(argv, inputfile, outputfile)
  char **argv;			/* The command to execute, and its args. */
  char *inputfile;		/* Input file for command. */
  char *outputfile;		/* Output file for command. */
{
    int old_stdin, old_stdout, old_stderr;
    FILEPTR fin, fout, fnull = NULL;
    int pid;
    fd_set readfds, fds;
    CommandStatus status = XtNew(CommandStatusRec);
    FD_ZERO(&fds);
    FD_SET(ConnectionNumber(theDisplay), &fds);
    DEBUG1("Executing %s ...", argv[0])

    if (inputfile) {
        old_stdin = dup(fileno(stdin));
	fin = FOpenAndCheck(inputfile, "r");
	(void) dup2(fileno(fin), fileno(stdin));
	myfclose(fin);
    }

    old_stdout = dup(fileno(stdout));
    if (outputfile) {
	fout = FOpenAndCheck(outputfile, "w");
	(void) dup2(fileno(fout), fileno(stdout));
	myfclose(fout);
    }
    else if (!app_resources.debug) { /* Throw away stdout. */
	fnull = FOpenAndCheck("/dev/null", "w");
	(void) dup2(fileno(fnull), fileno(stdout));
	myfclose(fnull);
    }

    if (pipe(status->error_pipe) /*failed*/) {
	SystemError( "couldn't re-direct standard error" );
	status->error_pipe[0]=0;
    }
    else {
	old_stderr = dup(fileno(stderr));
	(void) dup2(status->error_pipe[1], fileno(stderr));
	FD_SET(status->error_pipe[0], &fds);
#ifdef notdef
	status->error_inputId =
	    XtAddInput( err_pipe[0], (caddr_t)XtInputReadMask,
		        ReadStderr, (caddr_t)status
		       );
#endif /*notdef*/
	fcntl(status->error_pipe[0], F_SETFL, FNDELAY);
    }

    childdone = FALSE;
    status->popup = (Widget)NULL;
    status->lastInput = lastInput;
    status->error_output = NULL;
    status->buf_size = 0;
    (void) signal(SIGCHLD, ChildDone);
    pid = vfork();
    if (inputfile) {
	if (pid != 0) dup2(old_stdin,  fileno(stdin));
	close(old_stdin);
    }
    if (outputfile || !app_resources.debug) {
	if (pid != 0) dup2(old_stdout, fileno(stdout));
	close(old_stdout);
    }
    if (status->error_pipe[0]) {
	if (pid != 0) dup2(old_stderr, fileno(stderr));
	close(old_stderr);
    }

    if (pid == -1) Punt("Couldn't fork!");
    if (pid) {			/* We're the parent process. */
	int num_fds = (ConnectionNumber(theDisplay) < status->error_pipe[0])
	    ? status->error_pipe[0]+1 : ConnectionNumber(theDisplay)+1;
	status->child_pid = pid;
	while (!childdone) {
	    XEvent event;
	    while (XCheckTypedEvent( theDisplay, Expose, &event )) {
		XtDispatchEvent( &event );
	    }
	    readfds = fds;
	    (void) select(num_fds, (int *) &readfds,
			  (int *) NULL, (int *) NULL, (struct timeval *) NULL);
	    if (FD_ISSET(status->error_pipe[0], &readfds)) {
		CheckReadFromPipe( status->error_pipe[0],
				   &status->error_output,
				   &status->buf_size
				  );
	    }
	}
	CheckReadFromPipe( status->error_pipe[0],
			   &status->error_output,
			   &status->buf_size
			  );
#ifdef notdef
#ifdef SYSV
	(void) wait((int *) NULL);
#else /* !SYSV */
	(void) wait((union wait *) NULL);
#endif /* !SYSV */
#endif /*notdef*/

	DEBUG(" done\n")
	if (status->error_output != NULL) {
	    while (status->error_output[status->buf_size-1]  == '\0')
		status->buf_size--;
	    while (status->error_output[status->buf_size-1]  == '\n')
		status->error_output[--status->buf_size] = '\0';
	    DEBUG1( "stderr = \"%s\"\n", status->error_output );
	    PopupNotice( status->error_output, FreeStatus, (Pointer)status );
	}
	else {
	    FreeStatus( (Widget)NULL, (Pointer)status, (Pointer)NULL );
	}
    } else {			/* We're the child process. */
	(void) execv(FullPathOfCommand(argv[0]), argv);
	(void) execvp(argv[0], argv);
	Punt("Execvp failed!");
    }
}


static /*void*/
CheckReadFromPipe( fd, bufP, lenP )
    int fd;
    char **bufP;
    int *lenP;
{
    long nread;
    if (ioctl( fd, FIONREAD, &nread ) /*failed*/) {
	SystemError( "couldn't inquire bytes in pipe" );
    }
    else if (nread) {
	char buf[BUFSIZ];
	int old_end = *lenP;
	*bufP = XtRealloc( *bufP, (*lenP += nread) + 1 );
	while (nread > BUFSIZ) {
	    read( fd, buf, BUFSIZ );
	    bcopy( buf, *bufP+old_end, BUFSIZ );
	    nread -= BUFSIZ;
	    old_end += BUFSIZ;
	}
	read( fd, buf, nread );
	bcopy( buf, *bufP+old_end, nread );
	(*bufP)[old_end+nread] = '\0';
    }
}


/* ARGSUSED */
static void FreeStatus( w, closure, call_data )
    Widget w;			/* unused */
    Pointer closure;
    Pointer call_data;		/* unused */
{
    CommandStatus status = (CommandStatus)closure;
    if (status->popup != (Widget)NULL) {
	XtPopdown( status->popup );
	XtDestroyWidget( status->popup );
    }
#ifdef notdef
    XtRemoveInput( status->error_inputId );
#endif /*notdef*/
    if (status->error_pipe[0]) {
	close( status->error_pipe[0] );
	close( status->error_pipe[1] );
    }
    if (status->error_output != NULL) XtFree(status->error_output);
    XtFree( closure );
}



/* Execute the given command, waiting until it's finished.  Put the output
   in a newly mallocced string, and return a pointer to that string. */

char *DoCommandToString(argv)
char ** argv;
{
    char *result;
    char *file;
    int fid, length;
    file = DoCommandToFile(argv);
    length = GetFileLength(file);
    result = XtMalloc((unsigned) length + 1);
    fid = myopen(file, O_RDONLY, 0666);
    if (length != read(fid, result, length))
	Punt("Couldn't read result from DoCommandToString");
    result[length] = 0;
    DEBUG1("('%s')\n", result)
    (void) myclose(fid);
    DeleteFileAndCheck(file);
    return result;
}
    

#ifdef NOTDEF	/* This implementation doesn't work right on null return. */
char *DoCommandToString(argv)
  char **argv;
{
    static char result[1030];
    int fildes[2], pid, l;
    DEBUG1("Executing %s ...", argv[0])
    (void) pipe(fildes);
    pid = vfork();
    if (pid == -1) Punt("Couldn't fork!");
    if (pid) {
#ifdef SYSV
        while (wait((int *) 0) == -1) ;
#else /* !SYSV */
	while (wait((union wait *) 0) == -1) ;
#endif /* !SYSV */
	l = read(fildes[0], result, 1024);
	if (l <= 0) Punt("Couldn't read result from DoCommandToString");
	(void) myclose(fildes[0]);
	result[l] = 0;
	while (result[--l] == 0) ;
	while (result[l] == '\n') result[l--] = 0;
	DEBUG1(" done: '%s'\n", result)
	return result;
    } else {
	(void) dup2(fildes[1], fileno(stdout));
	(void) execv(FullPathOfCommand(argv[0]), argv);
	(void) execvp(argv[0], argv);
	Punt("Execvp failed!");
	return NULL;
    }
}
#endif NOTDEF



/* Execute the command to a temporary file, and return the name of the file. */

char *DoCommandToFile(argv)
  char **argv;
{
    char *name;
    name = MakeNewTempFileName();
    DoCommand(argv, (char *) NULL, name);
    return name;
}
