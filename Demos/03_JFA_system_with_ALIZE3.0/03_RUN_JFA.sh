#!/bin/bash

####################################################################
#	Joint Factor Analysis speaker verification system   based on ALIZE 3.0
#
#	This script is distributed in the hope that it will useful but WITHOUT ANY WARRANTY.
#
#	This tutorial is designed to help getting an hand on the LIA_RAL toolkit  by introducting 
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
#		24/09/2013
####################################################################

# This script run the core speaker verification engine assuming the existence of normalized feature files and label files
# If those files do not exist in ./data/prm/ and ./data/lbl/, please refer to other scripts provided in this tutorial:
#
# 	01_RUN_feature_extraction.sh
#	02a_RUN_spro_front-end.sh
#
# This script will train a Universal Background Model (UBM), train a Joint Factor Analysis model and adapt speaker models.
# Those models will be compared to test segments and the score are normalized in the last part of the process.
#
# 	Note that the output of this script (scores) are just provided for understanding of the whole process.
#	In any case they should be consider as state-of-the-art output as the system trained for this tutorial is far from state-of-the-art 
#	performance.
#	However, the configuration files provided for each executable used in this tutorial are as close as possible from the state-of-the-art configuration 
#	that can be used for NIST-SRE evaluation. In this respect, those configuration files are designed to provide a good starting point.


# 1. UBM training
	echo "Train Universal Background Model by EM algorithm"
	bin/TrainWorld --config cfg/TrainWorld.cfg &> log/TrainWorld.log
	echo "		done, see log/TrainWorld.log for details"


# 2. EigenVoice (compute the statistics and train the Matrix at once)
	echo "Estimate EigenVoice matrix"
	bin/EigenVoice --config cfg/EigenVoice.cfg &> log/EigenVoice.log
	echo "		done, see log/EigenVoice.log for details"

# 3. EigenChannel (first compute the statistics and then train the Matrix)
	echo "Compute sufficient statistics for EigenChannel training"
	bin/ComputeJFAStats --config cfg/ComputeJFAStatsEigenChannel.cfg &> log/Stats_EigenChannel.log
	echo "		done, see log/Stats_EigenChannel.log for details"

	echo "Compute sufficient statistics for EigenChannel training"
	bin/EigenChannel --config cfg/EigenChannel.cfg &> log/EigenChannel.log
	echo "		done, see log/EigenChannel.log for details"

# 4 EstimateDMatrix (compute the statistics and train the Matrix at once)
	echo "Compute sufficient statistics for DMatrix training"
	bin/EstimateDMatrix --config cfg/EstimateDMatrix.cfg &> log/DMatrix.log
	echo "		done, see log/Stats_DMatrix.log for details"

# 5. Speaker GMM model adaptation
	echo "Train Speaker dependent GMMs"
	bin/TrainTarget --config cfg/TrainTarget.cfg &> log/TrainTarget.log
	echo "		done, see log/TrainTarget.cfg for details"

# 6. Speaker model comparison
	echo "Compute Likelihood"
	bin/ComputeTest --config cfg/ComputeTest.cfg &> log/ComputeTest.log
	echo "		done, see log/ComputeTest.log"

