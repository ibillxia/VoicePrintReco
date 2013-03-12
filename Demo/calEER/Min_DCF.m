function [min_cost, Pmiss_opt, Pfa_opt] = Min_DCF(Pmiss, Pfa)
%function [min_cost, Pmiss_opt, Pfa_opt] = Min_DCF(Pmiss, Pfa)
%
% Min_DCF finds and returns the minimum value of the detection
% cost function for a given detection error trade-off curve.
%
% Pmiss and Pfa are the correcponding miss and false alarm
% trade-off probabilities.
%
%
% See DET_usage for an example of how to use Min_DCF.

global DCF_parameters

if isempty(DCF_parameters)
        error ('call Set_DCF to define DCF parameters before calling Min_DCF');
end

Cmiss = DCF_parameters(1);
Cfa = DCF_parameters(2);
Ptrue = DCF_parameters(3);
Pfalse = DCF_parameters(4);

npts = max(size(Pmiss));
if npts ~= max(size(Pfa))
        error ('vector size of Pmiss and Pfa not equal in call to Plot_DET');
end

%-------------------------
%Find DCF_best:

DCF_vector = Cmiss * Pmiss * Ptrue  + Cfa * Pfa * Pfalse;
[min_cost min_ptr] = min (DCF_vector);
Pmiss_opt = Pmiss(min_ptr(1)) ;
Pfa_opt = Pfa(min_ptr(1));

