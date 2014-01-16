function Set_DET_limits(Pmiss_min, Pmiss_max, Pfa_min, Pfa_max)
% function Set_DET_limits(Pmiss_min, Pmiss_max, Pfa_min, Pfa_max)
%
%  Set_DET_limits initializes the min.max plotting limits for P_min and P_fa.
%
%  See DET_usage for an example of how to use Set_DET_limits.

Pmiss_min_default = 0.0005+eps;
Pmiss_max_default = 0.5-eps;
Pfa_min_default = 0.0005+eps;
Pfa_max_default = 0.5-eps;

global DET_limits;

%-------------------------
% If value not supplied as arguement, then use previous value
% or use default value if DET_limits hasn't been initialized.

if (~isempty(DET_limits))
	Pmiss_min_default = DET_limits(1);
	Pmiss_max_default = DET_limits(2);
	Pfa_min_default  = DET_limits(3);
	Pfa_max_default  = DET_limits(4);
end

if ~(exist('Pmiss_min')); Pmiss_min = Pmiss_min_default; end;
if ~(exist('Pmiss_max')); Pmiss_max = Pmiss_max_default; end;
if ~(exist('Pfa_min')); Pfa_min = Pfa_min_default; end;
if ~(exist('Pfa_max')); Pfa_max = Pfa_max_default; end;

%-------------------------
% Limit bounds to reasonable values

Pmiss_min = max(Pmiss_min,eps);
Pmiss_max = min(Pmiss_max,1-eps);
if Pmiss_max <= Pmiss_min
	Pmiss_min = eps;
	Pmiss_max = 1-eps;
end

Pfa_min = max(Pfa_min,eps);
Pfa_max = min(Pfa_max,1-eps);
if Pfa_max <= Pfa_min
	Pfa_min = eps;
	Pfa_max = 1-eps;
end

%--------------------------
% Load DET_limits with bounds to use

DET_limits = [Pmiss_min Pmiss_max Pfa_min Pfa_max];

