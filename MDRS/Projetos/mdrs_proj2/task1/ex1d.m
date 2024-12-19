%% 1.d.

clear
clc

fprintf('------------------------------ Task 1.d.------------------------------\n');

% carregar os dados
load('InputDataProject2.mat');

% parâmetros
nNodes = size(Nodes, 1);
nFlows = size(T, 1);
nLinks = size(Links, 1);

v = 2 * 10^5;
D = L / v;

% inicializar variáveis para os melhores resultados
bestAnycastNodes = [];
minWorstRoundTripDelay = inf;
bestDelays = [];
bestTaux = [];
bestSP = [];

% testar todas as combinações possíveis de dois nós
for i = 1:nNodes
    for j = i+1:nNodes
        anycastNodes = [i j];
        
        % inicializar variáveis
        Taux = zeros(nFlows, 4);
        delays = zeros(nFlows, 1);
        sP = cell(nFlows, 1);
        
        % calcular os caminhos mais curtos e os atrasos de ida e volta
        for n = 1:nFlows
            if T(n, 1) == 1 || T(n, 1) == 2
                [shortestPath, totalCost] = kShortestPath(D, T(n, 2), T(n, 3), 1);
                sP{n} = shortestPath;
                delays(n) = totalCost;
                Taux(n, :) = T(n, 2:5);
            elseif T(n, 1) == 3
                if ismember(T(n, 2), anycastNodes)
                    sP{n} = {T(n, 2)};
                    nSP{n} = 1;
                    Taux(n, :) = T(n, 2:5);
                    Taux(n, 3) = T(n, 2);
                else
                    cost = inf;
                    Taux(n, :) = T(n, 2:5);
                    for k = anycastNodes
                        [shortestPath, totalCost] = kShortestPath(D, T(n, 2), k, 1);
                        if totalCost < cost
                            sP{n} = shortestPath;
                            nSP{n} = 1;
                            cost = totalCost;
                            delays(n) = totalCost;
                            Taux(n, 3) = k;
                        end
                    end
                end
            end
        end
        
        % calcular os atrasos de ida e volta
        maxDelayAnycast = max(delays(find(T(:, 1) == 3))) * 2 * 1000;
        
        % atualizar os melhores resultados se o atraso de ida e volta for menor
        if maxDelayAnycast < minWorstRoundTripDelay
            minWorstRoundTripDelay = maxDelayAnycast;
            bestAnycastNodes = anycastNodes;
            bestDelays = delays;
            bestTaux = Taux;
            bestSP = sP;
        end
    end
end

% calcular as cargas dos links
Loads = calculateLinkLoads(nNodes, Links, bestTaux, bestSP, ones(nFlows, 1));

% encontrar a pior carga de link
worstLinkLoad = max(max(Loads(:, 3:4)));

% Calcular os atrasos de ida e volta
maxDelayUnicast1 = max(bestDelays(find(T(:, 1) == 1))) * 2 * 1000;
avgDelayUnicast1 = mean(bestDelays(find(T(:, 1) == 1))) * 2 * 1000;
maxDelayUnicast2 = max(bestDelays(find(T(:, 1) == 2))) * 2 * 1000;
avgDelayUnicast2 = mean(bestDelays(find(T(:, 1) == 2))) * 2 * 1000;
maxDelayAnycast = max(bestDelays(find(T(:, 1) == 3))) * 2 * 1000;
avgDelayAnycast = mean(bestDelays(find(T(:, 1) == 3))) * 2 * 1000;

% mostrar os resultados
fprintf('Best anycast nodes = %d  %d\n', bestAnycastNodes(1), bestAnycastNodes(2));
fprintf('Worst link load = %.2f Gbps\n', worstLinkLoad);
fprintf('Worst round-trip delay (unicast service 1) = %.2f ms\n', maxDelayUnicast1);
fprintf('Average round-trip delay (unicast service 1) = %.2f ms\n', avgDelayUnicast1);
fprintf('Worst round-trip delay (unicast service 2) = %.2f ms\n', maxDelayUnicast2);
fprintf('Average round-trip delay (unicast service 2) = %.2f ms\n', avgDelayUnicast2);
fprintf('Worst round-trip delay (anycast service) = %.2f ms\n', maxDelayAnycast);
fprintf('Average round-trip delay (anycast service) = %.2f ms\n', avgDelayAnycast);
