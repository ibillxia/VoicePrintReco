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
##   $Date: 2003/07/25 15:43:30 $
##   $Revision: 1.7 $
##

## The foreign option disables checks for GNU specific files such as README, NEWS...
AUTOMAKE_OPTIONS = 1.4 foreign

ACLOCAL_AMFLAGS = -I auxdir

LDADD    = -lm -L. -lspro @sphere_lib@
INCLUDES = @sphere_include@

include_HEADERS = spro.h
lib_LIBRARIES = libspro.a
noinst_HEADERS = getopt.h

libspro_a_SOURCES = system.h spro.h sig.c spf.c header.c misc.c lpc.c convert.c fft.c

bin_PROGRAMS = scopy slpc slpcep sfbank sfbcep
noinst_PROGRAMS = scompare

scopy_SOURCES = scopy.c getopt.c getopt1.c
scopy_DEPENDENCIES = libspro.a

slpc_SOURCES = slpc.c getopt.c getopt1.c
slpc_DEPENDENCIES = libspro.a

slpcep_SOURCES = slpcep.c getopt.c getopt1.c
slpcep_DEPENDENCIES = libspro.a

sfbank_SOURCES = sfbank.c getopt.c getopt1.c
sfbank_DEPENDENCIES = libspro.a

sfbcep_SOURCES = sfbcep.c getopt.c getopt1.c
sfbcep_DEPENDENCIES = libspro.a

scompare_SOURCES = scompare.c getopt.c getopt1.c
scompare_DEPENDENCIES = libspro.a

SUBDIRS = doc auxdir test
EXTRA_DIST = README INSTALL COPYING CHANGES
