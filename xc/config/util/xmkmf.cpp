/**/#!/bin/sh

/**/#
/**/# generate a Makefile from an Imakefile from outside the X sources.
/**/# 

usage="usage:  $0 [topdirpathname]"

case $# in 
    0) ;;
    1) topdir=$1 ;;
    *) echo "$usage" 1>&2; exit 1 ;;
esac

case "$topdir" in
    -*) echo "$usage" 1>&2; exit 1 ;;
esac

if [ -f Makefile ]; then 
    echo mv Makefile Makefile.bak
    mv Makefile Makefile.bak
fi
echo imake -DUseInstalled CONFIGDIRSPEC $topdir
imake -DUseInstalled CONFIGDIRSPEC $topdir
