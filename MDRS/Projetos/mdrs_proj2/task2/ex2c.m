%% 2.c.

clear

fprintf('------------------------------ Task 2.c.------------------------------\n');

% carregar os dados
load('InputDataProject2.mat');

% parâmetros
nNodes = size(Nodes, 1);
nFlows = size(T, 1);
nLinks = size(Links, 1);
k = 6;

v = 2 * 10^5;
D = L / v;

anycastNodes = [4 12];

% inicializar variáveis para os atrasos e caminhos
Taux = zeros(nFlows, 4);
delays = zeros(nFlows, 1);
sP = cell(nFlows, 1);
nSP = zeros(nFlows, 1);

% calcular os caminhos mais curtos e os atrasos de ida e volta
for n = 1:nFlows
    if T(n, 1) == 1 || T(n, 1) == 2
        [shortestPaths, totalCosts] = kShortestPath(D, T(n, 2), T(n, 3), k);
        sP{n} = shortestPaths;
        nSP(n) = length(shortestPaths);
        delays(n) = totalCosts(1);
        Taux(n, :) = T(n, 2:5);
    elseif T(n, 1) == 3
        if ismember(T(n, 2), anycastNodes)
            sP{n} = {T(n, 2)};
            nSP(n) = 1;
            Taux(n, :) = T(n, 2:5);
            Taux(n, 3) = T(n, 2);
        else
            cost = inf;
            Taux(n, :) = T(n, 2:5);
            for i = anycastNodes
                [shortestPaths, totalCosts] = kShortestPath(D, T(n, 2), i, k);
                if totalCosts(1) < cost
                    sP{n} = shortestPaths;
                    nSP(n) = length(shortestPaths);
                    cost = totalCosts(1);
                    delays(n) = totalCosts(1);
                    Taux(n, 3) = i;
                end
            end
        end
    end
end

unicastFlows1 = find(T(:, 1) == 1);
unicastFlows2 = find(T(:, 1) == 2);
anycastFlows = find(T(:, 1) == 3);

maxDelayUnicast1 = max(delays(unicastFlows1)) * 2 * 1000;
avgDelayUnicast1 = mean(delays(unicastFlows1)) * 2 * 1000;

maxDelayUnicast2 = max(delays(unicastFlows2)) * 2 * 1000;
avgDelayUnicast2 = mean(delays(unicastFlows2)) * 2 * 1000;

maxDelayAnycast = max(delays(anycastFlows)) * 2 * 1000;
avgDelayAnycast = mean(delays(anycastFlows)) * 2 * 1000;

% parâmetros do algoritmo
maxTime = 30;
bestLoad = inf;
bestSol = [];
totalCycles = 0;
bestCycle = 0;
startTime = tic;

% "correr" o algoritmo Multi Start Hill Climbing
while toc(startTime) < maxTime
    % solução inicial
    [sol, load] = greedyRandomizedStrategy(nNodes, Links, Taux, sP, nSP);
    
    % melhorar a solução inicial
    [sol, load] = HillClimbingStrategy(nNodes, Links, Taux, sP, nSP, sol, load);
    
    totalCycles = totalCycles + 1;
    
    % verificar se a solução atual é a melhor encontrada
    if load < bestLoad
        bestLoad = load;
        bestSol = sol;
        bestTime = toc(startTime);
        bestCycle = totalCycles;
    end
end

% mostrar os resultados
fprintf('Multi Start Hill Climbing algorithm with initial Greedy Randomized (Anycast nodes: %d %d)\n', anycastNodes(1), anycastNodes(2));
fprintf('Worst link load of the network: %.2f\n', bestLoad);
fprintf('Total number of cycles run: %d\n', totalCycles);
fprintf('Running time at which the best solution was obtained: %.2f seconds\n', bestTime);
fprintf('Number of cycles at which the best solution was obtained: %d\n', bestCycle);
