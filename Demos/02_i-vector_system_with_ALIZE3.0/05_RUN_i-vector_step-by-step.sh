#!/bin/bash

####################################################################
#	Processing of I-Vector for speaker verification system   based on ALIZE 3.0
#
#	This script is distributed in the hope that it will useful but WITHOUT ANY WARRANTY.
#
#	This tutorial is designed to help getting an hand on the LIA_RAL toolkit  by introducing 
#	the main executables in context.
#
#	Executables used in this script are part of the ALIZE toolkit.
#	When using ALIZE, please use the citation provided below:
#
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

# This script run the core speaker verification engine 
# It assumes the existence of normalized feature files and label files, universal background model (UBM), 
# TotalVariability matrix (TV) and i-vectors
# If those files do not exist in ./data/prm/,  ./data/lbl/, ./gmm/, ./mat/, and ./iv/ please refer to other scripts provided in this tutorial:
#
# 	01_RUN_feature_extraction.sh
#	02a_RUN_spro_front-end.sh
#	03_RUN_i-vector_background.sh
#
# This script will compare i-vectors using Probabilistic Linear Discriminant Analysis
#
# 	Note that the output of this script (scores) are just provided for understanding of the whole process.
#	In any case they should be consider as state-of-the-art output as the system trained for this tutoriali
#	is far from state-of-the-art performance.
#	However, the configuration files provided for each executable used in this tutorial are as close as possible
#	from the state-of-the-art configuration 
#	that can be used for NIST-SRE evaluation. In this respect, those configuration files are designed 
#	to provide a good starting point.



# 1. I-vector Normalization
	echo "Normalize i-vectors"
	bin/IvNorm --config cfg/ivNorm.cfg &> log/IvNorm.log
	echo "		done, see log/IvNorm.log for details"

# 2. PLDA Training
	echo "Train Probabilistic Linear Discriminant Analysis model"
	bin/PLDA --config cfg/Plda.cfg &> log/Plda.log
	echo "		done, see log/Plda.log for details"

# 3. PLDA Testing
	echo "Compare models to test segments using PLDA native scoring"
	bin/IvTest --config cfg/ivTest_Plda.cfg &> log/IvTest_Plda.log
	echo "		done, see log/IvTest_Plda.log for details"
