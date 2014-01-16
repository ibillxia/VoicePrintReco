package Demo;
use strict;
use warnings;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(ExactFeature EnergyDetect NormalizateFeature NormalizateFeatureForEnergy );


sub ExactFeature
{
    my ($energydetectorlist, $spherepath, $featurepath, $labelpath) = @_;
    
    ExactFeatureFromList($energydetectorlist, $spherepath, $featurepath);
    NormalizateFeatureForEnergy($energydetectorlist, $featurepath);
    EnergyDetect($energydetectorlist, $featurepath, $labelpath);
    NormalizateFeature($energydetectorlist,$featurepath, $labelpath);
}


sub ExactFeatureFromList
{
    my ($utterlistfilename, $sphfilepath, $featurepath) = @_;
    
    open(TFH, "<$utterlistfilename");

    while (<TFH>)
    {
        chomp;
        my @tmpelements = split /\\/;
        my $utterchannel = $tmpelements[$#tmpelements];
        my $uttername;
        my $channelname;
        my $channelid = 0;
        my $channelindex = index($utterchannel, "_");
        if ( $channelindex > 0 )
        {
            $uttername = substr($utterchannel, 0, $channelindex);
            $channelname = substr($utterchannel, $channelindex + 1, $channelindex + 2);
            if(lc($channelname) eq "a"){ $channelid = 1; }
            if(lc($channelname) eq "b"){ $channelid = 2; }
        }
        else
        {
            $uttername = $utterchannel;
            $channelid = 1;
        }
        
        my $sourcefilename = "$sphfilepath$uttername.pcm"; 
        my $targetfilename = "$featurepath$_.pro4";
        
        ExercuteExactFeatureCmd($channelid, $sourcefilename, $targetfilename);
        
    }
    close(TFH); 
}


sub ExercuteExactFeatureCmd
{
    my ($channelid, $sourcefilename, $targetfilename) = @_;
    
    my $cmdstr = "SproBin4.0\\sfbcep -F sphere -m -p 13 -n 24 -k 0.97 -e -D -i 300 -u 3400 -x $channelid $sourcefilename $targetfilename";#-Z -R  # MFCC Feature
    #my $cmdstr = "SproBin4.0\\slpcep -F sphere -l 30 -n 18 -p 13 -e -D -x $channelid $sourcefilename $targetfilename";#-Z -R
    
    print $cmdstr, "\n";
    system($cmdstr);
    
}


sub NormalizateFeatureForEnergy
{
    my ($inputfeaturefilename, $featurepath) = @_;

    my $normcmd = "AlizeBin2.0\\NormFeat --config conf\\NormFeatEnergy.cfg ";
    $normcmd .= " --inputFeatureFilename   $inputfeaturefilename";
    $normcmd .= " --featureFilesPath   $featurepath";
    
    print $normcmd,"\n";
    system($normcmd) == 0 or die "Normalizate Feature For Energy Error£¡";
}


sub EnergyDetect
{
    my ($inputfeaturefilename, $featurepath, $labelpath) = @_;
    
    my $detectcmd = "AlizeBin2.0\\EnergyDetector --config conf\\EnergyDetector.cfg ";
    $detectcmd .= " --featureFilesPath       $featurepath";
    $detectcmd .= " --labelFilesPath         $labelpath";
    $detectcmd .= " --inputFeatureFilename   $inputfeaturefilename";
    
    print $detectcmd,"\n";
    system($detectcmd) == 0 or die "Energy Detect Error£¡";
}


sub NormalizateFeature
{
    my ($inputfeaturefilename,$featurefilespath, $labelpath) = @_;
    
    my $normcmd = "AlizeBin2.0\\NormFeat --config conf\\NormFeat.cfg ";
    $normcmd .= " --inputFeatureFilename   $inputfeaturefilename";
    $normcmd .= " --featureFilesPath       $featurefilespath";
    $normcmd .= " --labelFilesPath         $labelpath";
    
    print $normcmd,"\n";
    system($normcmd) == 0 or die "Normalization Feature Error£¡";
}


1;
