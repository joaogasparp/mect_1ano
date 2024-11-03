%% Exercise 1.c.

% Parameters
ber = [10^-6, 10^-4];  % Bit error rates for experiments 1.a and 1.b
pkt_sizes = [64, 110, 1518];  % Specific packet sizes
pkt_probs = [0.19, 0.23, 0.17];  % Probabilities of specific packet sizes
other_prob = (1 - sum(pkt_probs)) / ((109 - 65 + 1) + (1517 - 111 + 1));  % Probability of other sizes

% Initialize vector to store theoretical packet loss
ploss = zeros(1, length(ber));

% Calculate theoretical packet loss for each BER
for i = 1:length(ber)
    for size = 64:1518
        if size == 64
            ploss(i) = ploss(i) + (1 - (1 - ber(i))^(size * 8)) * 0.19;
        elseif size == 110
            ploss(i) = ploss(i) + (1 - (1 - ber(i))^(size * 8)) * 0.23;
        elseif size == 1518
            ploss(i) = ploss(i) + (1 - (1 - ber(i))^(size * 8)) * 0.17;
        else
            ploss(i) = ploss(i) + (1 - (1 - ber(i))^(size * 8)) * other_prob;
        end
    end
end

% Normalize theoretical packet loss
ploss = (ploss ./ (0.19 + 0.23 + 0.17 + ((109 - 65 + 1) + (1517 - 111 + 1)) * other_prob)) * 100;

% Compare results
fprintf('Theoretical packet loss for b = 10^-6: %.4f%%\n', ploss(1));
fprintf('Theoretical packet loss for b = 10^-4: %.4f%%\n', ploss(2));