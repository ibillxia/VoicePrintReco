#!/bin/bash

####################################################################
#	Featrure extraction for speaker verification system based on ALIZE 3.0
#
#	This script is distributed in the hope that it will useful but WITHOUT ANY WARRANTY.
#
#	This tutorial is designed to help getting an hand on the LIA_RAL toolkit  by introducting 
#	the main executables in context.
#
#	Executables used in this script are NOT part of the ALIZE toolkit.
#	The author of this tutorial recommand the use of these tools as he is familiar with.
#	However, acoustic feature extraction is not limited to Spro and HTK and 
#	other tools can be used for this purpose.
#
#	by Anthony Larcher
#		alarcher - at- i2r.a-star.edu.sg
#
#		07/06/2013
####################################################################

# This script extract acoustic features from the SPHERE files available in the ./data/sph/ directory and generate feature files in ./data/prm/

# Before running this script you should select a configuration  by setting the two following parameters.
# FEATURE_TYPE design the program used to extract the acoustic features: Spro or HTK
#
# In case Spro is selected and that sfbcep is not compiled linked to the SPHERE library, SPHERE files need to be converted into 
# raw PCM files by using the tools provided by the NIST and available at:
#
#	http://www.speech.cs.cmu.edu/comp.speech/Section1/AudioSoftware/nist.html
#
# In this case, set the parameter INPUT_FORMAT to PCM
#  
FEATURE_TYPE="SPro"		# can be SPro or HTK
INPUT_FORMAT="SPH"		# can be SPH or PCM


# If SPro has not been linked to the SPHERE library, convert first the SPHERE files into raw PCM files without header
if [ $INPUT_FORMAT = "PCM" ]; then

	for i in `cat data/data.lst`; do

		CMD_DECODE="bin/w_decode -o pcm data/sph/$i.sph data/pcm/$i.sph"
		echo $CMD_DECODE
		$CMD_DECODE

		CMD_CONVERT="bin/h_strip data/pcm/$i.sph data/pcm/$i.pcm"
		echo $CMD_CONVERT
		$CMD_CONVERT
	done

	# Remove the temporary SPHERE files
	echo "Remove temporary SPHERE files"
	rm data/pcm/*.sph

	# Extract MFCC features with SPro
	for i in `cat data/data.lst`;do
        	COMMAND_LINE="bin/sfbcep -m -k 0.97 -p19 -n 24 -r 22 -e -D -A -F PCM16  data/pcm/$i.pcm data/prm/$i.tmp.prm"
                echo $COMMAND_LINE
                $COMMAND_LINE
   	done

fi


if [ $INPUT_FORMAT = "SPH" ]; then


	if [ $FEATURE_TYPE = "SPro" ]; then

		# Extract a list of files
		for i in `cat data/data.lst`;do
	                COMMAND_LINE="bin/sfbcep -m -k 0.97 -p19 -n 24 -r 22 -e -D -A -F SPHERE  data/sph/$i.sph data/prm/$i.tmp.prm"
	 		echo $COMMAND_LINE
	      		$COMMAND_LINE
		done

	else	# Extract features using HTK
	
		# Extract a list of files
		COMMAND_LINE="bin/HCopy -C cfg/hcopy_sph.cfg -T 1 -S data/data_htk.scp"
		echo $COMMAND_LINE
		$COMMAND_LINE
	fi
fi

