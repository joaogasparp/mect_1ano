%% Exercise 2.e.

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
TT = zeros(N, length(n_values));

% Run the simulation for each value of n
for i = 1:length(n_values)
    n = n_values(i);
    for j = 1:N
        [~, ~, ~, ~, ~, ~, TT(j, i)] = Sim3A(lambda, C, f, P, n, b);
    end
end

% Calculate mean and confidence intervals
mean_TT = mean(TT);
ci_TT = norminv(1-alfa/2) * sqrt(var(TT) / N);

% Plotting the results for total throughput
figure;
bar(n_values, mean_TT);
hold on;
errorbar(n_values, mean_TT, ci_TT, '.');
title('Total Throughput');
xlabel('Number of VoIP Flows');
ylabel('Throughput (Mbps)');
grid on;
hold off;