
 Threshold_PM10 = 3;
 Threshold_CO = 0.5;
 Threshold_NO2 = 0.2;

readChannelID = 1860800;

readAPIKey = '723ZV5H7F6U45MLS';

[data_MAB, time, channel_info] = thingSpeakRead(readChannelID, 'Fields', [1,2,3,4], 'ReadKey', readAPIKey, NumMinutes=100);

index_id5 = find(( data_MAB(1:end,1) == 5) & (data_MAB (1:end,2)~= -1));
index_id6 = find(( data_MAB(1:end,1) == 6) & (data_MAB (1:end,3)~= -1));

temp5 = data_MAB(index_id5, [2,3,4]);
CO_5 = temp5(:,1);
NO2_5 = temp5(:, 2);
PM10_5 = temp5(:,3);

temp6 = data_MAB(index_id6, [2,3,4]);
CO_6 = temp6(:,1);
NO2_6 = temp6(:,2);
PM10_6 = temp6(:,3);

plot(time(index_id5), CO_5)
hold on;
plot(time(index_id5), NO2_5)
hold on;
plot(time(index_id5), PM10_5) 
hold off;

counter5_co = 0;
counter5_no2 = 0;
counter5_pm10 = 0; 

counter6_co = 0; 
counter6_no2 = 0; 
counter6_pm10 = 0;


for i = 1:(length(CO_5)-1)
    co_1a = CO_5(i);
    co_2a = CO_5(i+1);
    if(abs(co_1a - co_2a) > Threshold_CO)
        counter5_co = counter5_co + 1; 
    end
end
acc_co_5 = 1 - (counter5_co / length(CO_5))

for i = 1:(length(CO_6)-1)
    co_1b = CO_6(i);
    co_2b = CO_6(i+1);
    if(abs(co_1b - co_2b) > Threshold_CO)
        counter6_co = counter6_co + 1; 
    end
end
acc_co_6 = 1 - (counter6_co / length(CO_6))

for i = 1:(length(NO2_5)-1)
    no2_a = NO2_5(i);
    no2_b = NO2_5(i+1);
    if(abs(no2_a - no2_b)>Threshold_NO2)
          counter5_no2 = counter5_no2 + 1;
    end
end
acc_no2_5 = 1 - (counter5_no2 / length(NO2_5))

for i = 1:(length(NO2_6)-1)
    no2_a = NO2_5(i);
    no2_b = NO2_5(i+1);
    if(abs(no2_a - no2_b)>Threshold_NO2)
          counter6_no2 = counter6_no2 + 1;
    end
end
acc_no2_6 = 1 - (counter6_no2 / length(NO2_6))

for i = 1:(length(PM10_5)-1)
    
    pm10_a = PM10_5(i);
    pm10_b = PM10_5(i+1);
    if(abs(pm10_a - pm10_b)>Threshold_PM10)
        counter5_pm10 = counter5_pm10 + 1;
    end 
end
acc_pm10_5 = 1 - (counter5_pm10 / length(PM10_5))

for i = 1:(length(PM10_6)-1)
    
    pm10_a = PM10_6(i);
    pm10_b = PM10_6(i+1);
    if(abs(pm10_a - pm10_b)>Threshold_PM10)
        counter6_pm10 = counter6_pm10 + 1;
    end 
end
acc_pm10_6 = 1 - (counter6_pm10 / length(PM10_6))



