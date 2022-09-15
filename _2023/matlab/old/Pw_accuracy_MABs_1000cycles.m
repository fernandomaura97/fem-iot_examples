T_tx = 5.76 * 10^(-4); %%250 kbps
V = 3;
I_tx = 0.034; %%7dB

E_1tx = T_tx * V * I_tx;

%%MABS
Nb_1m = 1000;
n_tx1m = 251;

Nb_2m = 1000;
n_tx2m = 304;

Nb_3m = 1000;
n_tx3m = 305; 

Nb_4m = 1000;
n_tx4m = 240;

ar_nbm = [Nb_1m, Nb_2m, Nb_3m, Nb_4m];
ar_ntxm = [n_tx1m, n_tx2m, n_tx3m, n_tx4m];

%a_Etx_every = ar_nbm * E_1tx;
%a_Etx_MAB = ar_ntxm * E_1tx; 

%P_reduction_MAB = (a_Etx_MAB ./ a_Etx_every) * 100         MAL!!
Percent_dic = ((ar_nbm-ar_ntxm)./ar_nbm)*100

%%%Accuracy results: 
Threshold = 0.25; % ºC This info can be computed in advance, for each data type

readChannelID =  1764122; 
TemperatureFieldID = [1,2,3,4,5]; 
readAPIKey = 'JWG70R0JPMTX1U5G'; 

[data_MAB,time,channel_info] = thingSpeakRead(readChannelID,'Fields',TemperatureFieldID,'ReadKey',readAPIKey ,DateRange=[datetime(2022,6,30,16,06,00),datetime(2022,7,1,08,49,00)]);


index_id1m = find( (data_MAB (1:end,1) == 1) & (data_MAB (1:end,2)~=0)); %%correct
index_id2m = find( (data_MAB (1:end,1) == 2) & (data_MAB (1:end,3)~=0));
index_id3m = find( (data_MAB (1:end,1) == 3) & (data_MAB (1:end,4)~=0));
index_id4m = find( (data_MAB (1:end,1) == 4) & (data_MAB (1:end,5)~=0));

t1_m = data_MAB(index_id1m, 2);
t2_m = data_MAB(index_id2m, 3);
t3_m = data_MAB(index_id3m, 4);
t4_m = data_MAB(index_id4m, 5); 

plot(time(index_id1),t1_m)
hold on;
plot(time(index_id2),t2_m)
hold on;
plot(time(index_id3),t3_m)
hold on;
plot(time(index_id4),t4_m)

counter_t1 = 0;
counter_t2 = 0; 
counter_t3 = 0;
counter_t4 = 0; 

for i = 1:(length(t1_m)-1)
    t1_m1 = t1_m(i);
    t1_m2 = t1_m(i+1);    
    if(abs(t1_m1-t1_m2)>Threshold)
        counter_t1 = counter_t1 + 1; 
    end

end
acc1_m = 1 - (counter_t1 / length(t1_m))

for i = 1:(length(t2_m)-1)
    t2_m1 = t2_m(i);
    t2_m2 = t2_m(i+1);    
    if(abs(t2_m1-t2_m2)>Threshold)
        counter_t2 = counter_t2 + 1; 
    end

end
acc2_m = 1 - (counter_t2 / length(t2_m))

for i = 1:(length(t3_m)-1)
    t3_m1 = t3_m(i);
    t3_m2 = t3_m(i+1);    
    if(abs(t3_m1-t3_m2)>Threshold)
        counter_t3 = counter_t3 + 1; 
    end

end
acc3_m = 1 - (counter_t3 / length(t3_m))

for i = 1:(length(t4_m)-1)
    t4_m1 = t4_m(i);
    t4_m2 = t4_m(i+1);    
    if(abs(t4_m1-t4_m2)>Threshold)
        counter_t4 = counter_t4 + 1; 
    end

end
acc4_m = 1 - (counter_t4 / length(t4_m))