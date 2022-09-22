
Threshold_PM10 = 3;
Threshold_CO = 0.5;
Threshold_NO2 = 0.2;
Threshold_t = 0.15; % ºC This info can be computed in advance, for each data type


readChannelID = 1764122; 
readChannelID_m = 1721563;
FieldID = [1,2,3,4,5]; 
readAPIKey = 'JWG70R0JPMTX1U5G'; 
readAPIKey_m = 'Z6QVFMNONF947MJA';

[data_MAB,time,channel_info] = thingSpeakRead(readChannelID,'Fields',FieldID,'NumPoints',50,'ReadKey',readAPIKey); 

[data_MAB_m, time_m, channel_info_m] = thingSpeakRead(readChannelID_m, 'Fields', [1,2,3,4], 'ReadKey', readAPIKey_m, 'NumPoints', 100);


index_id1 = find( (data_MAB (1:end,1) == 1) & (data_MAB (1:end,2)~=-1)); %%correct
index_id2 = find( (data_MAB (1:end,1) == 2) & (data_MAB (1:end,3)~=-1));
index_id3 = find( (data_MAB (1:end,1) == 3) & (data_MAB (1:end,4)~=-1));
index_id4 = find( (data_MAB (1:end,1) == 4) & (data_MAB (1:end,5)~=-1));
index_id5 = find(( data_MAB_m(1:end,1) == 5) & (data_MAB_m (1:end,2)~= -1));
%index_id6 = find(( data_MAB_m(1:end,1) == 6) & (data_MAB_m (1:end,3)~= -1));

%get data of every sensor 
    %temperature data
    t1 = data_MAB(index_id1, 2);
    t2 = data_MAB(index_id2, 3);
    t3 = data_MAB(index_id3, 4);
    t4 = data_MAB(index_id4, 5); 
<<<<<<< HEAD
    %% gas data id5
=======
    %gas data id5
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    temp5 = data_MAB_m(index_id5, [2,3,4]);
    CO_5 = temp5(:,1);
    NO2_5 = temp5(:, 2);
    PM10_5 = temp5(:,3);
<<<<<<< HEAD
    %% gas data id6
=======
    %gas data id6
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
%     temp6 = data_MAB_m(index_id6, [2,3,4]);
%     CO_6 = temp6(:,1);
%     NO2_6 = temp6(:,2);
%     PM10_6 = temp6(:,3);
%
<<<<<<< HEAD

=======
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
%get timestamps of every data point for every variable
timestamps_id1 = time(index_id1);
timestamps_id2 = time(index_id2);
timestamps_id3 = time(index_id3);
timestamps_id4 = time(index_id4);
timestamps_id5 = time_m(index_id5);
%timestamps_id6 = time_m(index_id6);

<<<<<<< HEAD
%get timestamp of latest data point

t_now = datevec(datenum(datetime('now'))); %current time

if (~isempty(t1))
    tlast_id1 = datevec(datenum( timestamps_id1(end)));
    dt_id1 = etime(t_now, tlast_id1);
else
    display('id1 has dropped');
    dt_id1 = abs(etime(t_now , datevec(datenum(datetime('now')+seconds(3600)))));
end

if (~isempty(t2))
    tlast_id2 = datevec(datenum( timestamps_id2(end)));
    dt_id2 = etime(t_now, tlast_id2);
else
    display('id2 has dropped');
    dt_id2 = abs(etime(t_now , datevec(datenum(datetime('now')+seconds(3600)))));
end

if (~isempty(t3))
    tlast_id3 = datevec(datenum( timestamps_id3(end)));
    dt_id3 = etime(t_now, tlast_id3);
else
    display('id3 has dropped');
    dt_id3 = abs(etime(t_now , datevec(datenum(datetime('now')+seconds(3600)))));
end

if (~isempty(t4))
    tlast_id4 = datevec(datenum( timestamps_id4(end)));
    dt_id4 = etime(t_now, tlast_id4);
else
    display('id4 has dropped');
    dt_id4 = abs(etime(t_now , datevec(datenum(datetime('now')+seconds(3600)))));
