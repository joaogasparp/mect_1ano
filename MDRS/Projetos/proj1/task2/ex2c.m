%% Exercise 2.c.

% Parameters
lambda = 1500;                  % packet rate (packets/sec)
C = 10;                         % link bandwidth (Mbps)
f = 1000000;                    % queue size (Bytes)
b = 10^-5;                      % bit error rate
n_values = [10, 20, 30, 40];    % number of VoIP flows
N = 20;                         % number of runs
P = 100000;                     % number of packets (stopping criterion)
alfa = 0.1;                     % 90% confidence interval

% Variables to store the simulation results
APDdata = zeros(N, length(n_values));
APDVoIP = zeros(N, length(n_values));

% Run the simulation for each value of n
for i = 1:length(n_values)
    n = n_values(i);
    for j = 1:N
        [~, ~, APDdata(j, i), APDVoIP(j, i), ~, ~, ~] = Sim3A(lambda, C, f, P, n, b);
    end
end

% Calculate mean and confidence intervals
mean_APDdata = mean(APDdata);
mean_APDVoIP = mean(APDVoIP);

ci_APDdata = norminv(1-alfa/2) * sqrt(var(APDdata) / N);
ci_APDVoIP = norminv(1-alfa/2) * sqrt(var(APDVoIP) / N);

% Plotting the results for average packet delay of data packets
figure;
bar(n_values, mean_APDdata);
hold on;
errorbar(n_values, mean_APDdata, ci_APDdata, '.');
title('Average Packet Delay (Data)');
xlabel('Number of VoIP Flows');
ylabel('Average Delay (ms)');
grid on;
hold off;

% Plotting the results for average packet delay of VoIP packets
figure;
bar(n_values, mean_APDVoIP);
hold on;
errorbar(n_values, mean_APDVoIP, ci_APDVoIP, '.');
title('Average Packet Delay (VoIP)');
xlabel('Number of VoIP Flows');
ylabel('Average Delay (ms)');
grid on;
hold off;