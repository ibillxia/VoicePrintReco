function Set_DCF (Cmiss, Cfa, Ptrue)
%function Set_DCF (Cmiss, Cfa, Ptrue) initializes the detection
%cost function (DCF) parameters.  The detection cost function is
%defined as:
%
%     DCF = Cmiss * Pmiss * Ptrue  +  Cfa * Pfa * Pfalse
%
%  DCF is a function of Pmiss and Pfa, the miss and false alarm
%  probabilities.  The  DCF parameters are:
%
%     Cmiss, the cost of a miss,
%     Cfa, the cost of a false alarm,
%     Ptrue, the a priori probability of the target, and
%     Pfalse, = 1 - Ptrue.
%
%  See DET_usage for an example of how to use Set_DCF.

global DCF_parameters
DCF_parameters = [Cmiss, Cfa, Ptrue, 1-Ptrue];

