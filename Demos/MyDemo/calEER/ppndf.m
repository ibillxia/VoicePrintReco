function norm_dev = ppndf (cum_prob)
%function ppndf (prob)
%The input to this function is a cumulative probability.
%The output from this function is the Normal deviate
%that corresponds to that probability.  For example:
%  INPUT   OUTPUT
%  0.001   -3.090
%  0.01    -2.326
%  0.1     -1.282
%  0.5      0.0
%  0.9      1.282
%  0.99     2.326
%  0.999    3.090

 SPLIT =  0.42;

 A0 =   2.5066282388;
 A1 = -18.6150006252;
 A2 =  41.3911977353;
 A3 = -25.4410604963;
 B1 =  -8.4735109309;
 B2 =  23.0833674374;
 B3 = -21.0622410182;
 B4 =   3.1308290983;
 C0 =  -2.7871893113;
 C1 =  -2.2979647913;
 C2 =   4.8501412713;
 C3 =   2.3212127685;
 D1 =   3.5438892476;
 D2 =   1.6370678189;

% the following code is matlab-tized for speed.
% on 200000 points, time went from 76 seconds to 5 seconds!
% original routine is included at end for reference

[Nrows Ncols] = size(cum_prob);
norm_dev = zeros(Nrows, Ncols); % preallocate norm_dev for speed
cum_prob(find(cum_prob>= 1.0)) = 1-eps;
cum_prob(find(cum_prob<= 0.0)) = eps;

R = zeros(Nrows, Ncols); % preallocate R for speed

% adjusted prob matrix
adj_prob=cum_prob-0.5;

centerindexes = find(abs(adj_prob) <= SPLIT);
tailindexes   = find(abs(adj_prob) > SPLIT);

% do centerstuff first
R(centerindexes) = adj_prob(centerindexes) .* adj_prob(centerindexes);
norm_dev(centerindexes) = adj_prob(centerindexes) .* ...
                    (((A3 .* R(centerindexes) + A2) .* R(centerindexes) + A1) .* R(centerindexes) + A0);
norm_dev(centerindexes) = norm_dev(centerindexes) ./ ((((B4 .* R(centerindexes) + B3) .* R(centerindexes) + B2) .* ...
                             R(centerindexes) + B1) .* R(centerindexes) + 1.0);


% find left and right tails
right = find(cum_prob(tailindexes)> 0.5);
left  = find(cum_prob(tailindexes)< 0.5);

% do tail stuff
R(tailindexes) = cum_prob(tailindexes);
% if prob > 0.5 then prob = 1-prob
R(tailindexes(right)) = 1 - cum_prob(tailindexes(right));
R(tailindexes) = sqrt ((-1.0) .* log (R(tailindexes)));
norm_dev(tailindexes) = (((C3 .* R(tailindexes) + C2) .* R(tailindexes) + C1) .* R(tailindexes) + C0);
norm_dev(tailindexes) = norm_dev(tailindexes) ./ ((D2 .* R(tailindexes) + D1) .* R(tailindexes) + 1.0);

% swap sign on left tail
norm_dev(tailindexes(left)) = norm_dev(tailindexes(left)) .* -1.0;

return

%--------------------
% here is the old routine, which is much slower

function norm_dev = oldppndf (cum_prob)
%function ppndf (prob)
%The input to this function is a cumulative probability.
%The output from this function is the Normal deviate
%that corresponds to that probability.  For example:
%  INPUT   OUTPUT
%  0.001   -3.090
%  0.01    -2.326
%  0.1     -1.282
%  0.5      0.0
%  0.9      1.282
%  0.99     2.326
%  0.999    3.090

 SPLIT =  0.42;

 A0 =   2.5066282388;
 A1 = -18.6150006252;
 A2 =  41.3911977353;
 A3 = -25.4410604963;
 B1 =  -8.4735109309;
 B2 =  23.0833674374;
 B3 = -21.0622410182;
 B4 =   3.1308290983;
 C0 =  -2.7871893113;
 C1 =  -2.2979647913;
 C2 =   4.8501412713;
 C3 =   2.3212127685;
 D1 =   3.5438892476;
 D2 =   1.6370678189;

[Nrows Ncols] = size(cum_prob);
norm_dev = zeros(Nrows, Ncols); % preallocate norm_dev for speed
for irow=1:Nrows
   for icol=1:Ncols

      prob = cum_prob(irow, icol);
      if (prob >= 1.0)
         prob = 1-eps;
      elseif (prob <= 0.0)
         prob = eps;
      end

      q = prob - 0.5;
      if (abs(prob-0.5) <= SPLIT)
         r = q * q;
         pf = q * (((A3 * r + A2) * r + A1) * r + A0);
         pf = pf / ((((B4 * r + B3) * r + B2) * r + B1) * r + 1.0);
 
      else
         if (q>0.0)
            r = 1.0-prob;
         else
            r = prob;
         end

         r = sqrt ((-1.0) * log (r));
         pf = (((C3 * r + C2) * r + C1) * r + C0);
         pf = pf / ((D2 * r + D1) * r + 1.0);
         if (q < 0)
            pf = pf * (-1.0);
         end
      end
      norm_dev(irow, icol) = pf;
   end
end

