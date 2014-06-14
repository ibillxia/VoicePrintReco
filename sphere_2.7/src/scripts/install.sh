#!/bin/sh
#
# File:      install
#
#   The main installation script for the NIST software distributions
#
#   Changes since the original release:
#
#   1.01  Corrected echo's behavious on systemV systems
#   1.02  removed the shell functions for get_field and get_field_ids
#   1.03  Added new system definitions
#   1.04  Corrected the Custom Variable

#### Define the echo command
if test "`echo -n foobar | egrep -e -n`" = "" ; then
	ECHO_NNL="echo -n"
	DONE_CMD="echo Done"
else
	ECHO_NNL="echo"
	DONE_CMD=""
fi

VERSION=1.04
echo "Installation Script Version $VERSION for the NIST Speech utilities"
echo ""
$ECHO_NNL "Searching for packages . . . "
packages=`(cd lib/packages; ls *.pkg)|sed 's/.pkg//'`
numpackages=`echo $packages| tr ' ' '\012'|sed '/^$/d' |wc -l|awk '{print $1}'`
$DONE_CMD
echo ""
echo "Currently, there are $numpackages loaded and ready to install"
for pkg in $packages ; do
    echo "    `head -1 lib/packages/$pkg.pkg`"
done
echo ""

echo "Before Installation, this script needs information concerning"
echo "the computing environment on which this package is being compiled."
echo ""

echo "This install script has definitions for several operating system"
echo "environments on which this package may be installed.  If your"
echo "computing environment is not listed, the custom installation will"
echo "prompt for the necessary information."
echo ""

echo "This list is NOT a list of supported platforms.  For a list of"
echo "supported platforms, consult the package documentation files in"
echo "doc/*.doc or in the readme.txt in the top level directory."
echo ""

format="ID,OS,Compiler_Command,Compiler_Flags,Install_Command,Archive_Sorting_Command,Archive_Update_Command,Architecture"
defs=`echo '1:Linux:gcc -m32:-g:install -s -m 755:ranlib:ar ru:LINUX
2:OSX:gcc -m32 -gstabs:-g:install -s -m 755:ranlib:ar ru:LINUX
3:cygwin:gcc -m32:-g:install -s -m 755:ranlib:ar ru:LINUX
4:Custom:cc:-g:install -s -m 755:ranlib:ar ru:SUN' | tr '\012' ','`
Custom=4

#get_field(){
#    # $1 the definition string
#    # $2 is the identifier
#    # $3 is the field number
#    echo "$1" | tr ',' '\012' | egrep "^$2:" | awk -F: "{print $"$3"}"
#}

ans='n'
while test "$ans" = "n" ; do
    # get all the field keys
    for num in `echo "$defs" | tr ',' '\012' | awk -F: '{print $1}'` ; do
#	id=`get_field "$defs" $num 2`
        id=`echo "$defs" | tr ',' '\012' | egrep "^${num}:" | \
		awk -F: '{print $2}'`
        echo "$num:$id"  | awk -F: '{printf("   %6s:  %s\n",$1,$2)}'
    done
    echo ""
    echo "Please Choose one:  "
    $ECHO_NNL "   "
    read system
#    id=`get_field "$defs" $system 1`
    id=`echo "$defs" | tr ',' '\012' | egrep "^${system}:" | \
	awk -F: '{print $1}'`

    if test "$id" != "" ; then
	echo ""
#	echo "Using the `get_field "$defs" $system 2` Defaults"
	te=`echo "$defs" | tr ',' '\012' | egrep "^${system}:" | \
		awk -F: '{print $2}'`
	echo "Using the $te Defaults"
	echo ""
	ans="y"
    else
        echo "Invalid Answer.  Choose one of the following"
	echo ""
    fi
done

if test "$system" = "$Custom" ; then
    number=3;
    field_values="Custom:"
    for topic in `echo $format | sed 's/^[^,]*,[^,]*,//' | tr ',' ' '` ; do
	ans="n"
