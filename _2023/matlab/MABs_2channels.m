

Threshold = 0.15; % ºC This info can be computed in advance, for each data type

% New measure received 
readChannelID = 1764122; 
TemperatureFieldID = [1,2,3,4,5]; 
readAPIKey = 'JWG70R0JPMTX1U5G'; 
[data_TS,time,channel_info] = thingSpeakRead(readChannelID,'Fields',TemperatureFieldID,'NumPoints',50,'ReadKey',readAPIKey); 

index_id1 = find( (data_TS (1:end,1) == 1) & (data_TS (1:end,2)~=0)); %%correct
index_id2 = find( (data_TS (1:end,1) == 2) & (data_TS (1:end,3)~=0));
index_id3 = find( (data_TS (1:end,1) == 3) & (data_TS (1:end,4)~=0));
index_id4 = find( (data_TS (1:end,1) == 4) & (data_TS (1:end,5)~=0));


t1 = data_TS(index_id1, 2);
t2 = data_TS(index_id2, 3);
t3 = data_TS(index_id3, 4);
t4 = data_TS(index_id4, 5); 

timestamp_id1 = time(index_id1);
timestamp_id2 = time(index_id2);
timestamp_id3 = time(index_id3);
timestamp_id4 = time(index_id4);

t_now = datevec(datenum(datetime('now')));
tlast_id1 = datevec(datenum( timestamp_id1(end)));
tlast_id2 = datevec(datenum(timestamp_id2(end)));
tlast_id3= datevec(datenum( timestamp_id3(end)));
tlast_id4 = datevec(datenum(timestamp_id4(end)));

dt_id1 = etime(t_now,tlast_id1);  %%time difference between last reading and actual time
dt_id2 = etime(t_now, tlast_id2);
dt_id3 = etime(t_now,tlast_id3);  %%time difference between last reading and actual time
dt_id4 = etime(t_now, tlast_id4);


if (dt_id1 <=61)
   flag_id1 = 1; %%meaning we received data of NODEID1 this cycle 
else 
    flag_id1 = 0;
end

if (dt_id2 <=61)
    flag_id2 = 1; %% """" NODEID2
else 
    flag_id2 = 0; 
end

if(dt_id3 <= 61)
    flag_id3 = 1; %% """" NODEID2
else 
    flag_id3 = 0; 
end

