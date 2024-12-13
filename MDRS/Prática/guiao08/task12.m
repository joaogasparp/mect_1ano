%% 12.a.

clear
clc

load("InputData3.mat");

nNodes= size(Nodes,1);
nFlows= size(T,1);
nLinks= size(Links,1);

v = 2*10^5;
D = L/v;
anycastNodes= [3 5];

Taux = zeros(nFlows,4);
delays = zeros(nFlows,1);

for n=1:nFlows
    if T(n,1) == 1
        [shortestPath, totalCost] = kShortestPath(D, T(n,2), T(n,3), 1);
        sP{n} = shortestPath;
        nSP{n} = length(shortestPath);
        delays(n) = totalCost;
        Taux(n,:) = T(n,2:5);

    elseif T(n,1) == 2
        if ismember(T(n,2),anycastNodes)
            sP{n} = {T(n,2)};
            nSP{n} = 1;
            Taux(n,:) = T(n,2:5);
            Taux(n,2) = T(n,2);
        else
            cost = inf;
            Taux(n,:) = T(n,2:5);
            for i = anycastNodes
                [shortestPath, totalCost] = kShortestPath(D, T(n,2), i, 1);
                if totalCost < cost
                    sP{n} = shortestPath;
                    nSP{n} = 1;
                    cost = totalCost;
                    
                    delays(n)=totalCost;
                    Taux(n,2) = i;
                end
            end
        end
    end
end

maxDelayUnicast = max(delays(1:6))*2*1000;
avgDelayUnicast = mean(delays(1:6))*2*1000;

maxDelayAnycast = max(delays(7:17))*2*1000;
avgDelayAnycast = mean(delays(7:17))*2*1000;

fprintf('Anycast nodes = %d  %d\n', anycastNodes(1), anycastNodes(2));
fprintf('Worst round-trip delay (unicast service) = %.2f ms\n', maxDelayUnicast);
fprintf('Average round-trip delay (unicast service) = %.2f ms\n', avgDelayUnicast);
fprintf('Worst round-trip delay (anycast service) = %.2f ms\n', maxDelayAnycast);
fprintf('Average round-trip delay (anycast service) = %.2f ms\n', avgDelayAnycast);

%% 12.b.

clear
clc

% Carregar os dados de entrada
load('InputData3.mat');

% Definir os parâmetros
nNodes = size(Nodes, 1);
nFlows = size(T, 1);
nLinks = size(Links, 1);

v = 2 * 10^5; % velocidade da luz em km/s
D = L / v; % matriz de atrasos de propagação

% Definir os nós anycast
anycastNodes = [3 5];

% Inicializar variáveis
Taux = zeros(nFlows, 4);
delays = zeros(nFlows, 1);

% Calcular as rotas mais curtas e os atrasos
for n = 1:nFlows
    if T(n, 1) == 1
        [shortestPath, totalCost] = kShortestPath(D, T(n, 2), T(n, 3), 1);
        sP{n} = shortestPath;
        nSP{n} = length(shortestPath);
        delays(n) = totalCost;
        Taux(n, :) = T(n, 2:5);
    elseif T(n, 1) == 2
        if ismember(T(n, 2), anycastNodes)
            sP{n} = {T(n, 2)};
            nSP{n} = 1;
            Taux(n, :) = T(n, 2:5);
            Taux(n, 2) = T(n, 2);
        else
            cost = inf;
            Taux(n, :) = T(n, 2:5);
            for i = anycastNodes
                [shortestPath, totalCost] = kShortestPath(D, T(n, 2), i, 1);
                if totalCost < cost
                    sP{n} = shortestPath;
                    nSP{n} = 1;
                    cost = totalCost;
                    delays(n) = totalCost;
                    Taux(n, 2) = i;
                end
            end
        end
    end
end

% Calcular as cargas dos links
Loads = calculateLinkLoads(nNodes, Links, Taux, sP, ones(nFlows, 1));

% Encontrar a pior carga de link
worstLinkLoad = max(max(Loads(:, 3:4)));

% Exibir os resultados
fprintf('Anycast nodes = %d  %d\n', anycastNodes(1), anycastNodes(2));
fprintf('Worst link load = %.2f Gbps\n', worstLinkLoad);
for i = 1:nLinks
    fprintf('{%d-%d}: %.2f %.2f\n', Loads(i, 1), Loads(i, 2), Loads(i, 3), Loads(i, 4));
end

%% 12.c.

clear
clc

% Carregar os dados de entrada
load('InputData3.mat');

% Definir os parâmetros
nNodes = size(Nodes, 1);
nFlows = size(T, 1);
nLinks = size(Links, 1);

v = 2 * 10^5; % velocidade da luz em km/s
D = L / v; % matriz de atrasos de propagação

% Inicializar variáveis para armazenar os melhores resultados
bestAnycastNodes = [];
minWorstLinkLoad = inf;
bestDelays = [];
bestTaux = [];
bestSP = [];

