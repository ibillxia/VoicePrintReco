#!/usr/bin/perl -w


sub impSeg{
	@par = @{$_[0]};
	($NDX_DIR) = @par;

	open(IMPSEG,">${NDX_DIR}computetest_gmm_imp_seg.ndx");
	foreach $seg (@SEG){
	# write no more than 400 trials per line (limitation in XList from ALIZE)
		@tmp_imp=(@IMP);
		while($#tmp_imp > 400){
			print IMPSEG "$seg";
			for($i=0; $i<400; $i++){
				$imp=pop(@tmp_imp);
				print IMPSEG "  $imp"
			}
			print IMPSEG "\n";
		}
		if($#tmp_imp >0){
			print IMPSEG "$seg";
			foreach $imp (@tmp_imp){
				print IMPSEG "  $imp";
			}
			print IMPSEG "\n";
		}
	}
    close(IMPSEG);
}

sub impImp{
	@par = @{$_[0]};
	($NDX_DIR) = @par;
	open(IMPIMP,">${NDX_DIR}computetest_gmm_imp_imp.ndx");
	foreach $imp (@IMP){
		# write no more than 400 trials per line (limitation in XList from ALIZE)
		# the impostor segment to test is temporary removed from the list to avoid testing against itself
		@tmp_imp=();
		foreach $ti (@IMP){
			if($ti ne $imp){
				push(@tmp_imp, $ti);
			}
		}	
		while($#tmp_imp > 400){
			print IMPIMP "$imp";
			for($i=0; $i<400; $i++){
				$tar=pop(@tmp_imp);
				print IMPIMP "  $tar"
			}
			print IMPIMP "\n";
		}
		if($#tmp_imp >0){
			print IMPIMP "$imp";
			foreach $i (@tmp_imp){
				print IMPIMP "  $i";
			}
			print IMPIMP "\n";
		}
	}
    close(IMPIMP);
}

sub targetImp{
	@par = @{$_[0]};
	($NDX_DIR) = @par;
	open(TARIMP,">${NDX_DIR}computetest_gmm_target_imp.ndx");
	foreach $imp (@IMP){
		# write no more than 400 trials per line (limitation in XList from ALIZE)
		@tmp_tar=(@TAR);
		while($#tmp_tar > 400){
			print TARIMP "$imp";
			for($i=0; $i<400; $i++){
				$tar=pop(@tmp_tar);
				print TARIMP "  $tar"
			}
			print TARIMP "\n";
		}
		if($#tmp_tar >0){
			print TARIMP "$imp";
			foreach $tar (@tmp_tar){
				print TARIMP "  $tar";
			}
			print TARIMP "\n";
		}
	}
    close(TARIMP);
}





##################################################################
#	MAIN SCRIPT
##################################################################

# If there is not enough input arguments
if(scalar(@ARGV) < 3){
	print "\n *** CreateNormLists.pl\n\n";
	print "\t\t Anthony Larcher\n";
	print "\t\t 22/05/2013\n\n";
	print "\t This script is part of a tutorial for ALIZE 3.0\n";
	print "\t The author does not provide any waranty (really not)\n";
	print "\t Use at your own risk!!!\n\n";
	print  "Command line format:\n";
	print "\t ./CreateNormLists.pl <targetFile> <testFile> <impFile> <outputdir>\n\n";
	print "\t targetFile gives the lists of files to use for model training\n";
	print "\t testFile gives the trial definition for the main task\n";
	print "\t impFile is a list of impostor files to use for score normalization\n";
	print "\t outputdir is the directory to store the new NDX files\n\n";
	print "\t For more details, please refer to ALIZE documentation\n\n";
}
# Read input parameters from command line
else{
	
	$targetFile 	= $ARGV[0];
	$testFile 		= $ARGV[1];
	$impFile		= $ARGV[2];
	
	if(scalar(@ARGV) < 4){
		$ndx_dir 	= "./"	;
	}
	else{
		$ndx_dir	= $ARGV[3];
	}

	open(TARGET_NDX,$targetFile) or die "Cannot open ${targetFile}.\n";
	open(TEST_NDX,$testFile) or die "Cannot open ${testFile}.\n";
	open(IMP_CORPUS,$impFile) or die "Cannot open ${impFile}.\n";

	@TAR=();@SEG=();@IMP=();

	while($line=<TARGET_NDX>){
		chomp $line;
		$line=~s/[ \t]+/ /g;
		@part = split(/ /,$line);
		push (@TAR,$part[0]);
	}

	while($line=<IMP_CORPUS>){
		chomp $line;
		push (@IMP, $line);
	}

	while($line=<TEST_NDX>){
		chomp $line;
		$line=~s/[ \t]+/ /g;
		@part = split(/ /,$line);
		push (@SEG,$part[0]);
	}

	$tar=$#TAR+1; $imp=$#IMP+1; $seg=$#SEG+1;

	print "Create NDX files for score normalization:\n";
	print "\t Target number : $tar\n";
	print "\t Impostor number : $imp\n";
	print "\t Segment number : $seg\n";
	print  "\n";

	@param = ($ndx_dir);

	&impImp(\@param);
	&impSeg(\@param);
	&targetImp(\@param);

	close(TARGET_NDX);
	close(TEST_NDX);
	close(IMP_CORPUS);
}