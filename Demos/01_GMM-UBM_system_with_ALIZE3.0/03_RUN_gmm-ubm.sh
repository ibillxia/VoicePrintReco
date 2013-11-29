#!/bin/bash

####################################################################
#	GMM-UBM speaker verification system   based on ALIZE 3.0
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
#		07/06/2013
####################################################################

# This script run the core speaker verification engine assuming the existence of normalized feature files and label files
# If those files do not exist in ./data/prm/ and ./data/lbl/, please refer to other scripts provided in this tutorial:
#
# 	01_RUN_feature_extraction.sh
#	02a_RUN_spro_front-end.sh
#
# This script will train a Universl Background Model (UBM) and create speaker specific models (GMMs)
# Those models will be compared to test segments and the score are normalized in the last part of the process.
#
# 	Note that the output of this script (scores after normalization) are just provided for understanding of the whole process.
#	In any case they should be consider as state-of-the-art output as the system trained for this tutorial is far from state-of-the-art 
#	performance.
#	However, the configuration files provided for each executable used in this tutorial are as close as possible from the state-of-the-art configuration 
#	that can be used for NIST-SRE evaluation. In this respect, those configuration files are designed to provide a good starting point.


# 1. UBM training
	echo "Train Universal Background Model by EM algorithm"
	bin/TrainWorld --config cfg/TrainWorld.cfg &> log/TrainWorld.log
	echo "		done, see log/TrainWorld.log for details"

# 2. Speaker GMM model adaptation
	echo "Train Speaker dependent GMMs"
	bin/TrainTarget --config cfg/TrainTarget.cfg &> log/TrainTarget.cfg
	echo "		done, see log/TrainTarget.cfg for details"

# 3. Speaker model comparison
	echo "Compute Likelihood"
	bin/ComputeTest --config cfg/ComputeTest_GMM.cfg &> log/ComputeTest.cfg
	echo "		done, see log/ComputeTest.cfg"

# 4. Impostor GMM model adaptation for score normalization (t-norm)
	echo "Train Impostor dependent GMMs for T-norm"
	bin/TrainTarget --config cfg/TrainImp.cfg &> log/TrainImp.log
	echo "		done, see log/ComputeTest_tnorm.log"

# 5. Compute test for score normalization (z-norm)
	echo "Compute Z-norm scores"
	bin/ComputeTest --config cfg/ComputeTestZNorm.cfg &> log/ComputeTest_znorm.log
	echo "		done, see log/ComputeTest_znorm.log"

# 6. Compute test for score normalization (t-norm)
	echo "Compute T-norm scores"
	bin/ComputeTest --config cfg/ComputeTestTNorm.cfg &> log/ComputeTest_tnorm.log
	echo "		done, see log/ComputeTest_tnorm.log"

# 7. Compute test for scores normalization (zt-norm)
	echo "Compute ZT-norm scores"
	bin/ComputeTest --config cfg/ComputeTestZTnorm.cfg &> log/ComputeTestztnorm.log
	echo "		done, see log/ComputeTest_ztnorm.log"

# 8. Score Normalization: Z-norm
	echo "Apply Z-norm"
	bin/ComputeNorm --config cfg/ComputeNorm_znorm.cfg &> log/ComputeNorm_znorm.log
	echo "		done, see log/ComputeNorm_znorm.log"

# 9. Score Normalization: T-norm
	echo "Apply T-norm"
	bin/ComputeNorm --config cfg/ComputeNorm_tnorm.cfg &> log/ComputeNorm_tnorm.log
	echo "		done, see log/ComputeNorm_tnorm.log"

# 10. Score Normalization: ZT-norm
#
# BEWARE: ZT-norm means t-norm is applied first, then Z-norm follows
#	note that in this configuration, scores after t-norm are also provided as output
#	in a symetric manner, TZ-norm return scores after Z-norm and TZ-norm
#
	echo "Apply ZT-norm"
	bin/ComputeNorm --config cfg/ComputeNorm_ztnorm.cfg &> log/ComputeNorm_ztnorm.log
	echo "		done, see log/ComputeNorm_ztnorm.log"

	
	
	
	
	
	
	
	
	
	