% Testar todas as combinações possíveis de dois nós
for i = 1:nNodes
    for j = i+1:nNodes
        anycastNodes = [i j];
        
        % Inicializar variáveis
        Taux = zeros(nFlows, 4);
        delays = zeros(nFlows, 1);
        sP = cell(nFlows, 1);
        
        % Calcular as rotas mais curtas e os atrasos
        for n = 1:nFlows
            if T(n, 1) == 1
                [shortestPath, totalCost] = kShortestPath(D, T(n, 2), T(n, 3), 1);
                sP{n} = shortestPath;
                nSP{n} = length(shortestPath);
                delays(n) = totalCost;
                Taux(n, :) = T(n, 2:5);
            elseif T(n, 1) == 2
                if ismember(T(n, 2), anycastNodes)
                    sP{n} = {T(n, 2)};
                    nSP{n} = 1;
                    Taux(n, :) = T(n, 2:5);
                    Taux(n, 2) = T(n, 2);
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
                            Taux(n, 2) = k;
                        end
                    end
                end
            end
        end
        
        % Calcular as cargas dos links
        Loads = calculateLinkLoads(nNodes, Links, Taux, sP, ones(nFlows, 1));
        
        % Encontrar a pior carga de link
        worstLinkLoad = max(max(Loads(:, 3:4)));
        
        % Atualizar os melhores resultados se a carga de link for menor
        if worstLinkLoad < minWorstLinkLoad
            minWorstLinkLoad = worstLinkLoad;
            bestAnycastNodes = anycastNodes;
            bestDelays = delays;
            bestTaux = Taux;
            bestSP = sP;
        end
    end
end

% Calcular os atrasos de ida e volta
maxDelayUnicast = max(bestDelays(1:6)) * 2 * 1000;
avgDelayUnicast = mean(bestDelays(1:6)) * 2 * 1000;
maxDelayAnycast = max(bestDelays(7:17)) * 2 * 1000;
avgDelayAnycast = mean(bestDelays(7:17)) * 2 * 1000;

% Exibir os resultados
fprintf('Best anycast nodes = %d  %d\n', bestAnycastNodes(1), bestAnycastNodes(2));
fprintf('Worst link load = %.2f Gbps\n', minWorstLinkLoad);
fprintf('Worst round-trip delay (unicast service) = %.2f ms\n', maxDelayUnicast);
fprintf('Average round-trip delay (unicast service) = %.2f ms\n', avgDelayUnicast);
fprintf('Worst round-trip delay (anycast service) = %.2f ms\n', maxDelayAnycast);
fprintf('Average round-trip delay (anycast service) = %.2f ms\n', avgDelayAnycast);

%% 12.d.

clear
clc

% Carregar os dados de entrada
load('InputData3.mat');

% Definir os parâmetros
nNodes = size(Nodes, 1);
nFlows = size(T, 1);
nLinks = size(Links, 1);

v = 2 * 10^5; % velocidade da luz em km/s
D = L / v; % matriz de atrasos de propagação

% Inicializar variáveis para armazenar os melhores resultados
bestAnycastNodes = [];
minWorstRoundTripDelay = inf;
bestDelays = [];
bestTaux = [];
bestSP = [];

% Testar todas as combinações possíveis de dois nós
for i = 1:nNodes
    for j = i+1:nNodes
        anycastNodes = [i j];
        
        % Inicializar variáveis
        Taux = zeros(nFlows, 4);
        delays = zeros(nFlows, 1);
        sP = cell(nFlows, 1);
        
        % Calcular as rotas mais curtas e os atrasos
        for n = 1:nFlows
            if T(n, 1) == 1
                [shortestPath, totalCost] = kShortestPath(D, T(n, 2), T(n, 3), 1);
                sP{n} = shortestPath;
                nSP{n} = length(shortestPath);
                delays(n) = totalCost;
                Taux(n, :) = T(n, 2:5);
            elseif T(n, 1) == 2
                if ismember(T(n, 2), anycastNodes)
                    sP{n} = {T(n, 2)};
                    nSP{n} = 1;
                    Taux(n, :) = T(n, 2:5);
                    Taux(n, 2) = T(n, 2);
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
                            Taux(n, 2) = k;
                        end
                    end
                end
            end
        end
        
        % Calcular os atrasos de ida e volta
        maxDelayUnicast = max(delays(1:6)) * 2 * 1000;
        avgDelayUnicast = mean(delays(1:6)) * 2 * 1000;
        maxDelayAnycast = max(delays(7:17)) * 2 * 1000;
        avgDelayAnycast = mean(delays(7:17)) * 2 * 1000;
        
        % Atualizar os melhores resultados se o atraso de ida e volta for menor
        if maxDelayAnycast < minWorstRoundTripDelay
            minWorstRoundTripDelay = maxDelayAnycast;
            bestAnycastNodes = anycastNodes;
            bestDelays = delays;
            bestTaux = Taux;
            bestSP = sP;
        end
    end
end

% Calcular as cargas dos links para a melhor combinação
Loads = calculateLinkLoads(nNodes, Links, bestTaux, bestSP, ones(nFlows, 1));

% Encontrar a pior carga de link
worstLinkLoad = max(max(Loads(:, 3:4)));

% Calcular os atrasos de ida e volta
maxDelayUnicast = max(bestDelays(1:6)) * 2 * 1000;
avgDelayUnicast = mean(bestDelays(1:6)) * 2 * 1000;
maxDelayAnycast = max(bestDelays(7:17)) * 2 * 1000;
avgDelayAnycast = mean(bestDelays(7:17)) * 2 * 1000;

% Exibir os resultados
fprintf('Best anycast nodes = %d  %d\n', bestAnycastNodes(1), bestAnycastNodes(2));
fprintf('Worst link load = %.2f Gbps\n', worstLinkLoad);
fprintf('Worst round-trip delay (unicast service) = %.2f ms\n', maxDelayUnicast);
fprintf('Average round-trip delay (unicast service) = %.2f ms\n', avgDelayUnicast);
fprintf('Worst round-trip delay (anycast service) = %.2f ms\n', maxDelayAnycast);
fprintf('Average round-trip delay (anycast service) = %.2f ms\n', avgDelayAnycast);
