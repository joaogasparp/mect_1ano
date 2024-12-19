%% Exercise 2.a.

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
APDdata = zeros(N, length(n_values));
APDVoIP = zeros(N, length(n_values));
MPDdata = zeros(N, length(n_values));
MPDVoIP = zeros(N, length(n_values));
TT = zeros(N, length(n_values));

% Run the simulation for each value of n
for i = 1:length(n_values)
    n = n_values(i);
    for j = 1:N
        [PLdata(j, i), PLVoIP(j, i), APDdata(j, i), APDVoIP(j, i), MPDdata(j, i), MPDVoIP(j, i), TT(j, i)] = Sim3A(lambda, C, f, P, n, b);
    end
end

% Calculate mean and confidence intervals
mean_PLdata = mean(PLdata);
mean_PLVoIP = mean(PLVoIP);
mean_APDdata = mean(APDdata);
mean_APDVoIP = mean(APDVoIP);
mean_MPDdata = mean(MPDdata);
mean_MPDVoIP = mean(MPDVoIP);
mean_TT = mean(TT);

ci_PLdata = norminv(1-alfa/2) * sqrt(var(PLdata) / N);
ci_PLVoIP = norminv(1-alfa/2) * sqrt(var(PLVoIP) / N);
ci_APDdata = norminv(1-alfa/2) * sqrt(var(APDdata) / N);
ci_APDVoIP = norminv(1-alfa/2) * sqrt(var(APDVoIP) / N);
ci_MPDdata = norminv(1-alfa/2) * sqrt(var(MPDdata) / N);
ci_MPDVoIP = norminv(1-alfa/2) * sqrt(var(MPDVoIP) / N);
ci_TT = norminv(1-alfa/2) * sqrt(var(TT) / N);

% Print the results
for i = 1:length(n_values)
    fprintf('Number of VoIP Flows: %d\n', n_values(i));
    fprintf('PLdata: %.4f%% ± %.4f%%\n', mean_PLdata(i), ci_PLdata(i));
    fprintf('PLVoIP: %.4f%% ± %.4f%%\n', mean_PLVoIP(i), ci_PLVoIP(i));
    fprintf('APDdata: %.4f ms ± %.4f ms\n', mean_APDdata(i), ci_APDdata(i));
    fprintf('APDVoIP: %.4f ms ± %.4f ms\n', mean_APDVoIP(i), ci_APDVoIP(i));
    fprintf('MPDdata: %.4f ms ± %.4f ms\n', mean_MPDdata(i), ci_MPDdata(i));
    fprintf('MPDVoIP: %.4f ms ± %.4f ms\n', mean_MPDVoIP(i), ci_MPDVoIP(i));
    fprintf('TT: %.4f Mbps ± %.4f Mbps\n', mean_TT(i), ci_TT(i));
    fprintf('\n');
end
