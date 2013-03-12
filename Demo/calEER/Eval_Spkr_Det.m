%------------------------------
%load speaker detection output scores
load true_speaker_scores
load impostor_scores

%------------------------------
%initialize the DCF parameters
Set_DCF (10, 1, 0.01);

%------------------------------
%compute Pmiss and Pfa from experimental detection output scores
[P_miss,P_fa] = Compute_DET (true_speaker_scores, impostor_scores);

%------------------------------
%plot results

% Set tic marks
Pmiss_min = 0.01;
Pmiss_max = 0.45;
Pfa_min = 0.01;
Pfa_max = 0.45;
Set_DET_limits(Pmiss_min,Pmiss_max,Pfa_min,Pfa_max);

%call figure, plot DET-curve
figure;
Plot_DET (P_miss, P_fa,'r');
title ('Speaker Detection Performance');
hold on;

%find lowest cost point and plot
C_miss = 1;
C_fa = 1;
P_target = 0.5;
Set_DCF(C_miss,C_fa,P_target);
[DCF_opt Popt_miss Popt_fa] = Min_DCF(P_miss,P_fa);
Plot_DET (Popt_miss,Popt_fa,'ko');

