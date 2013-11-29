#!/bin/bash

####################################################################
#	RUN Spro Front-End
#
#	This script is distributed in the hope that it will useful but WITHOUT ANY WARRANTY.
#
#	This tutorial is designed to help getting an hand on the LIA_RAL toolkit  by introducting 
#	the main executables in context.
#
#	Executables used in this script are part of the ALIZE toolkit.
#	When using ALIZE, please use the citation provided below:
#
#	A. Larcher, J.-F. Bonastre, B. Fauve, K.A. Lee, C. Levy, H. Li, J.S.D. Mason, J.-Y Parfait, 
#	"ALIZE 3.0 - Open Source Toolkit for State-of-the-Art Speaker Recognition," 
#	in Annual Conference of the International Speech Communication Association (Interspeech), 2013
#
#		by Anthony Larcher
#		alarcher - at- i2r.a-star.edu.sg
#
#		07/06/2013
####################################################################
	 
# This script process the acoustic feature files extracted from the SPHERE files by using Spro
# If those features have not been generated (in directory ./data/prm/) you need to run 
#
#	01_RUN_feature_extraction.sh
#
# or decompress the archive SPRO_features.tar.gz located in ./data/ into ./data/prm/
#
# After this script is completed, you will get normalized feature files in ./data/prm/ (extension *.norm.prm)
# as well as label files indicating the time label of speech frames in ./data/lbl/
	 
        echo "Normalise energy : `date` "
	CMD_NORM_E="bin/NormFeat --config cfg/NormFeat_energy_SPro.cfg --inputFeatureFilename data/data.lst --featureFilesPath  data/prm/"
	echo $CMD_NORM_E
	$CMD_NORM_E
        echo "End normalise energy : `date`\n "

        echo "Energy Detector : `date` "
        CMD_ENERGY="bin/EnergyDetector  --config cfg/EnergyDetector_SPro.cfg --inputFeatureFilename data/data.lst --featureFilesPath  data/prm/  --labelFilesPath  data/lbl/"
	echo $CMD_ENERGY
	$CMD_ENERGY
        echo "End energy detector : `date`\n "

        echo "Normalise Features : `date`"
        CMD_NORM="bin/NormFeat --config cfg/NormFeat_SPro.cfg --inputFeatureFilename data/data.lst --featureFilesPath data/prm/   --labelFilesPath  data/lbl/"
	echo $CMD_NORM
	$CMD_NORM
        echo "End Normalise Features : `date`"

