%% Exercise 2.b.

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
PLdata = zeros(N, length(n_values));
PLVoIP = zeros(N, length(n_values));

% Run the simulation for each value of n
for i = 1:length(n_values)
    n = n_values(i);
    for j = 1:N
        [PLdata(j, i), PLVoIP(j, i), ~, ~, ~, ~, ~] = Sim3A(lambda, C, f, P, n, b);
    end
end

% Calculate mean and confidence intervals
mean_PLdata = mean(PLdata);
mean_PLVoIP = mean(PLVoIP);

ci_PLdata = norminv(1-alfa/2) * sqrt(var(PLdata) / N);
ci_PLVoIP = norminv(1-alfa/2) * sqrt(var(PLVoIP) / N);

% Plotting the results for packet loss of data packets
figure;
bar(n_values, mean_PLdata);
hold on;
errorbar(n_values, mean_PLdata, ci_PLdata, '.');
title('Average Packet Loss (Data)');
xlabel('Number of VoIP Flows');
ylabel('Packet Loss (%)');
grid on;
hold off;

% Plotting the results for packet loss of VoIP packets
figure;
bar(n_values, mean_PLVoIP);
hold on;
errorbar(n_values, mean_PLVoIP, ci_PLVoIP, '.');
title('Average Packet Loss (VoIP)');
xlabel('Number of VoIP Flows');
ylabel('Packet Loss (%)');
grid on;
hold off;