#!/bin/sh
#
# File:       gen_make.sh
#
# This script generates makefiles for the distrubution 
# automatically.
#
#  Changes from original version:
#
#  2.0 - 
#        Changed the source code directorys in 'src' to be 'bin' and 'lib'
#  2.1 -
#        Added new libraries, and corrected the handling of the scripts 
#        directory
#  2.2 - 
#        Removed the use of shell functions
#  2.3 -
#        Removed the use of exec'd variables, they're non-portable
#  2.4 - cleaned up the 'echo' and 'done' printouts for system V

VERSION=2.4
echo "gen_make version $VERSION"
SCRIPT_DIR=$1

#### Define the echo command
if test "`echo -n foobar | egrep -e -n`" = "" ; then
        ECHO_NNL="echo -n"
	DONE_CMD="echo Done"
else
        ECHO_NNL="echo"
	DONE_CMD=""
fi

if test ! -n "${COMPILER_COMMAND}" -o ! -n "${INSTALL_COMMAND}" -o ! -n "${RANLIB_COMMAND}" -o ! -n "${ARCHIVER_COMMAND}" -o ! -n "${LOCAL_CC_DEFINES}" ; then
        echo "Error: Environment variables must be set for:"
        echo "     COMPILER_COMMAND = ${COMPILER_COMMAND}"
        echo "     INSTALL_COMMAND  = ${INSTALL_COMMAND}"
        echo "     RANLIB_COMMAND   = ${RANLIB_COMMAND}"
        echo "     ARCHIVER_COMMAND = ${ARCHIVER_COMMAND}"
        echo "     LOCAL_CC_DEFINES = ${LOCAL_CC_DEFINES}"
        exit
fi

PROJECT_ROOT=`pwd`; export PROJECT_ROOT;

reset_variables_command='SRC_DIR_NAMES=""; export SRC_DIR_NAMES; 
        LIBRARY_NAMES="";       export LIBRARY_NAMES;
        LIBRARY_NAME="";        export LIBRARY_NAME;
        SOURCE_NAMES="";        export SOURCE_NAMES;
        OBJECT_NAMES="";        export OBJECT_NAMES;
        EXECUTABLE_NAMES="";    export EXECUTABLE_NAMES;
        PROG_SUBDIR_NAMES="";   export PROG_SUBDIR_NAMES;
        LINK_LIBRARY_NAMES="";  export LINK_LIBRARY_NAMES;
        SCRIPT_FILES="";        export SCRIPT_FILES;'  

LINK_LIBRARY_COMBINED="sphereCombinedLibs"; export LINK_LIBRARY_COMBINED;

$ECHO_NNL "Initialize main makefile . . . "
eval $reset_variables_command
sh $SCRIPT_DIR/gen_setv.sh < lib/makes/make_prj.txt > Makefile
$DONE_CMD

$ECHO_NNL "Initialize src Makefile . . . "
eval $reset_variables_command
src_dirs="lib bin scripts"
SRC_DIR_NAMES=$src_dirs ;       export SRC_DIR_NAMES;
sh $SCRIPT_DIR/gen_setv.sh < lib/makes/make_src.txt > src/Makefile
$DONE_CMD

