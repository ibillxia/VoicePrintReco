package Demo;

#!/usr/bin/perl -w

use strict;
use File::Copy;

use FeatureExtraction;
use UBMGMM;


my $featurepath = "expdata\\feat\\";
my $labelpath = "expdata\\label\\";
my $mixturepath = "expdata\\mixturepath\\";


#----------------------------------------------------------------------#
#                  Universal Background Model(UBM)                     #
#----------------------------------------------------------------------#

ExactFeature("ndx\\sre04_1side_male_train.lst",
             "corpus\\sre04\\male\\", $featurepath, $labelpath);

TrainUBM(0, "ndx\\sre04_1side_male_train.lst", "UBM_SRE04_MALE");


#----------------------------------------------------------------------#
#                Speaker Recognition£ºTraining Session                 #
#----------------------------------------------------------------------#

ExactFeature("ndx\\sre05_male_trian.lst",
             "corpus\\sre05\\male\\", $featurepath, $labelpath);

TrainTarget("ndx\\sre05_male_trian.trn", "UBM_SRE04_MALE",
            $featurepath,$mixturepath);


#----------------------------------------------------------------------#
#                Speaker Recognition£ºTesting Session                  #
#----------------------------------------------------------------------#

ExactFeature("ndx\\sre05_male_test.lst",
             "corpus\\sre05\\male\\", $featurepath, $labelpath);

TrainTarget("ndx\\sre05_male_test.trn", "UBM_SRE04_MALE",
            $featurepath,$mixturepath);

ComputeTest("ndx\\sre05_male_test.ndx", 
            $mixturepath, $featurepath, $labelpath, 
            "UBM_SRE04_MALE", "ndx\\sre05_male_test_result.txt");

