.\" $XConsortium: relnotes.mm,v 1.2 92/06/30 19:29:25 rws Exp $
.de TP
.sp 1
.tl \\*(}t
.if e 'tl \\*(}e
.if o 'tl \\*(}o
.sp 1
..
.VM 0 3
.de )k
..
'\"
'\" Start and end of a user-typed display
'\"
.de cS
.DS I
.ft C
.ps -2
..
.de cE
.ps +2
.ft R
.DE
..
'\" # Courier
.de C
\fC\\$1\fP\\$2\fC\\$3\fP\\$4\fC\\$5\fP\\$6
..
.ds HP 16 16 14
.ds rC "UniSoft Ltd.
.ds xW "X\ Window System
.ds xT "X\ test suite
.ds cH "0"
.nr Hc 2
.nr Hs 4
.nr HF 3 3 3 3 3 
.ds HP 14 14 14
.ds dD Release Notes for the X\ test suite - release 1.1
.ds dN "\s-1MIT\s0-3-00.3
.HU " "
.PH "'\s14Xlib and X\ Protocol test suite''\*(dN'"
.EH "'\*(dD '''"
.OH "'\*(dD '''"
.PF "'May 15, 1992''Page \\\\nP'"
.EF "''\s12\*(rC''"
.OF "''\s12\*(rC''"
.S 18
.HU "Release Notes for the X\ test suite - release 1.1"
.S 12
.ds HP 14 14 14
.nr H1 0
.nr Hc 1
.SP 2
.HU "Contents"
This release note describes the contents of release 1.1
of the revised \*(xT which has been produced by UniSoft under contract to MIT. 
.P
This release tests sections 2 to 10 
of the 
\fIXlib: C\ Language X\ Interface 
(MIT\ X\ Consortium\ Standard - X\ Version\ 11, Release 4)\fR\*F. 
.FS
The \*(xW is a trademark of the Massachusetts Institute of Technology.
.br
\*(xW Version\ 11 Release\ 4 is abbreviated to X11R4 in this document.
.br
\*(xW Version\ 11 Release\ 5 is abbreviated to X11R5 in this document.
.FE
It also tests parts of the 
\fIX\ Window\ System\ Protocol 
(MIT\ X\ Consortium\ Standard - X\ Version\ 11)\fR
where these cannot be infered from tests at the Xlib level.
.P
The revised \*(xT may be used to test later versions of X11
which are compatible with the X11R4 standards. The 
test suite is known to build correctly using the X11R5 Xlib distributed by MIT.
.P
This release includes fixes to all bugs submitted on the beta release 
and gamma release up to May 1, 1992. 
The list of bugs fixed is shown in appendices C and D.
.P
Appendix A compares this release of the \*(xT with earlier releases from 
UniSoft, and is intended primarily for reviewers of earlier releases.
.HU "Distribution"
This release is available from MIT.
.P
This release is available to members
of the X\ Consortium via xftp/xuucp to expo as the file
.cS
        /docs/xtest/stage3/rel1.1.tar.Z
.cE
.P
Contact Bob Scheifler (rws@expo.lcs.mit.edu) if you cannot obtain it in 
that way or if you require more details.
.HU "Submitting bug reports"
Please submit bug reports on this release
to MIT using the email address xtest-bugs@expo.lcs.mit.edu.
Use the report template provided in the file bug-report.
.P
UniSoft will continue to provide support for the \*(xT release 1.1 for MIT 
until June 29th, 1992. Until then, 
UniSoft intends to fix genuine defects in this release as 
defined in the Project Specification dated March 1990.
Fixes to genuine defects will be made available to MIT in an update via email
on June 29th, 1992.
.HU "Target execution environments"
The software was developed on a DECstation 3100 running ULTRIX\*F
.FS
ULTRIX, DEC, and DIGITAL are registered trademarks of Digital
Equipment Corporation.
.FE
4.2a,
and a Sequent
.SM DYNIX
V3.0.14 system.
It is also known to compile and run on a HP-UX\*F
.FS
HP-UX is a registered trademark of Hewlett Packard Corporation.
.FE
version 7.0
system, and on a SPARCstation running SunOS\*F
.FS
SunOS is a trademark of SUN Microsystems, Inc.
.FE
release 4.1.1.
.P
The main portability limitations occur in the
.SM TET
which is described further below. This is because the 
.SM TET 
was originally developed to run on systems which are POSIX.1\*F
.FS
IEEE Std 1003.1-1990, \fIPortable Operating System Interface for
Computer Environments\fR
.FE
compliant.
.P
The aim is that the \*(xT will run without
code modifications on BSD4.2 systems, UNIX\*F
.FS
UNIX is a registered trademark of UNIX System Laboratories, Inc. in
the U.S. and other countries.
.FE
System V (Release 3 and 4) 
and POSIX.1 compliant systems. During development of the revised \*(xT,
it was decided that since there are so many variants of the aforementioned
systems,
the aim should be to enable the test suite to execute without modification
on platforms on which X11R4 executes without modification (ie. those
for which there exist \fCconfig\fP files).
.P
To enable the \*(xT to build easily on 
BSD4.2 systems, a portability library has been developed which contains 
POSIX.1 functions not present on vanilla BSD4.2 systems. The 
contents and use of this library are described further in the User Guide.
.P
Beyond this, variants of the aforementioned systems may require 
some porting effort dependent on the number of commonly supported functions
which are absent in a particular implementation.
.HU "Status of the Test Environment Toolkit (\s-1TET\s0)"
.AL
.LI
The \*(xT includes a copy of
.SM TET 
version 1.9 with a small number of changes described below.
.LI
The supplied version of 
.SM TET 
includes fixes to four bugs reported since the 
.SM TET 
1.9 release.
.LI
The Makefiles supplied with
.SM TET 
1.9 have been modified slightly to use the build configuration 
scheme used by the \*(xT.
This reduces the need to edit Makefiles to modify configuration variables
when building the 
.SM TET.
.LI
You should only refer to the instructions in the User Guide for the \*(xT 
for details of installation of the 
.SM TET .
.P
For more complete information on the features of the \s-1TET\s0, you can format 
and print the on-line documentation for the
.SM TET 
(see "\s-1TET\s0 Documentation").
.LI
It is intended that the \*(xT should work in conjunction with 
future versions of the
.SM TET 
later than 1.9.
.P
You can obtain the latest released version by 
sending electronic mail to infoserver@xopen.co.uk. A message body of 
.cS
request: tet
topic: index
request: end
.cE
will obtain the index of files available for the 
.SM TET .
.LE
.HU "Issues identified during pre-release testing"
.AL
.LI
A BadAccess error may be reported by the touch tests for the X Protocol 
requests ChangeHosts
and SetAccessControl if you have not been able to set up the X\ server so
that the client can change the access control list, and enable/disable
access control.
.P
Some X servers now support an option -ac, 
which disables host-based access control mechanisms.
Using this option will circumvent the problem.
.LI
When using the 
.SM TET
to build the \*(xT in space-saving mode, warning messages may be given 
indicating that output from the build process has been truncated. This occurs
because the Makefiles used in the build process include unusually long lines.
These messages are not serious - the output is not lost but is 
split over several lines.
.LE
.HU "Documentation for release 1.1"
The following documentation is provided for release 1.1 of the \*(xT.
.P
If you have already used these documents with the beta or gamma release,
you only need to refer to the 
changed sections (described in appendix A).
.P
Further details, including instructions for formatting and printing 
the files on the release media, are given in the file xtest/doc/README.
.AL
.LI
The User Guide
gives enough information to enable an experienced test suite user,
(not necessarily familiar with the \*(xW) to configure, build and 
execute the \*(xT, and analyse the results produced.
.P
You can find the source of the User Guide
in the file xtest/doc/userguide.mm on the release media, and in PostScript
form in file xtest/doc/userguide.ps.
.LI
The Programmers Guide
gives enough information to enable an experienced programmer 
familiar with the \*(xW to modify or extend the \*(xT.
.P
You can find the source of the Programmers Guide
in the file xtest/doc/progguide.mm on the release media, and in PostScript
form in file xtest/doc/progguide.ps.
.LE
.HU "TET Documentation"
You need only refer to the instructions in the
User Guide for the \*(xT for details of installation and usage of the 
.SM TET .
.P
For more background information on the features and scope of the 
.SM TET ,
you can format and print the following items of documentation which are 
part of the 
.SM TET .
.P
Any conflict between this documentation and the User Guide for the \*(xT is
unintentional. You should assume the User Guide is correct in case of conflict,
because it has been checked against the \*(xT.
.AL
.LI
The release note for 
.SM TET 
1.9 is supplied in the file 
tet/doc/posix_c/rel_note.mm on the release media, and in PostScript
form in file tet/doc/posix_c/rel_note.ps.
.P
To format rel_note.mm, you require the utilities tbl,
and nroff/troff with the mm macros.
.LI
A manual page for the tcc utility is provided in file 
tet/doc/posix_c/tcc.1.
.P
To format the man page, you require the utility
nroff/troff with the man macros.
.LE
.HU "Setting up your X\ server"
Your attention is drawn to section 7.1 of the User Guide entitled 
"Setting up your X\ server". You should follow the guidelines in section 
7.1.1 to obtain reliable, repeatable results against your X\ server,
when running formal verification tests.
.P
It is also important to ensure that your X\ server is running no other clients
before starting formal verification tests. This is because some test programs
(for example, those which enable access control) may interfere with later
tests unless the X\ server resets in between. To ensure the X\ server resets
after each test program, make sure you are not running any other clients 
at the time.
.SK
.H 1 "Appendix A - changes from previous releases"
.HU "Changes since the gamma release"
The following features have changed since the gamma release of the
\*(xT.
.AL
.LI
A number of changes have been made to fix bugs reported in the beta and 
gamma releases. 
.P
27 bug reports were received against the beta release
up to May 1, 1992.
.br
All of these are resolved, and these are listed in appendix C.
.P
21 bug reports were received against the gamma release
up to May 1, 1992.
.br
All of these are resolved, and these are listed in appendix D.
.LI
The name of the execution configuration parameter
.SM XT_HOSTNAME
has changed to 
.SM XT_DISPLAYHOST
to avoid confusion with the build configuration parameter
.SM XTESTHOST .
.LI
The 
.C -y
option to the 
.C tcc
utility
is now mentioned in the User Guide for the \*(xT. This enables the user to 
build, execute or clean only the test cases matching a specified pattern from a 
.SM TET
scenario file.
.LI
This release is a completely separate release from the
snapshot, alpha, beta and gamma releases,
and you should not attempt to 
install it over the top of 
previous releases, or to use it with any files supplied in the 
previous releases. 
.LE
.HU "New features since beta release"
The following features, which were first available in 
the gamma release, are also available in release 1.1 of the \*(xT.
.AL
.LI
There are a number of enhanced tests 
making use of the 
.SM XTEST
extension. The list of assertions for which we have provided enhanced
tests is in appendix B.
.P
Refer to the User Guide for information on how to build and run these 
tests correctly.
The changes are in sections 2.2, 3.3.5 
.SM ( SYSLIBS ), 
3.3.6 
.SM ( DEFINES ),
and 7.2.2
.SM ( XT_EXTENSIONS ).
.LI
There is now an option 
.C -s
to the 
.C mc
utility which will cause the test strategy to be output as a C source 
file comment between the assertion and the test code.
.LI
There are summaries of the utilities 
.C mc ,
.C mmkf
and 
.C ma
which are in separate pages of the appendices to the Programmers Guide.
.LE
.HU "New features since alpha release"
The following features, which were first available in 
the beta release, are also available in release 1.1 of the \*(xT.
.AL
.LI
There are a number of new execution configuration parameters which are
described in the User Guide. In particular, it is now possible to 
specify a delay to allow time for the X\ server to reset.
This is used by the test 
suite on startup of each test set and in other places where a server 
reset is expected following the shutdown of the last client.
.LI
There are now three source files which provide alternative ways to build the 
code for making connections to the X\ server when building the 
X\ Protocol library.
These are documented in the description of the build configuration 
parameter 
.SM XP_OPEN_DIS .
.P
None of these files makes any assumptions about the contents of the 
Display structure; this had caused problems building the alpha release.
.LI
The effects of drawing in the root window using subwindow-mode of 
IncludeInferiors is now tested for all relevant graphics functions.
.LI
Some tests which were thought to be untestable during
stage two have now been reviewed as a result of feedback received 
from alpha sites. In particular, tests are now provided for 
assertions 3, 4 and 5 for XChangePointerControl, and assertion 19 for
XCloseDisplay.
.LE
.HU "Changes since the alpha release"
The following features have changed since the alpha release of the
\*(xT.
.AL
.LI
A large number of changes have been made to fix bugs reported in the 
alpha release. 
.P
In total, 116 bugs reports were reported against the alpha release.
Out of these, 113 are resolved. The remaining
three bug reports are outstanding only in respect of requested 
enhancements.
.LI
All bugs reported against snapshot releases have now been resolved.
.LI
A number of minor improvements have been made to both the software and 
documentation following the reviews conducted by alpha sites.
.P
Improvements were suggested both during the stage two review meeting 
and after the meeting.
.LI
In the alpha release were a number of tests which printed the 
values of various screen and display 
parameters, and expected the user to verify these by inspection of the 
journal file. These were known as FIP tests.
These tests have been modified to automatically 
compare the values returned from 
the X\ server against execution configuration parameters. Thus, there are
no FIP tests in this release.
.LI
A number of the assertions for the X\ Protocol have been reclassified as
extended assertions since they cannot be portably tested. This affects
assertion 2 for OpenDisplay (invalid byte-orientation). This 
cannot be tested when XOpenDisplay is used to make connections, since the 
Xlib function always creates valid connections. 
.P
Also affected are the assertions which 
state that a BadLength error occurs when the request length exceeds the maximum
request length accepted by the X\ server. This is because the maximum 
request length accepted may be greater than or equal to all representable
request lengths.
.LI
In this release, only the dot-m files are supplied for each test set.
The Makefiles and dot-c files, supplied in the alpha release, are not 
supplied, but are made automatically when the release is built. This is 
mainly to save space in the distribution. Should you need to remake these
at any time, consult the Programmers Guide.
.LE
.HU "Changes from approved assertions"
There have been some changes to the wording of the assertions 
since their approval during stage two of the project.
.P
Changes have been made where statements were made in assertions that are
erroneous but were not picked up during internal or external review.
.P
Where assertions have been modified since the end of the 
external review, the original wording has been
retained in the dot-m file for comparison and is commented out.
.P
We have resisted minor wording changes that would not affect the 
test strategies.
.P
Assertions added during the review periods are in a 
logical place (rather than necessarily at the end of file).
.P
The effect of this is that the tests in Test.c will be a different 
order to the assertions in the emailed distributions where assertions
were inserted during the review period.
.HU "Classification of assertions"
As planned, we have classified the assertions into POSIX assertion 
categories A, B, C and D. These categories are explained further in the 
"Programmers Guide".
.P
As predicted during the assertion reviews, we have classified a number
of assertions in the "def" category. This is explained further in the 
"Programmers Guide". These are assertions for which an identical 
test elsewhere already fully tests the assertion. 
Altogether we have classified 140 "def" assertions out of 2732 assertions 
which are test set specific. There are a further 17 "def" assertions 
out of the 89 generic assertions for GC components. The GC assertions are
included in many of the tests which use GC components.
.P
Many of the "def"
assertions occur in the graphics tests (particularly those 
associated with text handling) where the test for the 
first assertion draws text, or obtains font information, using each test font. 
The test fonts contain characters which are adequate to
fully test all the remaining assertions for the function under test, 
so it is not possible to test those assertions any further.
.SK
.H 1 "Appendix B - list of extended assertions now tested"
The following list shows the extended assertions produced during stage two 
of the project for which we have now provided upgraded tests using the 
.SM XTEST 
extension. In total, there are 160 new tests.
.P
Six of these assertions can be tested
without use of the extension, so we have reclassified them as base 
assertions (category A or C).
Where there have been changes, the new categories are shown in 
brackets in the table below.
.TS
box, center;
l | l.
Test set	Extended assertions
_
CH03/chngwdwatt	11,20
CH03/crtsmplwdw	8
CH03/crtwdw	18
CH06/dfncrsr	1,2,3,4
CH06/undfncrsr	1,2
CH07/allwevnts	5,8,10-27
CH07/chngactvpn	3
CH07/grbbttn	1-31
CH07/grbky	1-9
CH07/grbkybrd	3,5-12,17(A)
CH07/grbpntr	11-15,20
CH07/gtmdfrmppn	2
CH07/gtpntrmppn	1,2
CH07/qrykymp	1
CH07/stinptfcs	2-4,9(A)
CH07/stmdfrmppn	6
CH07/stpntrmppn	3,5
CH07/ungrbbttn	1-4
CH07/ungrbky	1-4
CH07/ungrbkybrd	1
CH08/bttnprss	1-2,4-12
CH08/bttnrls	1-9
CH08/entrntfy	6(C),13(C)
CH08/kyprss	1-9
CH08/kyrls	1-9
CH08/lvntfy	6(C),13(C)
CH08/mtnntfy	3-10,14,18-19
.TE
.SK
.H 1 "Appendix C - list of resolved bugs on beta release"
.cS
.ps -2
0206:Subject: tcc: unterminated string literal in scenario.c
0207:Subject: XTestLib.c: needs Xlibint.h
0208:Subject: src/libproto: cross-device link
0209:Subject: sub-processes need environment to do authorization
0210:Subject: exec_startup continues test when XOpenDisplay fails
0211:Subject: Font compiler script "fcomp" used /bin/ksh, not /bin/sh.
0212:Subject: tetclean.cfg doesn't specify SHELL variable
0213:Subject: stclsshnt: incorrect type
0214:Subject: stwmprprts: incorrect type
0215:Subject: tet: error building tcc with ANSI C compiler
0216:Subject: build: tetbuild.cfg could have sample config parameters for AIXV3
0217:Subject: CH08/gtmtnevnts: execution stops
0218:Subject: crtpxmpfrm 2: assertion and code check for wrong error
0219:Subject: libproto: Allocatable() uses wrong macro
0220:Subject: XK_script_switch is not a good misc. function key.
0221:Subject: Use of zero width lines in CH05/stbg and CH05/ststt
0222:Subject: BadLength problems in src/libproto/SendSup.c
0223:Subject: opndspy 2: bad assertion, bad test
0224:Subject: fllarcs 2: a2.dat is still wrong
0225:Subject: XDisplayName isn't tested.
0226:Subject: clsdsply 11: buildtree
0227:Subject: onpdsply 3 and 4 bug
0242:Subject: CH04/stslctnown
0243:Subject: CH07/kllclnt, CH08/sndevnt
0245:Subject: enhanced tet_scen
0246:Subject: enhanced link_scen
0247:Subject: cpyar 1: if colormap creation fails, the test can deadlock
.ps +2
.cE
.SK
.H 1 "Appendix D - list of resolved bugs on gamma release"
.cS
.ps -2
0228:Subject: tet api fails to compile on SunOS
0229:Subject: libxtest.a: bad type in warppointer
0230:Subject: libxtest.a: missing declaration of Dsp in nbuttons
0231:Subject: tet api: install does not run ranlib
0232:Subject: lkpstr 2: use of Delete key not portable
0233:Subject: bug in tset/CH10/lkpstr
0234:Subject: entrntfy 6, 13 and lvntfy 6, 13: incorrect assumptions about events
0235:Subject: grbbttn 17, 18, 21: device is never thawed
0236:Subject: grbbttn 16: assertion does not make sense
0237:Subject: allwevnts 5: incorrect change of event mask
0238:Subject: allwevnts 17: presses wrong key
0239:Subject: allwevnts 19: incorrect strategy applied
0240:Subject: allwevnts 20: wrong event mask used
0241:Subject: pt: shell :- syntax not universal
0244:Subject: src/lib: windows on alternate screen can be off-screen
0248:Subject: tet: non-posix compile(?)
0249:Subject: tet: non-posix compile (?)
0250:Subject: strdup() function prototype
0251:Subject: strdup() function is defined incorrectly
0252:Subject: strdup() declaration is incorrect in xtest/src/bin/mc/main.c
0253:Subject: spelling error
.ps +2
.cE
