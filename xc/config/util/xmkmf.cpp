/**/#!/bin/sh

/**/#
/**/# generate a Makefile from an Imakefile from outside the X sources.
/**/# 

imake CONFIGDIRSPEC -sMakefile -DUseInstalled $*
