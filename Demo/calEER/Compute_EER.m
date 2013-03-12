function EER = Compute_EER(P_miss,P_fa)


%Zhenchun Lei 05-07-23

for j = 1 : size(P_miss, 1)
        if P_miss(j) >= P_fa(j)
            break
        end
end
    
if P_miss(j) == P_fa(j)
    EER = P_miss(j);
else
    EER = (P_miss(j)+P_fa(j)+P_miss(j-1)+P_fa(j-1))/4;
end