if(dt_id4 <= 61)
    flag_id4 = 1; %% """" NODEID2
else 
    flag_id4 = 0;
end


%% channel for ID + rewards: 1777720
%% W: SEHQCVT1DS0L6W2J  R: 94QU8IYRJXPKSX7P  

%% channel for sampling rates:  1777719
%% W: 2296YVIBYEW5796F  R: RO50KRZDA98EVRSW

ID_S_RW = thingSpeakRead(1777720,'Fields',[1:7],'NumPoints',200,'ReadKey','94QU8IYRJXPKSX7P');

%%[data_MAB,time,channel_info] = thingSpeakRead(readChannelID,'Fields',TemperatureFieldID,'ReadKey',readAPIKey ,DateRange=[datetime(2022,6,30,16,06,00),datetime(2022,6,30,18,06,00)]);


ind_s1 = find(ID_S_RW(1:end,1) == 1);
ind_s2 = find(ID_S_RW(1:end,1) == 2);
ind_s3 = find(ID_S_RW(1:end,1) == 3);
ind_s4 = find(ID_S_RW(1:end,1) == 4);

Sa_RW1 = ID_S_RW(ind_s1, (2:end));
Sa_RW2 = ID_S_RW(ind_s2, (2:end));
Sa_RW3 = ID_S_RW(ind_s3, (2:end));
Sa_RW4 = ID_S_RW(ind_s4, (2:end));

%%Get latest Sampling rate and rewards for each ID
S_RW1 = Sa_RW1(end,:);
S_RW2 = Sa_RW2(end,:);
S_RW3 = Sa_RW3(end,:);
S_RW4 = Sa_RW4(end,:);

global T_new1;
global T_new2;
global T_new3; 
global T_new4;




T_new1 = zeros(1,6);
T_new2 = zeros(1,6);
T_new3 = zeros(1,6);
T_new4 = zeros(1,6);

if (flag_id1 == 1)
    Difference1 = abs(t1(end)-t1(end-1));

    Reward1 = 0;
    last_action1 = S_RW1(end,1);

    % Calculate Rewards
    if (Difference1 < Threshold)
        Reward1 = last_action1;
    else
        Reward1 = 0;
    end

    % Update Rewards
    global T_new1
    for i=1:5
        if(i == last_action1)
           T_new1(i+1)=0.5*S_RW1(i+1) + 0.5*Reward1;  
        else
           T_new1(i+1)=S_RW1(i+1); 
        end
    end

    disp('Updated Rewards1');
    disp(T_new1);

    % MAB implementation
    explore1 = rand();
    next_T1 = 0;
    if explore1 < 0.25
        next_T1 = randi(5,1);
    else
        [~,next_T1] = max(T_new1(2:6));
    end

    disp('explore1 | nextT is1:');
    disp([explore1 next_T1]);

    T_new1(1)=next_T1;
    
end


if (flag_id2 == 1)
    Difference2 = abs(t2(end)-t2(end-1));

    Reward2 = 0;
    last_action2 = S_RW2(end,1);

    % Calculate Rewards
    if (Difference2 < Threshold)
        Reward2 = last_action2;
    else
        Reward2 = 0;
    end

    % Update Rewards
    global T_new2;
    for i=1:5
        if(i == last_action2)
           T_new2(i+1)=0.5*S_RW2(i+1) + 0.5*Reward2;  
        else
           T_new2(i+1)=S_RW2(i+1); 
        end
    end

    disp('Updated Rewards2');
    disp(T_new2);

    % MAB implementation
    explore2 = rand();
    next_T2 = 0;
    if explore2 < 0.25
        next_T2 = randi(5,1);
    else
        [~,next_T2] = max(T_new2(2:6));
    end

    disp('explore2 | nextT2 is:');
    disp([explore2 next_T2]);

    T_new2(1)=next_T2;
    
end

if (flag_id3 == 1)
    Difference3 = abs(t3(end)-t3(end-1));

    Reward3 = 0;
    last_action3 = S_RW3(end,1);

    % Calculate Rewards
    if (Difference3 < Threshold)
        Reward3 = last_action3;
    else
        Reward3 = 0;
    end

    % Update Rewards
    global T_new3;
    
    for i=1:5
        if(i == last_action3)
           T_new3(i+1)=0.5*S_RW3(i+1) + 0.5*Reward3;  
        else
           T_new3(i+1)=S_RW3(i+1); 
        end
    end

    disp('Updated Rewards3');
    disp(T_new3);

    % MAB implementation
    explore3 = rand();
    next_T3 = 0;
    if explore3 < 0.25
        next_T3 = randi(5,1);
    else
        [~,next_T3] = max(T_new3(2:6));
    end

    disp('explore3 | nextT3 is:');
    disp([explore3 next_T3]);

    T_new3(1)=next_T3;
    
end


if (flag_id4 == 1)
    Difference4 = abs(t4(end)-t4(end-1));

    Reward4 = 0;
    last_action4 = S_RW4(end,1);

    % Calculate Rewards
    if (Difference4 < Threshold)
        Reward4 = last_action4;
    else
        Reward4 = 0;
    end

    % Update Rewards
    global T_new4; 
    
    for i=1:5
        if(i == last_action4)
           T_new4(i+1)=0.5*S_RW4(i+1) + 0.5*Reward4;  
        else
           T_new4(i+1)=S_RW4(i+1); 
        end
    end

    disp('Updated Rewards4');
    disp(T_new4);

    % MAB implementation
    explore4 = rand();
    next_T4 = 0;
    if explore4 < 0.25
        next_T4 = randi(5,1);
    else
        [~,next_T4] = max(T_new4(2:6));
    end

    disp('explore | nextT is:');
    disp([explore4 next_T4]);

    T_new4(1)=next_T4;
    
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if(flag_id1 | flag_id2 | flag_id3 | flag_id4) 
    global T_new1;
    global T_new2;
    global T_new3;
    global T_new4; 
    
    
    newmatrix = zeros(4,7);
    newmatrix(:,1) = (1:4);  %% Make an empty matrix with ID of every one of the sampling rates, 
    
    if(flag_id1)
    newmatrix(1,2:7) = T_new1;
    else
    newmatrix(1,2:7) = S_RW1(end,:);
    end  
    
    if(flag_id2)
    newmatrix(2,2:7) = T_new2;
    else
      newmatrix(2,2:7) = S_RW2(end,:);
    end
    
    if(flag_id3)
        newmatrix(3,2:7) = T_new3;
    else 
        newmatrix(3,2:7) = S_RW3(end,:);
    end
    
    if(flag_id4)
        newmatrix(4,2:7) = T_new4;
    else
        newmatrix(4,2:7) = S_RW4(end,:);
    end
    
    
    disp('new upload matrix is: ');
    disp(newmatrix);

    %%Create different timestamps for the bulk update
    tst = datetime('now') - seconds(3):seconds(1):datetime('now');

    %%upload ID, Sampling Rates (or last action) and Rewards
    thingSpeakWrite(1777720, newmatrix, 'TimeStamp', tst, 'WriteKey', 'SEHQCVT1DS0L6W2J');

    SamplingRates = [ newmatrix(1,2), newmatrix(2,2), newmatrix(3,2), newmatrix(4,2)];
    %%Upload sampling rates
    thingSpeakWrite(1777719, 'Fields',[1,2,3,4], 'Values', SamplingRates,'WriteKey', '2296YVIBYEW5796F');

end