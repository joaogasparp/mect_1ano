%% Exercise 3.a.

% Parameters
lambda = 1500;  % pps (packets/sec)
C = 10;        % Link bandwidth (Mbps)
f = 10000;     % Queue size (Bytes)
P = 100000;    % Stop criterion (number of packets)
nVoIPs = [10, 20, 30, 40];  % VoIP flows
N = 20;        % Number of simulations
alpha = 0.1;   % for 90% confidence interval

% Results arrays
PLdata_results = zeros(N, length(nVoIPs));
PLVoIP_results = zeros(N, length(nVoIPs));
APDdata_results = zeros(N, length(nVoIPs));
APDVoIP_results = zeros(N, length(nVoIPs));

% Simulation loop
for i = 1:length(nVoIPs)
    for j = 1:N
        % Run Sim3 for each VoIP flow count
        [PLdata_results(j,i), PLVoIP_results(j,i), APDdata_results(j,i), APDVoIP_results(j,i)] = ...
            Sim3(lambda, C, f, P, nVoIPs(i));
    end
end

% Calculate mean and confidence intervals
mean_PLdata = mean(PLdata_results);
mean_PLVoIP = mean(PLVoIP_results);
mean_APDdata = mean(APDdata_results);
mean_APDVoIP = mean(APDVoIP_results);

% Standard error and 90% confidence interval (Z = 1.645)
Z = norminv(1 - alpha / 2);
ci_PLdata = Z * std(PLdata_results) / sqrt(N);
ci_PLVoIP = Z * std(PLVoIP_results) / sqrt(N);
ci_APDdata = Z * std(APDdata_results) / sqrt(N);
ci_APDVoIP = Z * std(APDVoIP_results) / sqrt(N);

% Plot Average Packet Delay for Data and VoIP with error bars
figure;
hold on; grid on;
b = bar(nVoIPs, [mean_APDdata; mean_APDVoIP]', 'grouped');  % Plot bar chart
% Get x-coordinates of each bar for accurate error bar placement
xData = b(1).XEndPoints;  % Data bar positions
errorbar(xData, mean_APDdata, ci_APDdata, 'k.', 'linestyle', 'none'); % Data errors centered
xData = b(2).XEndPoints;  % VoIP bar positions
errorbar(xData, mean_APDVoIP, ci_APDVoIP, 'k.', 'linestyle', 'none'); % VoIP errors centered
xlabel('Number of VoIP Flows');
ylabel('Average Packet Delay (ms)');
title('Average Packet Delay for Data and VoIP');
legend('Data', 'VoIP');
hold off;

% Plot Packet Loss for Data and VoIP with error bars
figure;
hold on; grid on;
b = bar(nVoIPs, [mean_PLdata; mean_PLVoIP]', 'grouped');  % Plot bar chart
% Get x-coordinates of each bar for accurate error bar placement
xData = b(1).XEndPoints;  % Data bar positions
errorbar(xData, mean_PLdata, ci_PLdata, 'k.', 'linestyle', 'none'); % Data errors centered
xData = b(2).XEndPoints;  % VoIP bar positions
errorbar(xData, mean_PLVoIP, ci_PLVoIP, 'k.', 'linestyle', 'none'); % VoIP errors centered
xlabel('Number of VoIP Flows');
ylabel('Packet Loss (%)');
title('Packet Loss for Data and VoIP');
legend('Data', 'VoIP');
hold off;
