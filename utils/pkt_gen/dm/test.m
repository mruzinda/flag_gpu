
Nm = 1;
Nt = 1;%85;
Nf = 3;
Nc = 8;


for m=0:Nm-1
    for t=0:Nt-1
        for f=0:Nf-1
            for c=0:Nc-1
                david_marsh_test(m,f,t,c)
            end
        end
    end
end