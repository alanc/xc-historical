/**/#!/bin/sh

/**/#
/**/# generate a Makefile from an Imakefile from outside the X sources.
/**/# 

imake CONFIGDIRSPEC -TImake.tmpl -sMakefile -DUseInstalled Makefile
