# 
#  Copyright 1990, 1991 by the Massachusetts Institute of Technology and
#  UniSoft Group Limited.
#  
#  Permission to use, copy, modify, distribute, and sell this software and
#  its documentation for any purpose is hereby granted without fee,
#  provided that the above copyright notice appear in all copies and that
#  both that copyright notice and this permission notice appear in
#  supporting documentation, and that the names of MIT and UniSoft not be
#  used in advertising or publicity pertaining to distribution of the
#  software without specific, written prior permission.  MIT and UniSoft
#  make no representations about the suitability of this software for any
#  purpose.  It is provided "as is" without express or implied warranty.
# 
# $XConsortium: Makefile,v 1.3 92/07/06 18:50:01 rws Exp $

#
# Makefile for building the TET and the X test suite libraries/utilities.
#
# For full details on what needs to be built and how to build, refer
# to the User Guide in file userguide.mm in the directory xtest/doc.
# This Makefile just performs some of the required steps more easily.
#
# First, work through the instructions in the section entitled 
# "Configuring the X test suite" in the User Guide. This involves 
# setting the configuration parameters correctly for your system.
# This also includes setting the TET_ROOT environment variable to the 
# directory in which this Makefile is supplied.
#
# Ensure your PATH environment variable is set to include $TET_ROOT/xtest/bin.
#
# You may then need to build the portability library, if your system is not 
# a POSIX.1 system. This is described further in the User Guide.
#
# Then, you can perform the following steps described in the User Guide
# by just typing "make" in this directory:
#
# Building the TET:
# 1. The Test Case Controller (TCC)
# 2. The API library
#
# Building the X test suite libraries and utilities:
# 1. Building the X test suite library
# 2. Building the X Protocol library
# 3. Building the X test fonts library
# 4. Building the mc utility
# 5. Building the blowup utility
#
# You then need to build and install the test fonts as described in the 
# User Guide.
#

all: install

install:
	cd tet/src/posix_c/tools; pmake install
	cd tet/src/posix_c/api; pmake install
	cd xtest/src/lib; pmake install
	cd xtest/src/libproto; pmake install
	cd xtest/src/xim; pmake install
	cd xtest/fonts; pmake install
	cd xtest/src/bin/mc ; pmake install
	cd xtest/src/pixval/blowup; pmake install

clean:
	cd tet/src/posix_c/tools; pclean
	cd tet/src/posix_c/api; pclean
	cd xtest/src/lib; pclean
	cd xtest/src/libproto; pclean
	cd xtest/src/xim; pclean
	cd xtest/fonts; pclean
	cd xtest/src/bin/mc ; pclean
	cd xtest/src/pixval/blowup; pclean
