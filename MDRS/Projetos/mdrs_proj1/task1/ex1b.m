%% Exercise 1.b.

C = 10;     % Capacity of the link (Mbps)
f = 10000;  % Size of the queue (Bytes)
N = 20;     % Times to run the simulation
P = 100000; % Stopping criteria
alfa = 0.1; % 90% confidence interval

b = 10^-4;                                  % Bit error rate
arrival_rate = [1500 1600 1700 1800 1900];  % Arrival rate values (pps)

% Variables to store the simulation results
PL = zeros(1, N);
APD = zeros(1, N);
MPD = zeros(1, N);
TT = zeros(1, N);

% Variables to store bar graph data
APD_values = zeros(1, length(arrival_rate));
APD_terms = zeros(1, length(arrival_rate));
PL_values = zeros(1, length(arrival_rate));
PL_terms = zeros(1, length(arrival_rate));

for i = 1:length(arrival_rate)
    lambda = arrival_rate(i);
    for j = 1:N
        [PL(j), APD(j), MPD(j), TT(j)] = Sim2(lambda, C, f, P, b);
    end

    media_PL = mean(PL);
    term_PL = norminv(1-alfa/2)*sqrt(var(PL)/N);
    PL_values(i) = media_PL;
    PL_terms(i) = term_PL;

    media_APD = mean(APD);
    term_APD = norminv(1-alfa/2)*sqrt(var(APD)/N);
    APD_values(i) = media_APD;
    APD_terms(i) = term_APD;
end

% Plotting the results in separate bar charts with error bars
figure;
bar(arrival_rate, PL_values);
hold on;
grid on;
errorbar(arrival_rate, PL_values, PL_terms, '.');
title('Average Packet Loss');
xlabel('Arrival Rate (pps)');
ylabel('Packet Loss (%)');
hold off;

figure;
bar(arrival_rate, APD_values);
hold on;
grid on;
errorbar(arrival_rate, APD_values, APD_terms, '.');
title('Average Packet Delay');
xlabel('Arrival Rate (pps)');
ylabel('Average Delay (ms)');
hold off;