end

if (~isempty(CO_5)||~isempty(NO2_5)||~isempty(PM10_5))
    tlast_id5 = datevec(datenum( timestamps_id5(end)));  
    dt_id5 = etime(t_now, tlast_id5);
else
    display('id5 has dropped');
    dt_id5 = abs(etime(t_now , datevec(datenum(datetime('now')+seconds(3600)))));
end

%compare current time and last data point, check 'freshness'
% dt_id1 = etime(t_now, tlast_id1);
% dt_id2 = etime(t_now, tlast_id2);
% dt_id3 = etime(t_now, tlast_id3);
% dt_id4 = etime(t_now, tlast_id4);
% dt_id5 = etime(t_now, tlast_id5);
%dt_id6 = etime(t_now, tlast_id6);


if(dt_id1 <= 65) %%if last message is 'fresh' (i.e. arrived this cycle)
=======
%get timestamp of latest data point 
t_now = datevec(datenum(datetime('now'))); %current time
tlast_id1 = datevec(datenum( timestamps_id1(end)));
tlast_id2 = datevec(datenum(timestamps_id2(end)));
tlast_id3= datevec(datenum( timestamps_id3(end)));
tlast_id4 = datevec(datenum(timestamps_id4(end)));
tlast_id5 = datevec(datenum(timestamps_id5(end)));
%tlast_id6 = datevec(datenum(timestamps_id6(end)));

%compare current time and last data point, check 'freshness'
dt_id1 = etime(t_now, tlast_id1);
dt_id2 = etime(t_now, tlast_id2);
dt_id3 = etime(t_now, tlast_id3);
dt_id4 = etime(t_now, tlast_id4);
dt_id5 = etime(t_now, tlast_id5);
%dt_id6 = etime(t_now, tlast_id6);


if(dt_id1 <= 60) %%if last message is 'fresh' (i.e. arrived this cycle)
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    f1 = 1; %%flag for later
else
    f1 = 0;
end
<<<<<<< HEAD
if(dt_id2 <= 66) 
=======
if(dt_id2 <= 60) 
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    f2 = 1;
else
    f2 = 0;
end
<<<<<<< HEAD
if(dt_id3 <= 67)
=======
if(dt_id3 <= 60)
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    f3 = 1;
else
    f3 = 0;
end
<<<<<<< HEAD
if(dt_id4 <= 68) 
=======
if(dt_id4 <= 60) 
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    f4 = 1;
else
    f4 = 0;
end
<<<<<<< HEAD
if(dt_id5 <= 69) 
=======
if(dt_id5 <= 60) 
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    f5 = 1;
else
    f5 = 0;
end
% if(dt_id6 <= 60) 
%     f6 = 1;
% else
%     f6 = 0;
% end
   
    
 %% channel for ID + rewards: 1777720
%% W: SEHQCVT1DS0L6W2J  R: 94QU8IYRJXPKSX7P  
%%latest_sr_rewards
ID_S_RW = thingSpeakRead(1777720,'Fields',[1:7],'NumPoints',50,'ReadKey','94QU8IYRJXPKSX7P');

%indexes for every nodeID
ind_s1 = find(ID_S_RW(1:end,1) == 1);
ind_s2 = find(ID_S_RW(1:end,1) == 2);
ind_s3 = find(ID_S_RW(1:end,1) == 3);
ind_s4 = find(ID_S_RW(1:end,1) == 4);
ind_s5 = find(ID_S_RW(1:end,1) == 5);
ind_s6 = find(ID_S_RW(1:end,1) == 6);

%Latest sampling rate and rwrds for each ID
Sa_RW1 = ID_S_RW(ind_s1, (2:end));
Sa_RW2 = ID_S_RW(ind_s2, (2:end));
Sa_RW3 = ID_S_RW(ind_s3, (2:end));
Sa_RW4 = ID_S_RW(ind_s4, (2:end));
Sa_RW5 = ID_S_RW(ind_s4, (2:end));
Sa_RW6 = ID_S_RW(ind_s4, (2:end));

