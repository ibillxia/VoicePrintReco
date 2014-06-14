#!/usr/bin/perl -w 

use strict;
use Data::Dumper;
use FileHandle;

### TEST
my $Usage = "Usage: mux.pl [ -h | -t | -g RawGeometry ] -o outFile -i inFile inFile ...\n".
    "       -t  ->  Run the self tests\n".
    "       -h  ->  Print this  help\n".
    "       -o FILE ->  Name the output file\n".
    "       -i FILES -> Name the input files\n".
    "       -g RawGeometry -> String idenfifying the geometry of the raw files used as input.\n".
    "                         A raw file is output and the entire input files are mux'.\n".
    "                         The string's format is:\n".
    "                            SampleNBytes\n".
    "       mux.pl";

my $RunTest = undef;
my $GeometryString = undef;
my $Help;
my $newFile = undef;
my @InFiles = ();
use Getopt::Long;
my $result = GetOptions("g=s" => \$GeometryString,
			"h" => \$Help,
			"t" => \$RunTest,
			"o=s" => \$newFile,
			"i=s@" => \@InFiles);
die "Aborting:\n$Usage\n" if (!$result);

print "OutFile: $newFile\n";
print "InFiles: ".join(" ",@InFiles)."\n";


die $Usage if (defined($Help));
if (defined($RunTest)){
    $ENV{PATH} = "../bin:".$ENV{PATH};
    system "./mux2.pl -o foo.sph -i ../../lib/data/ex5_c1_p.wav -i ../../lib/data/ex5_c2_p.wav";
    system "../bin/w_diff -v foo.sph ../../lib/data/ex5_p.wav";
    system "./mux2.pl -o foo.sph -i ../../lib/data/ex5_c1_p.wav -i ../../lib/data/ex5_c2_p.wav";
    system "./mux2.pl -o foo.sph -i ../../lib/data/ex5_c1.wav -i ../../lib/data/ex5_c2.wav";
    system "w_diff -v foo.sph ../../lib/data/ex5.wav";
    system "./mux2.pl -o foo.sph -i ../../lib/data/ex5_c1_p.wav -i ../../lib/data/ex5_c2_p.wav -i ../../lib/data/ex5_c1_p.wav -i ../../lib/data/ex5_c2_p.wav";

    system "../bin/h_strip ../../lib/data/ex5_c1_p.wav x1.raw";
    system "../bin/h_strip ../../lib/data/ex5_c2_p.wav x2.raw";
    system "../bin/h_strip ../../lib/data/ex5_p.wav x3.raw";
    system "./mux2.pl -g 2 -o foo.raw -i x1.raw -i x2.raw";
    system "cmp foo.raw x3.raw";
    system "rm -f foo.sph x?.raw";
    exit 0;
}
die "Error: -o req'd\n$Usage\n" if (!$newFile);

## Get the files to merge

if (defined($GeometryString)){
    die "Error: Malformed Geometry String" if ($GeometryString !~ /^(\d+)$/);
    my ($sampleNBytes) = $1;

    ### Open the files and get the byte sizes
    my @files;
    my $sampleCount = undef;
    foreach $_(@InFiles){
	print "MuxData file $_\n";
	push @files , new FileHandle;
	$files[$#files]->open($_) || die "Failed to open $_ for read";
	my @a = stat($files[$#files]);
	die "Error: file $_ does not have a integer number of $sampleNBytes-byte samples" if ($a[7] % $sampleNBytes != 0);
	$sampleCount = $a[7] / $sampleNBytes if (!defined($sampleCount));
	$sampleCount = $a[7] / $sampleNBytes if ($a[7] / $sampleNBytes < $sampleCount);
    }
    open FOO, ">$newFile" || die "Failed to open '$newFile' for write";
    binmode FOO;
    my $buff;

    ### Read one sample at a time and multiplex
    foreach (my $i=0; $i<$sampleCount; $i++){
	foreach my $sp(@files){
	    die "Error: SampleNBytes not consistent" unless ($sp->read($buff, $sampleNBytes) == $sampleNBytes);
	    print FOO $buff;
	}
    }
    close FOO;
} else {
    my @SPs;
    foreach $_(@InFiles){
	print "MuxData file $_\n";
	push @SPs, openSPHERE($_);
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
    print "Sample trunc $sampleCount\n";

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
}
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
