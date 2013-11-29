#=========================================================================
#                           Find Alize Library
#                    See http://alize.univ-avignon.fr/
#-------------------------------------------------------------------------
#
## 1: Setup:
# The following variables are optionally searched for defaults
#  Alize_DIR:            Base directory of Alize tree to use.
#
#
## 2: Variable
# The following are set after configuration is done: 
#  
#  Alize_INCLUDE_DIR    - where to find alize.h, etc.
#  Alize_LIBS   		- List of libraries when using Alize.
#  Alize_FOUND     		- True if Alize found.
# 
#
## 3: Version
#
# 2011/02/12 Pierre CLÉMENT, Creation of the script.
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
mark_as_advanced(Alize_LIBS Alize_INCLUDE_DIR)

# Already in cache, be silent
if(Alize_INCLUDE_DIR)
  set(Alize_FIND_QUIETLY TRUE)
endif(Alize_INCLUDE_DIR)

#=========================================================================
#  Find ALIZE libraries
#-------------------------------------------------------------------------
if(EXISTS "${Alize_DIR}")
	execute_process(COMMAND uname -s OUTPUT_VARIABLE OS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
	find_path(Alize_INCLUDE_DIR alize.h PATHS ${Alize_DIR}/include)
	find_library(Alize_LIBS NAMES alize_${OS}_${ARCH} PATHS ${Alize_DIR}/lib)
        find_library(Alize_LIBS NAMES alize PATHS ${Alize_DIR}/lib)
    include_directories(${Alize_INCLUDE_DIR})
	set(Alize_FOUND TRUE)
else(EXISTS "${Alize_DIR}")
	set(ERR_MSG "Please specify Alize directory using Alize_DIR env. variable.")
endif(EXISTS "${Alize_DIR}")


#=========================================================================
#  Print message
#-------------------------------------------------------------------------
if(NOT Alize_FOUND)
  # make FIND_PACKAGE friendly
  if(NOT Alize_FIND_QUIETLY)
        if(Alize_FIND_REQUIRED)
          message(FATAL_ERROR "Alize required but some headers or libs not found. ${ERR_MSG}")
        else(Alize_FIND_REQUIRED)
          message(STATUS "WARNING: Alize was not found. ${ERR_MSG}")
        endif(Alize_FIND_REQUIRED)
  endif(NOT Alize_FIND_QUIETLY)
endif(NOT Alize_FOUND)
##====================================================