<<<<<<< HEAD
debug = 1;
%%[DEBUG] PRINT ALL
if debug
    disp('Sa_RW1: ');
    %disp(Sa_RW1);
    disp(Sa_RW1(end,:));
    
    disp('Sa_RW2: ');
    %disp(Sa_RW2);
    disp(Sa_RW2(end,:));
    
    disp('Sa_RW3: ');
    %disp(Sa_RW3);
    disp(Sa_RW3(end,:));
    
    disp('Sa_RW4: ');
    %disp(Sa_RW4);
    disp(Sa_RW4(end,:));
    
    disp('Sa_RW5: ');
    %disp(Sa_RW5);
    disp(Sa_RW5(end,:));
    
    disp('Sa_RW6: ');
    %disp(Sa_RW6);
    disp(Sa_RW6(end,:));
end
%% [DEBUG END]


=======
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
%% channel for sampling rates:  1777719
%% W: 2296YVIBYEW5796F  R: RO50KRZDA98EVRSW

S_RW1 = Sa_RW1(end,:);
S_RW2 = Sa_RW2(end,:);
S_RW3 = Sa_RW3(end,:);
S_RW4 = Sa_RW4(end,:);
S_RW5 = Sa_RW5(end,:);
S_RW6 = Sa_RW6(end,:);


global T_new1;
global T_new2;
global T_new3; 
global T_new4;
global T_new5;
global T_new6;

T_new1 = zeros(1,6);
T_new2 = zeros(1,6);
T_new3 = zeros(1,6);
T_new4 = zeros(1,6);
T_new5 = zeros(1,6);
T_new6 = zeros(1,6);

<<<<<<< HEAD
%% Calculate next sampling BEGIN
if(f1 ==1)
    Dif1 = abs(t1(end)-t1(end-1));
=======

