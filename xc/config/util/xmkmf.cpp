/**/#!/bin/sh

/**/#
/**/# generate a Makefile from an Imakefile from inside or outside the sources!
/**/# 

usage="usage:  $0 [top_of_sources_pathname]"

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

if [ "$topdir" = "" ]; then
    arginst=-DUseInstalled
    argtopdir=
else
    arginst=
    argtopdir=-DTOPDIR=$topdir
fi

echo imake $arginst CONFIGDIRSPEC $argtopdir
imake $arginst CONFIGDIRSPEC $argtopdir
