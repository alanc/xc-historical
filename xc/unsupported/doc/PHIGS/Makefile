##
# $XConsortium$
##
## 
## Copyright (c) 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.
## 
##			 All Rights Reserved
## 
## Permission to use, copy, modify, and distribute this software and its 
## documentation for any purpose and without fee is hereby granted, 
## provided that the above copyright notice appear in all copies and that
## both that copyright notice and this permission notice appear in 
## supporting documentation, and that the names of Sun Microsystems,
## the X Consortium, and MIT not be used in advertising or publicity 
## pertaining to distribution of the software without specific, written 
## prior permission.  
## 
## SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
## INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
## EVENT SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
## CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
## USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
## OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
## PERFORMANCE OF THIS SOFTWARE.

DOCBIN=..

XREF=$(DOCBIN)/xref

RECODE=$(DOCBIN)/recode

INDEXER=$(DOCBIN)/indexer

PEXMACROS=$(DOCBIN)/pex.macros

all: cover.dit doc.dit index.dit 

PHIGSMAN = phigs.macros
INDEX_MACROS_STD = index.macros
NAMES = names.make

include $(NAMES)

TROFF=/usr/doctools/bin/troff
TBL=/usr/doctools/bin/tbl
EQN=/usr/doctools/bin/eqn

doc.dit: $(NAMES)
	(cat $(PHIGSMAN) $(INDEX_MACROS_STD) $(NAMES) ; echo .tm section 1: \\n%) | $(TBL) | $(EQN) | $(TROFF) -rC1 -rD1 -rF1 -rj$$FORTRAN -t > draft/doc.dit 2>> draft/book.temp

cover.dit: cover.phigs $(PHIGSMAN)
	$(TROFF) -rj$$FORTRAN -t -ms cover.phigs > draft/cover.dit 2>> draft/book.temp

index.dit: $(INDEX_MACROS_STD) doc.dit
	$(RECODE) < draft/book.temp > draft/book.file
	grep '^\.IE' draft/book.file | (echo .UH INDEX ; $(INDEXER)) | $(TROFF) -mex -rC1 -rD1 -rF1 -t -rP`grep 'section 1:' draft/book.temp | awk '{printf("%s", $$3+(($$3%2)+1))}'` > draft/index.dit 2>> draft/book.temp

# PRINTING TARGETS

print:
	ditprint draft/cover.dit
	ditsheet -o1-99 draft/doc.dit | ditprint
	ditsheet -o100-199 draft/doc.dit | ditprint
	ditsheet -o200-299 draft/doc.dit | ditprint
	ditsheet -o300-399 draft/doc.dit | ditprint
	ditsheet -o400-499 draft/doc.dit | ditprint
	ditsheet -o500-599 draft/doc.dit | ditprint
	ditsheet -o600-699 draft/doc.dit | ditprint
	ditsheet -o700-799 draft/doc.dit | ditprint
	ditsheet -o800- draft/doc.dit | ditprint
	ditprint draft/index.dit

clean:
	rm -f draft/book* draft/*.dit
