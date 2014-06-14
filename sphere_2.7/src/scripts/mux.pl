#!/usr/bin/perl -w 

use strict;
use Data::Dumper;
use FileHandle;

### TEST
die "Usage: mux.pl outFile inFile inFile ...\n".
    "       mux.pl -t\n".
    "       mux.pl" if (! defined($ARGV[0]) || $ARGV[0] eq "-h");
if ($ARGV[0] eq "-t"){
    $ENV{PATH} = "../bin:".$ENV{PATH};
    system "./mux.pl foo.sph ../../lib/data/ex5_c1_p.wav ../../lib/data/ex5_c2_p.wav";
    system "w_diff -v foo.sph ../../lib/data/ex5_p.wav";
    system "./mux.pl foo.sph ../../lib/data/ex5_c1.wav ../../lib/data/ex5_c2.wav";
    system "w_diff -v foo.sph ../../lib/data/ex5.wav";
    system "./mux.pl foo.sph ../../lib/data/ex5_c1_p.wav ../../lib/data/ex5_c2_p.wav ../../lib/data/ex5_c1_p.wav ../../lib/data/ex5_c2_p.wav";
    system "rm -f foo.sph";
    exit;
}

###Get the file names
my $newFile = shift @ARGV;
print "NewFile is $newFile\n";
### Get the files to merge
my @SPs = ();
while (@ARGV > 0){
    print "MuxData file $ARGV[0]\n";
    push @SPs, openSPHERE(shift @ARGV);
}


### Check the headers to make sure the signal geometry is identical
foreach my $hdChk("sample_rate", "sample_n_bytes", "sample_coding", "sample_byte_format", "channel_count"){
    my $value = $SPs[0]->{HEADER}{$hdChk}{VALUE};
    foreach my $sp(@SPs){
	die "Header filed '$hdChk' doesn't match '$value' and '$sp->{HEADER}{$hdChk}{VALUE}'" if ($value ne $sp->{HEADER}{$hdChk}{VALUE});
    }
}
die "Error: only single channel files chan be muxed" if ($SPs[0]->{HEADER}{"channel_count"}{VALUE} != 1);

### Find the minimum sample count and wrote that many samples
my $sampleCount = $SPs[0]->{HEADER}{"sample_count"}{VALUE};
foreach my $sp(@SPs){
    $sampleCount = $sp->{HEADER}{"sample_count"}{VALUE} if ($sampleCount > $sp->{HEADER}{"sample_count"}{VALUE});
}
### Save off the sample N bytes
my $sampleNBytes = $SPs[0]->{HEADER}{"sample_n_bytes"}{VALUE};

### Build values for the headers
my $newHeadValues = "";
foreach my $hdChk("sample_rate", "sample_n_bytes", "sample_coding", "sample_byte_format"){
    $newHeadValues .= " -".uc($SPs[0]->{HEADER}{$hdChk}{TYPE})." $hdChk=$SPs[0]->{HEADER}{$hdChk}{VALUE}";
}
$newHeadValues .= " -I channel_count=".scalar(@SPs);
$newHeadValues .= " -I sample_count=".$sampleCount;

### open the new file
open FOO, "| h_add - $newFile" || die "Failed to open '$newFile' for write";
binmode FOO;
my $buff;
### Read one sample at a time and multiplex
foreach (my $i=0; $i<$sampleCount; $i++){
    foreach my $sp(@SPs){
        die "Error: SampleNBytes not consistent" unless ($sp->{AUDIOFH}->read($buff, $sampleNBytes) == $sampleNBytes);
        print FOO $buff;
    }
}
close FOO;

### Set the header fields
system "h_edit -c $newHeadValues $newFile";
exit 0;


#########################################################################################################

sub openSPHERE{
    my ($file) = @_;
    my %sp = ();
    open (HEAD, "h_read -t $file |") || die "Failed to open sphere file '$file'";
    while (<HEAD>){
	chomp;
	my ($field, $type, $value) = split(/\s/,$_,3);
	$sp{HEADER}{$field} = {TYPE => $type, VALUE => $value};
    }
    close (HEAD);
    
    $sp{HEADER}{"sample_coding"} = "pcm" if (! defined($sp{HEADER}{"sample_coding"}));
    my $isCompressed = ($sp{HEADER}{"sample_coding"}{VALUE} =~ s/,embedded-shorten.*$//);
    $sp{AUDIOFH} = new FileHandle;
    if ($isCompressed){
	$sp{AUDIOFH}->open("w_decode $file - | h_strip - - |") || die "Failed to open $file decompressed";
    } else {
	$sp{AUDIOFH}->open("h_strip $file - |") || die "Failed to open $file as audio";
    }
    \%sp;
}
