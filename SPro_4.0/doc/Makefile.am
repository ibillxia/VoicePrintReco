#############################################################################
##                                                                         ##
##                               Makefile.am                               ##
##                                                                         ##
##                               SPro toolkit                              ##
##                                                                         ##
## Guig                                                    Fri Jul  5 2002 ##
## ----------------------------------------------------------------------- ##
##                                                                         ##
##  Copyright (C) 2002 Guillaume Gravier (ggravier@irisa.fr)               ##
##                                                                         ##
##  This program is free software; you can redistribute it and/or modify   ##
##  it under the terms of the GNU General Public License as published by   ##
##  the Free Software Foundation; either version 2, or (at your option)    ##
##  any later version.                                                     ##
##                                                                         ##
##  This program is distributed in the hope that it will be useful,        ##
##  but WITHOUT ANY WARRANTY; without even the implied warranty of         ##
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          ##
##  GNU General Public License for more details.                           ##
##                                                                         ##
##  You should have received a copy of the GNU General Public License      ##
##  along with this program; if not, write to the Free Software            ##
##  Foundation, Inc., 59 Temple Place - Suite 330,                         ##
##  Boston, MA 02111-1307, USA.                                            ##
##                                                                         ##
#############################################################################
##
## CVS log:
##
##   $Author: ggravier $
##   $Date: 2003/08/22 16:18:30 $
##   $Revision: 1.4 $
##

MAKEINFO = @MAKEINFO@ --no-split
TEXI2HTML = texi2html
TEXI2HTML_FLAGS = -split_chapter

info_TEXINFOS = spro.texi 
spro_TEXINFOS = changes.texi index.texi intro.texi library.texi license.texi reference.texi speech.texi user.texi version.texi

# Files from texi2dvi that should be removed, but which Automake does
# not know.
CLEANFILES = spro.acs  spro.cvs  spro.mss  spro.prs \
             spro.ats  spro.evs  spro.fns  spro.ovs \
             spro.tmp spro*.html spro*.pdf spro.ps spro.cps 

EXTRA_DIST = asvar-fig1.eps fbank.eps framing.eps \
             asvar-fig1.jpg fbank.jpg framing.jpg \
             asvar-fig1.pdf fbank.pdf framing.pdf

## ----------------------------- ##
## Other documentation formats.  ##
## ----------------------------- ##

html: spro.html

spro.html: spro.texi 
	$(TEXI2HTML) $(TEXI2HTML_FLAGS) $(srcdir)/spro.texi

pdf: spro.pdf

postscript: spro.ps

spro.pdf: spro.texi changes.texi index.texi intro.texi library.texi license.texi reference.texi speech.texi user.texi version.texi
	$(TEXI2DVI) --pdf --batch $(srcdir)/spro.texi

spro.ps: spro.texi changes.texi index.texi intro.texi library.texi license.texi reference.texi speech.texi user.texi version.texi
	$(TEXI2DVI) --batch $(srcdir)/spro.texi
	$(DVIPS) -o $(srcdir)/spro.ps $(srcdir)/spro

.PHONY: html pdf postscript
