use strict;

my $f1 = "ndx\\sre05_male_test_result.txt";
my $f12 = "ndx\\sre05_male_test_key.txt";
my $f2 = "ndx\\sre05_male_test_result_true";
my $f22 = "ndx\\sre05_male_test_result_imposter";

my $FH;
my $FH2;
open(IN,"<$f1");
open(IN2,"<$f12");
open($FH,">$f2");
open($FH2,">$f22");

my %HASH;
while(<IN2>){
    chomp;
    my @elems = split / /;
    if($elems[2] eq 'target'){
        $HASH{$elems[0].$elems[1]} = 1;
    }else{
        $HASH{$elems[0].$elems[1]} = 0;
    }
}

while(<IN>){
    chomp;
    my @elems = split / /;
    if($HASH{$elems[1].$elems[3]}){
        print $FH $elems[4],"\n";
    }else{
        print $FH2 $elems[4],"\n";
    }
}

close(IN);
close(IN2);
close($FH);
close($FH2);
