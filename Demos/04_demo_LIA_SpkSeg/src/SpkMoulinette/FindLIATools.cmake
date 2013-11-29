#=========================================================================
#                           Find LIATools Library
#                    See http://alize.univ-avignon.fr/
#-------------------------------------------------------------------------
#
## 1: Setup:
# The following variables are optionally searched for defaults
#  LIATools_DIR:            Base directory of LIATools tree to use.
#
#
## 2: Variable
# The following are set after configuration is done: 
#  
#  LIATools_INCLUDE_DIR	- where to find liatools.h, etc.
#  LIATools_LIBS   		- List of libraries when using LIATools.
#  LIATools_FOUND     	- True if LIATools found.
# 
#
## 3: Version
#
# 2011/03/21 Pierre CLÉMENT, Creation of the script.
#
#
## 4: Licence:
#
# LGPL 3.0 : GNU Lesser General Public License Usage
# Alternatively, this file may be used under the terms of the GNU Lesser

# General Public License version 3.0 as published by the Free Software
# Foundation and appearing in the file LICENSE.LGPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU Lesser General Public License version 3.0 requirements
# will be met: http://www.gnu.org/licenses/lgpl.html
#
#-------------------------------------------------------------------------

# For GUI tools, show these variables only in "Advance"
mark_as_advanced(LIATools_LIBS LIATools_INCLUDE_DIR)

# Already in cache, be silent
if(LIATools_INCLUDE_DIR)
  set(LIATools_FIND_QUIETLY TRUE)
endif(LIATools_INCLUDE_DIR)

#=========================================================================
#  Find LIATools libraries
#-------------------------------------------------------------------------
if(EXISTS "${LIATools_DIR}")
	execute_process(COMMAND uname -s OUTPUT_VARIABLE OS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
	find_path(LIATools_INCLUDE_DIR liatools.h PATHS ${LIATools_DIR}/include)
	find_library(LIATools_LIBS NAMES liatools_${OS}_${ARCH} PATHS ${LIATools_DIR}/../lib)
	include_directories(${LIATools_INCLUDE_DIR})
	set(LIATools_FOUND TRUE)
else(EXISTS "${LIATools_DIR}")
	set(ERR_MSG "Please specify LIATools directory (LIA_RAL/LIA_SpkTools) using LIATools_DIR env. variable.")
endif(EXISTS "${LIATools_DIR}")


#=========================================================================
#  Print message
#-------------------------------------------------------------------------
if(NOT LIATools_FOUND)
  # make FIND_PACKAGE friendly
  if(NOT LIATools_FIND_QUIETLY)
        if(LIATools_FIND_REQUIRED)
          message(FATAL_ERROR "LIATools required but some headers or libs not found. ${ERR_MSG}")
        else(LIATools_FIND_REQUIRED)
          message(STATUS "WARNING: LIATools was not found. ${ERR_MSG}")
        endif(LIATools_FIND_REQUIRED)
  endif(NOT LIATools_FIND_QUIETLY)
endif(NOT LIATools_FOUND)
##====================================================
