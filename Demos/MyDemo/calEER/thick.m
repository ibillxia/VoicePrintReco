function [lh] = thick(w,lh)
% THICK chages the width of the lines references by habdles
%    lh, the line handles
%     w, new width (default is 0.5)
% Example usage: thick(2,plot([1:5],[1,0,1,0,1],'b'))

for i=1:length(lh)
   set (lh(i),'LineWidth',w);
end
