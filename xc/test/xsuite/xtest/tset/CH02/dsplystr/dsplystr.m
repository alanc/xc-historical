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
>>SET   macro
>>TITLE XDisplayString CH02
char *
XDisplayString(display);
Display	*display = Dsp;
>>MAKE
>>#
>>#
>># Plant some rules in the Makefile to construct
>># stand-alone executable Test1 to allow the setting
>># of environment variables.
>>#
>># Cal 22/6/91
>>#
#
# The following lines are copied from the .m file by mc
# under control of the >>MAKE directive
# to create rules for the executable file Test1.
#
AUXFILES=Test1
AUXCLEAN=Test1.o Test1

all: Test

Test1 : Test1.o $(LIBS) $(TCMCHILD)
	$(CC) $(LDFLAGS) -o $@ Test1.o $(TCMCHILD) $(LIBLOCAL) $(LIBS) $(SYSLIBS)

#
# End of section copied from the .m file.
#
>>ASSERTION Good A
A call to xname returns the string that was used as the argument to the
.S XOpenDisplay
call that returned the value used as the
.A display
argument.
>>STRATEGY
Open a connection using XOpenDisplay.
Obtain the display string using xname.
Verify that the value of the string is the parameter used in XOpenDisplay.
Close the display using XCloseDisplay.
>>CODE
char	*dispstr;
char	*rdispstr;

	rdispstr = XCALL;

	if(rdispstr == (char *) NULL) {
		report("%s() returned NULL.", TestName);
		FAIL;
	} else {
		CHECK;
		if(strcmp(rdispstr, config.display) != 0) {
			report("%s() returned \"%s\" instead of \"%s\".", TestName, rdispstr, config.display);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2);

>>ASSERTION Good C
If the system is POSIX compliant:
When the argument passed to the
.S XOpenDisplay
call that returned the value used as the
.A display
argument was NULL, then a call to xname returns the value of
the DISPLAY environment variable when
.S XOpenDisplay
was called.
>>STRATEGY
Fork a child process using tet_fork.
In child :
  Exec the file "./Test1" with the environment variable DISPLAY set to the value of XT_DISPLAY config variable.
  Open the display "" using XOpenDisplay.
  Obtain the value of the display string using xname.
  Obtain the value of the DISPLAY environment variable.
  Verify that the two strings are identical.
>>CODE

	if(config.posix_system == 0) {
		unsupported("This assertion can only be tested on a POSIX system.");
	} else
		(void) tet_fork(t002exec, TET_NULLFP, 0, 0xFF);

>>EXTERN
extern char **environ;

static void
t002exec()
{
char	*argv[2];
char	*str;
char	*dstr;
char	*mstr = "DISPLAY=%s";
int	pass = 0, fail = 0;

	if((dstr = tet_getvar("XT_DISPLAY")) == (char *) NULL) {
		delete("XT_DISPLAY configuration variable is not defined.");
		return;
	}		

	if((str = (char *) malloc( strlen(dstr) + strlen(mstr) + 1)) == (char *) NULL) {
		delete("malloc() failed.");
		return;
	}

	sprintf(str, mstr, dstr);

	argv[0] = "./Test1";
	argv[1] = (char *) NULL;

	if (xtest_putenv( str ) ) {
		delete("xtest_putenv failed");
		return;
	}

	(void) tet_exec("./Test1", argv, environ);

	delete("Exec of file ./Test1 failed");
	free( (char *) str);
}
