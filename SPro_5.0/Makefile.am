#############################################################################
##                                                                         ##
##                               Makefile.am                               ##
##                                                                         ##
##                               SPro toolkit                              ##
##                                                                         ##
## Guig                                                    Fri Jul  5 2002 ##
## ----------------------------------------------------------------------- ##
## 
##    $Author: guig $
##    $Date: 2010-11-09 16:57:22 +0100 (Tue, 09 Nov 2010) $
##    $Revision: 151 $
## 
##  
##    Copyright (C) 1997-2010 Guillaume Gravier (guig@irisa.fr)
## 
##    Permission is hereby granted, free of charge, to any person
##    obtaining a copy of this software and associated documentation
##    files (the "Software"), to deal in the Software without
##    restriction, including without limitation the rights to use, copy,
##    modify, merge, publish, distribute, sublicense, and/or sell copies
##    of the Software, and to permit persons to whom the Software is
##    furnished to do so, subject to the following conditions:
## 
##    The above copyright notice and this permission notice shall be
##    included in all copies or substantial portions of the Software.
## 
##    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
##    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
##    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
##    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
##    BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
##    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
##    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
##    SOFTWARE.
## 

## The foreign option disables checks for GNU specific files such as README, NEWS...
AUTOMAKE_OPTIONS = 1.4 foreign

ACLOCAL_AMFLAGS = -I auxdir

LDADD    = -lm -L. -lspro @sphere_lib@
INCLUDES = @sphere_include@

include_HEADERS = spro.h sptables.h
lib_LIBRARIES = libspro.a
noinst_HEADERS = gggetopt.h

libspro_a_SOURCES = system.h spro.h sptables.h sig.c spf.c header.c misc.c lpc.c convert.c fft.c

bin_PROGRAMS = scopy slpc slpcep sfbank sfbcep splp
noinst_PROGRAMS = scompare

scopy_SOURCES = scopy.c gggetopt.c
scopy_DEPENDENCIES = libspro.a

slpc_SOURCES = slpc.c gggetopt.c
slpc_DEPENDENCIES = libspro.a

slpcep_SOURCES = slpcep.c gggetopt.c
slpcep_DEPENDENCIES = libspro.a

sfbank_SOURCES = sfbank.c gggetopt.c
sfbank_DEPENDENCIES = libspro.a

sfbcep_SOURCES = sfbcep.c gggetopt.c
sfbcep_DEPENDENCIES = libspro.a

splp_SOURCES = splp.c gggetopt.c
splp_DEPENDENCIES = libspro.a

scompare_SOURCES = scompare.c gggetopt.c
scompare_DEPENDENCIES = libspro.a

SUBDIRS = doc auxdir test
EXTRA_DIST = README INSTALL COPYING CHANGES
