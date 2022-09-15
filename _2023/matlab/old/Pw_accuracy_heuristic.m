
T_tx = 5.76 * 10^(-4); %%250 kbps
V = 3;
I_tx = 0.034; %%7dB

E_1tx = T_tx * V * I_tx;

%%Heuristic
Nb_1h = 120;
Nb_2h = 120;
Nb_3h = 120;
Nb_4h = 120;

n_tx1h = 28;
n_tx2h = 35;
n_tx3h = 27;
n_tx4h = 34;

ar_nbh = [Nb_1h, Nb_2h, Nb_3h, Nb_4h];
ar_ntxh = [n_tx1h, n_tx2h, n_tx3h, n_tx4h];


a_Etx_everyh = ar_nbh * E_1tx;
a_Etx_heur = ar_ntxh * E_1tx; 

Percent_dic = ((ar_nbh-ar_ntxh)./ar_nbh)*100







%%%%%%%%%%%%%%%%%%%%%%%%%%Accuracy results: %%%%%%%%%%%%%%%%%%%%%%%%%%

Threshold = 0.25; % ºC This info can be computed in advance, for each data type



readChannelID =  1764122; 
TemperatureFieldID = [1,2,3,4,5]; 
readAPIKey = 'JWG70R0JPMTX1U5G'; 

[data_heur,time,channel_info] = thingSpeakRead(readChannelID,'Fields',TemperatureFieldID,'ReadKey',readAPIKey ,DateRange=[datetime(2022,6,30,13,01,00),datetime(2022,6,30,15,11,00)]);

index_id1 = find( (data_heur (1:end,1) == 1) & (data_heur (1:end,2)~=0)); %%correct
index_id2 = find( (data_heur (1:end,1) == 2) & (data_heur (1:end,3)~=0));
index_id3 = find( (data_heur (1:end,1) == 3) & (data_heur (1:end,4)~=0));
index_id4 = find( (data_heur (1:end,1) == 4) & (data_heur (1:end,5)~=0));


t1_h = data_heur(index_id1, 2);
t2_h = data_heur(index_id2, 3);
t3_h = data_heur(index_id3, 4);
t4_h = data_heur(index_id4, 5); 

%t1_h1 = t1_h(1);
%t1_h2 = t1_h(2);

counter_t1 = 0; 

for i = 1:(length(t1_h)-1)
    t1_h1 = t1_h(i);
    t1_h2 = t1_h(i+1);    
    if(abs(t1_h1-t1_h2)>Threshold)
        counter_t1 = counter_t1 + 1; 
    end

end
acc1_h = 1 - (counter_t1 / length(t1_h))

%%ID2 heur
counter_t2 = 0; 
for i = 1:(length(t2_h)-1)
    t2_h1 = t2_h(i);
    t2_h2 = t2_h(i+1);    
    if(abs(t2_h1-t2_h2)>Threshold)
        counter_t2 = counter_t2 + 1; 
    end

end
acc2_h = 1 - (counter_t2 / length(t2_h))

counter_t3 = 0; 
for i = 1:(length(t3_h)-1)
    t3_h1 = t3_h(i);
    t3_h2 = t3_h(i+1);    
    if(abs(t3_h1-t3_h2)>Threshold)
        counter_t3 = counter_t3 + 1; 
    end

end

acc3_h = 1 - (counter_t3 / length(t3_h))

counter_t4 = 0; 
for i = 1:(length(t4_h)-1)
    t4_h1 = t4_h(i);
    t4_h2 = t4_h(i+1);    
    if(abs(t4_h1-t4_h2)>Threshold)
        counter_t4 = counter_t4 +1; 
    end

end
acc4_h = 1 - (counter_t4 / length(t4_h))






