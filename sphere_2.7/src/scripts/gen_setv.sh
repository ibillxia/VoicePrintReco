#!/bin/sh

sed -e "s:PROJECT_DIR_NAME:${PROJECT_ROOT}:"   \
       -e "s: SRC_DIR_NAMES: ${SRC_DIR_NAMES}:"  \
       -e "s: LIBRARY_NAMES: ${LIBRARY_NAMES}:"  \
       -e "s: LIBRARY_NAME: ${LIBRARY_NAME}:"  \
       -e "s: SOURCE_NAMES: ${SOURCE_NAMES}:"  \
       -e "s: OBJECT_NAMES: ${OBJECT_NAMES}:"  \
       -e "s: EXECUTABLE_NAMES: ${EXECUTABLE_NAMES}:"  \
       -e "s: PROG_SUBDIR_NAMES: ${PROG_SUBDIR_NAMES}:"  \
       -e "s: LINK_LIBRARY_NAMES: ${LINK_LIBRARY_NAMES}:"  \
       -e "s: LINK_LIBRARY_COMBINED: ${LINK_LIBRARY_COMBINED}:"  \
       -e "s: SCRIPT_FILES: ${SCRIPT_FILES}:"  \
       -e "s: COMPILER_COMMAND: ${COMPILER_COMMAND}:"  \
       -e "s: INSTALL_COMMAND: ${INSTALL_COMMAND}:"  \
       -e "s: RANLIB_COMMAND: ${RANLIB_COMMAND}:"  \
       -e "s: ARCHIVER_COMMAND: ${ARCHIVER_COMMAND}:"  \
       -e "s: LOCAL_CC_DEFINES: ${LOCAL_CC_DEFINES}:" 
