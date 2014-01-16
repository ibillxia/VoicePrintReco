function h = Plot_DET (Pmiss, Pfa, plot_code, opt_thickness)
%function h = Plot_DET (Pmiss, Pfa, plot_code, opt_thickness)
%
%  Plot_DET plots detection performance tradeoff on a DET plot
%  and returns the handle for plotting.
%
%  Pmiss and Pfa are the vectors of miss and corresponding false
%  alarm probabilities to be plotted.
%
%  The usage of Plot_DET is analogous to the standard matlab
%  p,ot function.
%
% See DET_usage for an example of how to use Plot_DET.
%
% opt_thickness : controls the line thickness. The default thickness
% is 0.5. A value between 2 and 5 will give a nice thick line.
%

Npts = max(size(Pmiss));
if Npts ~= max(size(Pfa))
        error ('vector size of Pmiss and Pfa not equal in call to Plot_DET');
end

%------------------------------
% plot the DET

if ~exist('plot_code')
        plot_code = 'y';
end

if ~exist('opt_thickness')
        opt_thickness = 0.5;
end

Set_DET_limits;
h = thick(opt_thickness,plot(ppndf(Pfa), ppndf(Pmiss), plot_code));
Make_DET;


function Make_DET()
%function Make_DET()
%
%  Make_DET creates a plot for displaying the Detection Error
%  Trade-off for a detection system.  The detection performance
%  is characterized by the miss and false alarm probabilities,
%  with the axes scaled and labeled so that a normal Gaussian
%  distribution will plot as a straight line.
%
%    The y axis represents the miss probability.
%    The x axis represents the false alarm probability.
%
%  See also Compute_DET, Plot_DET and Plot_DCF.

pticks = [0.00001 0.00002 0.00005 0.0001  0.0002   0.0005 ...
          0.001   0.002   0.005   0.01    0.02     0.05 ...
          0.1     0.2     0.4     0.6     0.8      0.9 ...
          0.95    0.98    0.99    0.995   0.998    0.999 ...
          0.9995  0.9998  0.9999  0.99995 0.99998  0.99999];

xlabels = [' 0.001' ; ' 0.002' ; ' 0.005' ; ' 0.01 ' ; ' 0.02 ' ; ' 0.05 ' ; ...
           '  0.1 ' ; '  0.2 ' ; ' 0.5  ' ; '  1   ' ; '  2   ' ; '  5   ' ; ...
           '  10  ' ; '  20  ' ; '  40  ' ; '  60  ' ; '  80  ' ; '  90  ' ; ...
           '  95  ' ; '  98  ' ; '  99  ' ; ' 99.5 ' ; ' 99.8 ' ; ' 99.9 ' ; ...
           ' 99.95' ; ' 99.98' ; ' 99.99' ; '99.995' ; '99.998' ; '99.999'];

ylabels = xlabels;

%---------------------------
% Get the min/max values of Pmiss and Pfa to plot

global DET_limits;

if isempty(DET_limits)
	Set_DET_limits;
end

Pmiss_min = DET_limits(1);
Pmiss_max = DET_limits(2);
Pfa_min   = DET_limits(3);
Pfa_max   = DET_limits(4);

%----------------------------
% Find the subset of tick marks to plot

ntick = max(size(pticks));
for (n=ntick:-1:1)
	if (Pmiss_min <= pticks(n))
		tmin_miss = n;
	end
	if (Pfa_min <= pticks(n))
		tmin_fa = n;
	end
end

for (n=1:ntick)
	if (pticks(n) <= Pmiss_max)
		tmax_miss = n;
	end
	if (pticks(n) <= Pfa_max)
		tmax_fa = n;
	end
end

%-----------------------------
% Plot the DET grid

set (gca, 'xlim', ppndf([Pfa_min Pfa_max]));
set (gca, 'xtick', ppndf(pticks(tmin_fa:tmax_fa)));
set (gca, 'xticklabel', xlabels(tmin_fa:tmax_fa,:));
set (gca, 'xgrid', 'on');
xlabel ('False Alarm probability (in %)');


set (gca, 'ylim', ppndf([Pmiss_min Pmiss_max]));
set (gca, 'ytick', ppndf(pticks(tmin_miss:tmax_miss)));
set (gca, 'yticklabel', ylabels(tmin_miss:tmax_miss,:));
set (gca, 'ygrid', 'on')
ylabel ('Miss probability (in %)')

set (gca, 'box', 'on');
axis('square');
axis(axis);

