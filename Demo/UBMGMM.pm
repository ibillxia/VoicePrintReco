package Demo;
use strict;
use base 'Exporter';
our @EXPORT_OK = qw(TrainUBM TrainTarget ComputeTest );


sub TrainUBM
{
    my ($lambda, $inputfeaturefilename, $outputwordfilename) = @_;
    
    my $trainubm = "AlizeBin2.0\\TrainWorld --config conf\\TrainWorld.cfg ";
    $trainubm .= " --inputFeatureFilename     $inputfeaturefilename ";
    $trainubm .= " --nbTrainIt                25";
    $trainubm .= " --baggedFrameProbabilityInit   0.5";
    $trainubm .= " --baggedFrameProbability   0.5";
    $trainubm .= " --outputWorldFilename      $outputwordfilename";
    $trainubm .= " --fileInit                 false";
    $trainubm .= " --lambda                   $lambda";
    
    print $trainubm,"\n";
    system($trainubm) == 0 or die "TrainUBM Error£¡";
}


sub TrainTarget
{
    my ($targetidlist,$worldfilename,$FeatPath,$MixturePath) = @_;
    
    my $traintarget = "AlizeBin2.0\\TrainTarget --config conf\\TrainTarget.cfg ";
    $traintarget .= " --targetIdList             $targetidlist";
    $traintarget .= " --mixtureFilesPath         $MixturePath";
    $traintarget .= " --featureFilesPath         $FeatPath";
    $traintarget .= " --inputWorldFilename       $worldfilename";
    
    print $traintarget, "\n";
    system($traintarget) == 0 or die "TrainTarget Error£¡";
}


sub ComputeTest
{
    my ($testndxFilename, $testmodelPath, $testFeaturePath, $testLabelPath, $worldFilename, $resultfile) = @_;
    
    my $testtarget = "AlizeBin2.0\\ComputeTest --config conf\\ComputeTest.cfg ";
    $testtarget .= " --ndxFilename              $testndxFilename";
    $testtarget .= " --mixtureFilesPath         $testmodelPath";
    $testtarget .= " --featureFilesPath         $testFeaturePath";
    $testtarget .= " --labelFilesPath           $testLabelPath";
    $testtarget .= " --inputWorldFilename       $worldFilename";
    $testtarget .= " --outputFilename           $resultfile";
    
    print $testtarget,"\n";
    system($testtarget) == 0 or die "ComputeTest Error£¡";
}

1;