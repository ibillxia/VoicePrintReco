#! /usr/bin/perl -w

sub impSeg{

	@par = @{$_[0]};
	($NDX_DIR) = @par;

	open(IMPSEG,">${NDX_DIR}imp_seg.ndx");
	foreach $seg (@SEG){
#on gère le cas où il y aurait plus de 400 modèles à tester
		@tmp_imp=(@IMP);
		#tant qu'on a plus de 400 modèles à écrire on découpe en plusieurs lignes
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
	
    open(IMPIMP,">${NDX_DIR}imp_imp.ndx");
	foreach $imp (@IMP){
		#on gère le cas où il y aurait plus de 400 modèles à tester
		#au passage on supprime le segment à tester de la liste temporaire des imposteurs
		@tmp_imp=();
		foreach $ti (@IMP){
			if($ti ne $imp){
				push(@tmp_imp, $ti);
			}
		}	
		#tant qu'on a plus de 400 modèles à écrire on découpe en plusieurs lignes
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
	($NDX_DIR,$COND_TRAIN,$COND_TEST,$YEAR) = @par;

    open(TARIMP,">${NDX_DIR}target_imp.ndx");
	foreach $imp (@IMP){
		#on gère le cas où il y aurait plus de 400 modèles à tester
		@tmp_tar=(@TAR);
		#tant qu'on a plus de 400 modèles à écrire on découpe en plusieurs lignes
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


