%% Exercise 2.d.

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
MPDdata = zeros(N, length(n_values));
MPDVoIP = zeros(N, length(n_values));

% Run the simulation for each value of n
for i = 1:length(n_values)
    n = n_values(i);
    for j = 1:N
        [~, ~, ~, ~, MPDdata(j, i), MPDVoIP(j, i), ~] = Sim3A(lambda, C, f, P, n, b);
    end
end

% Calculate mean and confidence intervals
mean_MPDdata = mean(MPDdata);
mean_MPDVoIP = mean(MPDVoIP);

ci_MPDdata = norminv(1-alfa/2) * sqrt(var(MPDdata) / N);
ci_MPDVoIP = norminv(1-alfa/2) * sqrt(var(MPDVoIP) / N);

% Plotting the results for maximum packet delay of data packets
figure;
bar(n_values, mean_MPDdata);
hold on;
errorbar(n_values, mean_MPDdata, ci_MPDdata, '.');
title('Maximum Packet Delay (Data)');
xlabel('Number of VoIP Flows');
ylabel('Maximum Delay (ms)');
grid on;
hold off;

% Plotting the results for maximum packet delay of VoIP packets
figure;
bar(n_values, mean_MPDVoIP);
hold on;
errorbar(n_values, mean_MPDVoIP, ci_MPDVoIP, '.');
title('Maximum Packet Delay (VoIP)');
xlabel('Number of VoIP Flows');
ylabel('Maximum Delay (ms)');
grid on;
hold off;