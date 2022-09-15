[data,timestamps,channelInfo] = thingSpeakRead(1721563,Fields=[1,2,3,4],DateRange=[datetime(2022,5,3,10,00,01),datetime(2022,5,3,14,30,00)])


[a,b] = size(data);
control = zeros(a,1); 

index = find( data (1:end,2) == 1); %index of nodeid1
o_index = find( data (1:end,2) == 2); %index of nodeid2

data2(isnan(data))=0;
lastnonzero = 0;  %%maybe should be initialised to 1
data3 = data2;

for i=1:length(data3) %% Fill NaN with last non-zero value (sampling rate)
       
    if data3(i) ~= 0
        lastnonzero = data3(i);
    elseif(data3(i)==0)
        data3(i) = lastnonzero;       
    end  
end


temperature_c = (data(index,3));
hum_c = (data(index,4));
timestamp_c = timestamps(index);


temp_o = data(o_index, 3);
hum_o = data(o_index, 4);
timestamp_o = timestamps(o_index);

temperature_c = filloutliers(temperature_c,'nearest','mean');
temp_o = filloutliers(temp_o,'nearest','mean');
hum_c = filloutliers(hum_c,'nearest','mean');
hum_o = filloutliers(hum_o,'nearest','mean');


plot_t1 = subplot(3,2,1);
plot(timestamp_c, temperature_c, '-o')
xlabel('Time')
ylabel('Temperature (Control)')

plot_h1 = subplot(3,2,2)
plot(timestamp_c, hum_c, '-o')
xlabel('time')
ylabel('humidity (Control)')


plot_t2 = subplot(3,2,3)
plot(timestamp_o, temp_o, '-o')
xlabel('time')
ylabel('temperature (Adaptive Sampling)')

plot_h2 =subplot(3,2,4)
plot(timestamp_o, hum_o, '-o')
xlabel('time')
ylabel('humidity (Adaptive Sampling))')

% subplot(3,2,5)
% plot(timestamp, data3, 'b-')
% xlabel('time')
% ylabel('sample rate for node2 [cycles of 1 min]')


linkaxes([plot_t1, plot_t2],'xy')
linkaxes([plot_h1, plot_h2], 'xy')

