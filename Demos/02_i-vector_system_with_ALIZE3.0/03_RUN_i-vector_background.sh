#!/bin/bash

####################################################################
#	Background training for I-Vector speaker verification system   based on ALIZE 3.0
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
# It assumes the existence of normalized feature files and label files
# If those files do not exist in ./data/prm/ and ./data/lbl/, please refer to other scripts provided in this tutorial:
#
# 	01_RUN_feature_extraction.sh
#	02a_RUN_spro_front-end.sh
#
# This script will train an i-vector extractor including Universal Background Model (UBM), TotalVariability Matrix,
# and extract i-vectors
#
# 	Note that the output of this script (i-vectors) are just provided for understanding of the whole process.
#	In any case they should be consider as state-of-the-art output as the system trained for this tutoriali
#	is far from state-of-the-art performance.
#	However, the configuration files provided for each executable used in this tutorial are as close as possible
#	from the state-of-the-art configuration 
#	that can be used for NIST-SRE evaluation. In this respect, those configuration files are designed 
#	to provide a good starting point.
#
#	To train a state-of-the-art i-vector systems, one should modify the number of distributions of the UBm, the rank 
#	of the TotalVariability matrix and increase the quantity of data used for each of the steps.
#


# 1. UBM training
	echo "Train Universal Background Model by EM algorithm"
	bin/TrainWorld --config cfg/TrainWorld.cfg &> log/TrainWorld.log
	echo "		done, see log/TrainWorld.log for details"

# 2. Total Variability matrix Estimation
	echo "Train TotalVariability matrix"
	bin/TotalVariability --config cfg/TotalVariability_fast.cfg &> log/TotalVariability.log
	echo "		done, see log/TotalVariability.log for details"

# 3. I-vector extraction
	echo "Extract i-vectors"
	bin/IvExtractor --config cfg/ivExtractor_fast.cfg &> log/IvExtractor.log
	echo "		done, see log/IvExtractor.log for details"

