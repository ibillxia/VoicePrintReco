% DET evaluation Graphics
% Version 1.2 28-JUL-2000
% George Doddington
%
% DET plots (Detection Error Trade-off)
%	DET_usage	- Example program of below functions
%	Compute_DET	- Computes miss/false_alarm probabilities
%	Plot_DET	- Plots a DET
%	Set_DCF		- Initializes the detection cost fundtion
%	Min_DCF		- Finds minimum detection cost function
%	Set_DET_limits	- initializes the min/max plotting limits
%	ppndf		- Warps cumulative probability to normal deviate
%
% The Detection Error Trade-off (DET) Plotting Package
%
% The DET plotting package provide functions to support plotting the results
% of detection experiments in an intuitively meaningful way. Detection error
% probabilities (miss and false alarm probabilities) are plotted on a nonlinear
% scale. This scale transforms the error probability by mapping it to is
% corresponding Gaussian deviate. Thus DET curves are straight lines when
% the underlying distributions are Gaussian. This makes DET plots more
% intuitive and visually meaningful.
%
% The DET plotting package comprises 7 matlab functions:
%
%  There are two primary user-callable functions:
%    Compute_DET -- computes the DET from detection system output results.
%    Plot_DET    -- plots the DET produced by Compute_DET.
%
%  There are two auxiliary functions which find a DET's minimum cost point:
%    Set_DCF -- initializes the detection cost unction parameters.
%    Min_DCF -- finds the detection error trade-off which gives minimum cost.
%
%  There are three supporting functions (which the user needn't call):
%    Set_DET_limits -- changes the DET plotting limits.
%    ppndf          -- transforms probabilities to corresponding Gaussian deviates.
%
% Further documentation is available for the various DET functions by using
% the matlab help command.
%
% Execute this script to see 5 examples using the DET pakage.

echo on;
%	The Detection Error Trade-off (DET) Plotting Package
%
% Here are five examples which serve to illustraite how to use the DET plotting
% package. In its simplest form, it is mearly two lines of code, namely
% "Compute_DET" to compute the DET and "Plot_DET' to plot it.
% Here is a simple example:

   %-----------------------
   % Create simulated detection output scores

   Ntrials_True = 1000;
   True_scores = randn(Ntrials_True,1);

   Ntrials_False = 1000;
      mean_False = -3;
      stdv_False = 1.5;
   False_scores = stdv_False * randn(Ntrials_False,1) + mean_False;

   %-----------------------
   % Compute Pmiss and Pfa from experimental detection output scores

   [P_miss,P_fa] = Compute_DET(True_scores,False_scores);

   %-----------------------
   % Plot the detection error trade-off

   figure;
   Plot_DET (P_miss,P_fa,'r');
   title('A DET plot');

echo off;
clear False_scores P_miss stdv_False Ntrials_False True_scores ...
      Ntrials_True ans P_fa mean_False;
clear global DET_limits;

input('Hit <cr> for next example');
echo on;

% In actual usage, you will likely want to plot the DETs for multiple
% experiments on the same plot. You may do this with Plot_DET just
% as you would with the standard matlab plot function. Here is an
% example of how to plot multiple DETs on the same plot:

   %-----------------------
   % Initialize the plot
   figure;
   title('A DET plot contrasting multiple DETs');
   hold on;

   %-----------------------
   % Create simulated detection output scores
   N_plots = 2;
   Ntrials_True = [1000 1000];
   Ntrials_False = [1000 1000];
      mean_False = [-2 -3];
      stdv_False = [0.75 1.5];
   plot_code = ['r' 'g'];

   %-----------------------
   % Plot the detection error trade-off
   for n=1:N_plots

      True_scores = randn(Ntrials_True(n),1);
      False_scores = stdv_False(n) * randn(Ntrials_False(n),1) + mean_False(n);

      [P_miss,P_fa] = Compute_DET(True_scores,False_scores);
      Plot_DET (P_miss,P_fa,plot_code(n));
   end

echo off;
clear False_scores P_miss plot_code N_plots True_scores stdv_False ...
      Ntrials_False ans Ntrials_True mean_False P_fa n                   

clear global DET_limits;

input('Hit <cr> for next example');
echo on;

% It is also possible to search through a DET curve to find the point of
% minimum cost, according to a Detection Cost Function (DCF). This is
% supported by two matlab function "Set_DCF" and "Min_DCF". Set_DCF
% initializes the DCF parameters, and Min_DCF searches through the DET
% curve for the minimum cost point. This point may then be plotted along
% with the DET. Here is an example:

   %-----------------------
   % Create simulated detection output scores

   Ntrials_True = 1000;
   True_scores = randn(Ntrials_True,1);

   Ntrials_False = 1000;
      mean_False = -3;
      stdv_False = 1;
   False_scores = stdv_False * randn(Ntrials_False,1) + mean_False;

   %-----------------------
   % Compute Pmiss and Pfa from experimental detection output scores

   [P_miss,P_fa] = Compute_DET(True_scores,False_scores);

   %-----------------------
   % Plot the detection error trade-off

   figure;
   Plot_DET (P_miss,P_fa,'r');
   title('A DET plot showing the optimum (minimum) detection cost point');
   hold on;

   C_miss = 1;
   C_fa = 1;
   P_target = 0.5;

   Set_DCF(C_miss,C_fa,P_target);
   [DCF_opt Popt_miss Popt_fa] = Min_DCF(P_miss,P_fa);
   Plot_DET (Popt_miss,Popt_fa,'ko');

echo off;

clear C_fa P_fa ans C_miss P_miss mean_False DCF_opt P_target stdv_False ...
      False_scores Popt_fa Ntrials_False Popt_miss Ntrials_True True_scores
clear global DET_limits;

input('Hit <cr> for next example');
echo on;

% Although it is desirable to keep the same DET plotting format, in terms of
% range of probabilities displayed, there may be times when it becomes 
% necessary to change this range. This may be done using "Set_DE_limits".
% Here is an example:

   %-----------------------
   % Create simulated detection output scores

   Ntrials_True = 1000;
   True_scores = randn(Ntrials_True,1);

   Ntrials_False = 1000;
      mean_False = -3;
      stdv_False = 1.5;
   False_scores = stdv_False * randn(Ntrials_False,1) + mean_False;

   %-----------------------
   % Compute Pmiss and Pfa from experimental detection output scores

   [P_miss,P_fa] = Compute_DET(True_scores,False_scores);

   %-----------------------
   % Plot the detection error trade-off

   Pmiss_min = 0.01;
   Pmiss_max = 0.8;
   Pfa_min = 0.003;
   Pfa_max = 0.3;
   Set_DET_limits(Pmiss_min,Pmiss_max,Pfa_min,Pfa_max);

   figure;
   Plot_DET (P_miss,P_fa,'r');
   title('A DET plot which uses different detection error limits');

echo off;

clear C_fa P_fa ans C_miss P_miss mean_False DCF_opt P_target stdv_False ...
      False_scores Popt_fa Ntrials_False Popt_miss Ntrials_True True_scores ...
      Pmiss_min Pmiss_max Pfa_min Pfa_max
clear global DET_limits;

input('Hit <cr> for next example');
echo on;

% Sometimes it is desirable to have a thicker line plotted. To adjust
% the line thickness, just add an argument to the Plot_DCF. Default
% is 0.5. A value between 2 and 5 will give a thick line.
% Here is an example:

   %-----------------------
   % Create simulated detection output scores

   Ntrials_True = 1000;
   True_scores = randn(Ntrials_True,1);

   Ntrials_False = 1000;
      mean_False = -3;
      stdv_False = 1;
   False_scores = stdv_False * randn(Ntrials_False,1) + mean_False;

   %-----------------------
   % Compute Pmiss and Pfa from experimental detection output scores

   [P_miss,P_fa] = Compute_DET(True_scores,False_scores);

   %-----------------------
   % Plot the detection error trade-off

   thickness = 2;
   figure;
   Plot_DET (P_miss,P_fa,'r',thickness);
   title('A DET plot with a thick line');
   hold on;

   C_miss = 1;
   C_fa = 1;
   P_target = 0.5;

   Set_DCF(C_miss,C_fa,P_target);
   [DCF_opt Popt_miss Popt_fa] = Min_DCF(P_miss,P_fa);
   Plot_DET (Popt_miss,Popt_fa,'ko',thickness);

% The underlying function (called "thick") which makes the line thick can also be 
% used with the standard matlab plot function. 

% Example usage: thick(2,plot([1:5],[1,0,1,0,1],'b'))

echo off;
clear C_fa P_fa ans C_miss P_miss mean_False DCF_opt P_target stdv_False ...
      False_scores Popt_fa thickness Ntrials_False Popt_miss Ntrials_True True_scores         
clear global DET_limits;

