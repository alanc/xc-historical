#ifndef lint
static char rcs_id[] = "$XConsortium: command.c,v 2.21 89/06/01 15:10:49 kit Exp $";
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
    int		output_pipe[2];
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
   long commands.  Returns 0 if stderr empty, -1 otherwise. */

static int _DoCommandToFileOrPipe(argv, inputfd, outputfd, bufP, lenP)
  char **argv;			/* The command to execute, and its args. */
  int inputfd;			/* Input stream for command. */
  int outputfd;			/* Output stream; /dev/null if == -1 */
  char **bufP;			/* output buffer ptr if outputfd == -2 */
  int *lenP;			/* output length ptr if outputfd == -2 */
{
    int return_status;
    int old_stdin, old_stdout, old_stderr;
    int pid;
    fd_set readfds, fds;
    Boolean output_to_pipe = False;
    CommandStatus status = XtNew(CommandStatusRec);
    FD_ZERO(&fds);
    FD_SET(ConnectionNumber(theDisplay), &fds);
    DEBUG1("Executing %s ...", argv[0])

    if (inputfd != -1) {
	old_stdin = dup(fileno(stdin));
	(void) dup2(inputfd, fileno(stdin));
    }

    if (outputfd == -1) {
	if (!app_resources.debug) { /* Throw away stdout. */
	    outputfd = open( "/dev/null", O_WRONLY, 0 );
	    old_stdout = dup(fileno(stdout));
	    (void) dup2(outputfd, fileno(stdout));
	    close(outputfd);
	}
    }
    else if (outputfd == -2) {	/* make pipe */
	if (pipe(status->output_pipe) /*failed*/) {
	    SystemError( "couldn't re-direct standard output" );
	    status->output_pipe[0]=0;
	}
	else {
	    outputfd = status->output_pipe[1];
	    old_stdout = dup(fileno(stdout));
	    (void) dup2(status->output_pipe[1], fileno(stdout));
	    FD_SET(status->output_pipe[0], &fds);
#ifdef notdef
	    status->output_inputId =
		XtAddInput( output_pipe[0], (caddr_t)XtInputReadMask,
			    ReadStdout, (caddr_t)status
			   );
#endif /*notdef*/
	    output_to_pipe = True;
	}
    }
    else {
	old_stdout = dup(fileno(stdout));
	(void) dup2(outputfd, fileno(stdout));
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
    }

    childdone = FALSE;
    status->popup = (Widget)NULL;
    status->lastInput = lastInput;
    status->error_output = NULL;
    status->buf_size = 0;
    (void) signal(SIGCHLD, ChildDone);
    pid = vfork();
    if (inputfd != -1) {
	if (pid != 0) dup2(old_stdin,  fileno(stdin));
	close(old_stdin);
    }
    if (outputfd != -1) {
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
	DEBUG1( " pid=%d", pid )
	while (!childdone) {
	    XEvent event;
	    while (XCheckTypedEvent( theDisplay, Expose, &event )) {
		XtDispatchEvent( &event );
	    }
	    readfds = fds;
	    (void) select(num_fds, (int *) &readfds,
			  (int *) NULL, (int *) NULL, (struct timeval *) NULL);
	    if (output_to_pipe && FD_ISSET(status->output_pipe[0], &readfds)) {
		CheckReadFromPipe( status->output_pipe[0], bufP, lenP );
	    }
	    if (FD_ISSET(status->error_pipe[0], &readfds)) {
		CheckReadFromPipe( status->error_pipe[0],
				   &status->error_output,
				   &status->buf_size
				  );
	    }
	}
	if (output_to_pipe && FD_ISSET(status->output_pipe[0], &readfds)) {
	    CheckReadFromPipe( status->output_pipe[0], bufP, lenP );
	}
	CheckReadFromPipe( status->error_pipe[0],
			   &status->error_output,
			   &status->buf_size
			  );
#ifdef SYSV
	(void) wait((int *) NULL);
#else /* !SYSV */
	(void) wait((union wait *) NULL);
#endif /* !SYSV */

	DEBUG(" done\n")
	if (output_to_pipe) {
	    close( status->output_pipe[0] );
	    close( status->output_pipe[1] );
	}
	if (status->error_pipe[0]) {
	    close( status->error_pipe[0] );
	    close( status->error_pipe[1] );
	}
	if (status->error_output != NULL) {
	    while (status->error_output[status->buf_size-1]  == '\0')
		status->buf_size--;
	    while (status->error_output[status->buf_size-1]  == '\n')
		status->error_output[--status->buf_size] = '\0';
	    DEBUG1( "stderr = \"%s\"\n", status->error_output );
	    PopupNotice( status->error_output, FreeStatus, (Pointer)status );
	    return_status = -1;
	}
	else {
	    FreeStatus( (Widget)NULL, (Pointer)status, (Pointer)NULL );
	    return_status = 0;
	}
    } else {			/* We're the child process. */
	(void) execv(FullPathOfCommand(argv[0]), argv);
	(void) execvp(argv[0], argv);
	Punt("Execvp failed!");
	return_status = -1;
    }
    return return_status;
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
    if (status->error_output != NULL) XtFree(status->error_output);
    XtFree( closure );
}

/* Execute the given command, waiting until it's finished.  Put the output
   in the specified file path.  Returns 0 if stderr empty, -1 otherwise */

DoCommand(argv, inputfile, outputfile)
  char **argv;			/* The command to execute, and its args. */
  char *inputfile;		/* Input file for command. */
  char *outputfile;		/* Output file for command. */
{
    int fd_in, fd_out;
    int status;

    if (inputfile != NULL) {
	FILEPTR file = FOpenAndCheck(inputfile, "r");
	fd_in = dup(fileno(file));
	myfclose(file);
    }
    else
	fd_in = -1;

    if (outputfile) {
	FILEPTR file = FOpenAndCheck(outputfile, "w");
	fd_out = dup(fileno(file));
	myfclose(file);
    }
    else
	fd_out = -1;

    status = _DoCommandToFileOrPipe( argv, fd_in, fd_out, NULL, NULL );
    if (fd_in != -1) close(fd_in);
    if (fd_out != -1) close(fd_out);
    return status;
}

/* Execute the given command, waiting until it's finished.  Put the output
   in a newly mallocced string, and return a pointer to that string. */

char *DoCommandToString(argv)
char ** argv;
{
    char *result = NULL;
    int len = 0;
    _DoCommandToFileOrPipe( argv, -1, -2, &result, &len );
    if (result == NULL) result = XtMalloc(1);
    result[len] = '\0';
    DEBUG1("('%s')\n", result)
    return result;
}
    

/* Execute the command to a temporary file, and return the name of the file. */

char *DoCommandToFile(argv)
  char **argv;
{
    char *name;
    FILEPTR file;
    int fd;
    name = MakeNewTempFileName();
    file = FOpenAndCheck(name, "w");
    fd = dup(fileno(file));
    myfclose(file);
    _DoCommandToFileOrPipe(argv, -1, fd, NULL, NULL);
    close(fd);
    return name;
}