#        command=`get_field "$defs" $system $number`
	command=`echo "$defs" | tr ',' '\012' | egrep "^${system}:" | \
		awk -F: '{print $'$number'}'`
	while test "$ans" = "n" ; do
            echo "What is/are the `echo $topic|tr '_' ' '` ? [$command]"
            read query
	    if test "$query" = "" ; then
                ans="y"
                field_values="${field_values}${command}:"
            else
		command="$query"
                echo "OK, The $topic command is '$command'.  Is this OK? [yes]"
                read qu
                if test "$qu" = "" -o "$qu" = "yes" -o "$qu" = "Y" -o "$qu" = "y" ; then
                    ans="y"
                    field_values="${field_values}${command}:"
                fi
            fi
        done
	number=`expr $number + 1`
    done
else
    field_values=`echo "$defs" | tr ',' '\012' | egrep "^$system:" | sed 's/^[^:]*://'`
fi

COMPILER_COMMAND=`echo "$field_values" | awk -F: '{print $2}'` ; \
			export COMPILER_COMMAND
INSTALL_COMMAND=`echo "$field_values" | awk -F: '{print $4}'` ; \
			export INSTALL_COMMAND
RANLIB_COMMAND=`echo "$field_values" | awk -F: '{print $5}'` ; \
			export RANLIB_COMMAND
ARCHIVER_COMMAND=`echo "$field_values" | awk -F: '{print $6}'` ; \
			export ARCHIVER_COMMAND
LOCAL_CC_DEFINES=`echo "$field_values" | awk -F: '{print $3}'`\ -DNARCH_`echo "$field_values" | awk -F: '{print $7}'` ; export LOCAL_CC_DEFINES

echo "Defined Commands and Options:"
echo "   Compiler           = $COMPILER_COMMAND"
echo "   Compiler options   = $LOCAL_CC_DEFINES"
echo "   Install            = $INSTALL_COMMAND"
echo "   Ranlib             = $RANLIB_COMMAND"
echo "   Archiver           = $ARCHIVER_COMMAND"
echo ""

# first do a chmod on the directoryies and files
echo "File Permissions:"
echo "     If this distribution was loaded from a CD-rom, it is imperative"
echo "     to change the permissions of all files and directories to be writable"
echo "     by you.  If not, the installation process will fail."
echo ""
echo "Changing Permisions on Directories to 755"
find . -type d -exec chmod 755 \{\} \;
echo "Changing Permisions on Files to 644"
find . -type f -exec chmod 644 \{\} \;

# run each installation script for the packages"
for pkg in $packages ; do
    pkg_id=`head -1 lib/packages/$pkg.pkg`
    if test -f lib/packages/$pkg.pas ; then
        if test "`diff lib/packages/$pkg.pas lib/packages/$pkg.pkg`" = "" ; then
            echo "Package $pkg_id Already installed"
        else
	    echo "Running Installation for $pkg_id"
	    if test -f src/scripts/$pkg.ins ; then
		sh src/scripts/$pkg.ins
        	if test "$?" != "0" ; then
	            echo "    Install script for $pkg_id failed, Call NIST"
        	    exit 1
        	fi
	    fi
	    cp lib/packages/$pkg.pkg lib/packages/$pkg.pas
        fi
    else
	echo "Running Installation for $pkg_id"
	if test -f src/scripts/$pkg.ins ; then
	    sh src/scripts/$pkg.ins
            if test "$?" != "0" ; then
	        echo "    Install script for $pkg_id failed, Call NIST"
        	exit 1
            fi
	fi
	cp lib/packages/$pkg.pkg lib/packages/$pkg.pas
    fi
done
echo ""

echo "Creating Makefiles . . . "
sh src/scripts/gen_make.sh `pwd`/src/scripts

echo "Creating executables . . . "
make install
