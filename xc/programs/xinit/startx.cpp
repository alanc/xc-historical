/**/#!/bin/sh

xinit $*

#ifdef macII
Xrepair
screenrestore
#endif

#ifdef sun
kbd_mode -a
#endif

