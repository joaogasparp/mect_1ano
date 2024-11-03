%% Exercise 2.f.

% Parameters
C = 10;                % Link bandwidth in Mbps
lambda = 1500;         % packets per second for data

% Calculate B_avg
prob_64 = 0.19;
prob_110 = 0.23;
prob_1518 = 0.17;
mean_aux2 = mean([65:109 111:1517]);
B_avg = prob_64 * 64 + prob_110 * 110 + prob_1518 * 1518 + (1 - prob_64 - prob_110 - prob_1518) * mean_aux2;

n_values = [10, 20, 30, 40]; % Number of VoIP flows

% Calculate theoretical throughput for each n
for i = 1:length(n_values)
    n = n_values(i);
    TT_theoretical = min(C, lambda * (B_avg * 8) / 1e6);
    fprintf('Theoretical throughput for %d VoIP flows is: %.2f Mbps\n', n, TT_theoretical);
end