eval $reset_variables_command
for srcdir in $src_dirs ; do
    if test "`echo $srcdir | grep lib`" != "" ; then
        $ECHO_NNL "    Initializing src dir $srcdir as a library dir . . . "
        lib_dirs=`(cd src/$srcdir ; ls -d * )|grep -v Makefile|grep -v CVS|grep -v readme|sort -r|tr '\012' ' '`
        eval $reset_variables_command
        SRC_DIR_NAMES="$src_dirs" ; export SRC_DIR_NAMES;
        LIBRARY_NAMES="$lib_dirs" ; export LIBRARY_NAMES;
        sh $SCRIPT_DIR/gen_setv.sh < lib/makes/make_lib.txt > src/$srcdir/Makefile
        $DONE_CMD
        for srclibdir in $lib_dirs ; do
            linklibs="$linklibs -l$srclibdir"
            $ECHO_NNL "        Initializing src dir $srcdir/$srclibdir . . . "
            src_names=`(cd src/$srcdir/$srclibdir ; ls -d *.c)|tr '\012' ' '`
            obj_names=`(cd src/$srcdir/$srclibdir ; ls -d *.c)|sed 's/.c$/.o/'|tr '\012' ' '`
            eval $reset_variables_command
            SRC_DIR_NAMES="$src_dirs" ; export SRC_DIR_NAMES;
            LIBRARY_NAMES="$lib_dirs" ; export LIBRARY_NAMES;
            LIBRARY_NAME="$srclibdir" ; export LIBRARY_NAME;
            SOURCE_NAMES="$src_names" ; export SOURCE_NAMES;
            OBJECT_NAMES="$obj_names" ; export OBJECT_NAMES;
            sh $SCRIPT_DIR/gen_setv.sh < lib/makes/make_lnm.txt \
                          > src/$srcdir/$srclibdir/Makefile            
            $DONE_CMD
        done
    elif test "`echo $srcdir | grep bin`" != "" ; then
        $ECHO_NNL "    Initializing src dir $srcdir as a program dir . . . " 
        prg_names=`(cd src/$srcdir ; ls -d *.c)| tr '\012' ' '`
        exe_names=`(cd src/$srcdir ; ls -d *.c)| sed 's/\.c//g'|tr '\012' ' '`
        prgsubdir=`(cd src/$srcdir ; find . -type d -print | grep -v CVS | grep ./. |sed 's:./::') \
                   |tr '\012' ' '` 
        eval $reset_variables_command
        SRC_DIR_NAMES="$src_dirs" ;      export SRC_DIR_NAMES;
        LIBRARY_NAMES="$lib_dirs" ;      export LIBRARY_NAMES;
        LIBRARY_NAME="" ;                export LIBRARY_NAME;
        SOURCE_NAMES="$prg_names" ;      export SOURCE_NAMES;
        OBJECT_NAMES="" ;                export OBJECT_NAMES;
        EXECUTABLE_NAMES="$exe_names" ;  export EXECUTABLE_NAMES;
        PROG_SUBDIR_NAMES="$prgsubdir" ; export PROG_SUBDIR_NAMES;
#        LINK_LIBRARY_NAMES="`echo $linklibs | tr ' ' '\012' | \
#                                   sh $SCRIPT_DIR/gen_lord.sh | \
#                                   sort -t_ -n +1 | sed s/_[0-9]$// | \
#                                   tr '\012' ' ' `" ; export  LINK_LIBRARY_NAMES
        if test "$prgsubdir" = "" ; then
            sh $SCRIPT_DIR/gen_setv.sh < lib/makes/make_spg.txt > src/$srcdir/Makefile
        else
            sh $SCRIPT_DIR/gen_setv.sh < lib/makes/make_prg.txt > src/$srcdir/Makefile
        fi
        $DONE_CMD
        
        for prgsub in $prgsubdir ; do
            $ECHO_NNL "       Initializing src dir $srcdir/$prgsub as a program dir . . ."
            prg_names=`(cd src/$srcdir/$prgsub ; ls -d *.c)|tr '\012' ' '`
            obj_names=`(cd src/$srcdir/$prgsub ; ls -d *.c)| egrep -v "^$prgsub.c"|sed 's/\.c/.o/' |tr '\012' ' '`
            eval $reset_variables_command
            SRC_DIR_NAMES="$src_dirs";  export SRC_DIR_NAMES;
            LIBRARY_NAMES="$lib_dirs";  export LIBRARY_NAMES;
            LIBRARY_NAME="";            export LIBRARY_NAME;
            SOURCE_NAMES="$prg_names";  export SOURCE_NAMES;
            OBJECT_NAMES="$obj_names";  export OBJECT_NAMES;
            EXECUTABLE_NAMES="$prgsub"; export EXECUTABLE_NAMES;
            PROG_SUBDIR_NAMES="";       export PROG_SUBDIR_NAMES;
#            LINK_LIBRARY_NAMES="`echo $linklibs | tr ' ' '\012' | \
#                                   sh $SCRIPT_DIR/gen_lord.sh | \
#                                   sort -t_ -n +1 | sed s/_[0-9]$// | \
#                                   tr '\012' ' ' `"; export LINK_LIBRARY_NAMES;
            sh $SCRIPT_DIR/gen_setv.sh < lib/makes/make_spg.txt > src/$srcdir/$prgsub/Makefile             
            $DONE_CMD
        done
    elif test "`echo $srcdir | grep scripts`" != "" ; then
        $ECHO_NNL "    Initializing src dir $srcdir as a script dir . . . " 
        scripts=`(cd src/$srcdir ; ls -d *)|grep -v Makefile|grep -v readme | \
                        grep -v cleandst.sh |grep -v gen_make.sh | grep -v install.sh | \
                        grep -v '.ins\$' | tr '\012' ' '`
        eval $reset_variables_command
        SCRIPT_FILES="$scripts" ; export SCRIPT_FILES;
        sh $SCRIPT_DIR/gen_setv.sh < lib/makes/make_scr.txt > src/$srcdir/Makefile
        $DONE_CMD
    else
        echo "not $srcdir"
    fi
        
done    