if(f1 ==1)
    Dif1 = abs(t1(end)-t1(end-1);
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    Rwrd1 = 0;
    last_action1 = S_RW1(end,1);  
    %calculate next reward
    if(Dif1 <Threshold_t)
        Rwrd1 = last_action1;
    else
        Rwrd1 = 0; 
    end
    %Update the reward: 
    global T_new1;
    for i = 1:5
        if(i == last_action1)
            T_new1(i+1) = 0.5*S_RW1(i+1) + 0.5 * Rwrd1;
        else
            T_new1(i+1) = S_RW1(i+1);
        end
    end
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

if(f2 ==1)
<<<<<<< HEAD
    Dif = abs(t2(end)-t2(end-1));
=======
    Dif = abs(t2(end)-t2(end-1);
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    Rwrd = 0;
    last_action = S_RW2(end,1);  
    %calculate next reward
    if(Dif <Threshold_t)
        Rwrd = last_action;
    else
        Rwrd = 0; 
    end
    %Update the reward: 
    global T_new2;
    for i = 1:5
        if(i == last_action)
            T_new2(i+1) = 0.5*S_RW2(i+1) + 0.5 * Rwrd;
        else
            T_new2(i+1) = S_RW2(i+1);
        end
    end
     explore = rand();
    next_T = 0;
    if explore < 0.25
        next_T = randi(5,1);
    else
        [~,next_T] = max(T_new2(2:6));
    end

    disp('explore2 | nextT2 is:');
    disp([explore next_T]);
    T_new2(1)=next_T; 
end

if(f3 ==1)
<<<<<<< HEAD
    Dif = abs(t3(end)-t3(end-1));
=======
    Dif = abs(t3(end)-t3(end-1);
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    Rwrd = 0;
    last_action = S_RW3(end,1);  
    %calculate next reward
    if(Dif <Threshold_NO2)  %%%Depending on what variable we want to track!!
        Rwrd = last_action;
    else
        Rwrd = 0; 
    end
    %Update the reward: 
    global T_new3;
    for i = 1:5
        if(i == last_action)
            T_new3(i+1) = 0.5*S_RW3(i+1) + 0.5 * Rwrd;
        else
            T_new3(i+1) = S_RW3(i+1);
        end
    end
     explore = rand();
    next_T = 0;
    if explore < 0.25
        next_T = randi(5,1);
    else
        [~,next_T] = max(T_new3(2:6));
    end

    disp('explore3 | nextT3 is:');
    disp([explore next_T]);
    T_new3(1)=next_T; 
end


if(f4 ==1)
<<<<<<< HEAD
    Dif = abs(t4(end)-t4(end-1));
=======
    Dif = abs(t4(end)-t4(end-1);
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    Rwrd = 0;
    last_action = S_RW4(end,1);  
    %calculate next reward
    if(Dif <Threshold_t)
        Rwrd = last_action;
    else
        Rwrd = 0; 
    end
    %Update the reward: 
    global T_new4;
    for i = 1:5
        if(i == last_action)
            T_new4(i+1) = 0.5*S_RW4(i+1) + 0.5 * Rwrd;
        else
            T_new4(i+1) = S_RW4(i+1);
        end
    end
    explore = rand();
    next_T = 0;
    if explore < 0.25
        next_T = randi(5,1);
    else
        [~,next_T] = max(T_new4(2:6));
    end

    disp('explore4 | nextT4 is:');
    disp([explore next_T]);
    T_new4(1)=next_T; 
end

if(f5 ==1)
<<<<<<< HEAD
    Dif = abs(NO2_5(end)-NO2_5(end-1));
=======
    Dif = abs(t5(end)-t5(end-1);
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    Rwrd = 0;
    last_action = S_RW5(end,1);  
    %calculate next reward
    if(Dif <Threshold_t)
        Rwrd = last_action;
    else
        Rwrd = 0; 
    end
    %Update the reward: 
    global T_new5;
    for i = 1:5
        if(i == last_action)
            T_new5(i+1) = 0.5*S_RW5(i+1) + 0.5 * Rwrd;
        else
            T_new5(i+1) = S_RW5(i+1);
        end
    end
    explore = rand();
    next_T = 0;
    if explore < 0.25
        next_T = randi(5,1);
    else
        [~,next_T] = max(T_new5(2:6));
    end

<<<<<<< HEAD
    disp('explore5 | nextT5 is:');
=======
    disp('explore45 | nextT5 is:');
>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff
    disp([explore next_T]);
    T_new5(1)=next_T; 
end

<<<<<<< HEAD
%% Calculate next Sampling END

if(f1|f2|f3|f4|f5)
    global T_new1;
    global T_new2;
    global T_new3; 
    global T_new4;
    global T_new5;
    
    newmatrix = zeros(5,7);
    newmatrix(:,1) = (1:5);
    
    if(f1)
        newmatrix(1,2:7) = T_new1;
    else
        newmatrix(1,2:7) = S_RW1(end,:);
    end
      
    if(f2)
        newmatrix(2,2:7) = T_new2;
    else
        newmatrix(2,2:7) = S_RW2(end,:);
    end
    
    if(f3)
        newmatrix(3,2:7) = T_new3;
    else
        newmatrix(3,2:7) = S_RW3(end,:);
    end
    
    if(f4)
        newmatrix(4,2:7) = T_new4;
    else
        newmatrix(4,2:7) = S_RW4(end,:);
    end
    
    if(f5) 
        newmatrix(5,2:7) = T_new5;
    else
        newmatrix(5,2:7) = T_new5;
    end
    
    
    
    disp ('New upload matrix: ')
    disp(newmatrix); 
    
    tst = datetime('now')-seconds(4):seconds(1):datetime('now');
    
    jj = thingSpeakWrite(1777720, newmatrix, 'TimeStamp', tst, 'WriteKey', 'SEHQCVT1DS0L6W2J')
    
    NEW_SamplingRates = [newmatrix(1,2),newmatrix(2,2),newmatrix(3,2),newmatrix(4,2),newmatrix(5,2)];
    kk = thingSpeakWrite(1777719, 'Fields',[1,2,3,4,5], 'Values', NEW_SamplingRates,'WriteKey', '2296YVIBYEW5796F')
end
=======





    
    
    


>>>>>>> ef7e54b197c2c56edbcc7fa88eecb172fa1072ff







