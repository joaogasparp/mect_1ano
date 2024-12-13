%% ex11.a.

clear
clc

load('InputData2.mat')
nNodes = size(Nodes, 1);
nLinks = size(Links, 1);
nFlows = size(T, 1);

% Calcular os caminhos mais curtos para todos os fluxos
sP = cell(1, nFlows);
nSP = zeros(1, nFlows);
for f = 1:nFlows
    k = inf;
    [shortestPath, totalCost] = kShortestPath(L, T(f, 1), T(f, 2), k);
    sP{f} = shortestPath;
    nSP(f) = length(totalCost);
end

% Calcular as cargas dos links
sol = ones(1, nFlows);
Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);

% Calcular o consumo de energia total (E)
E = calculateEnergy(Links, Loads, L);

% Calcular a pior carga de link (W)
W = max(max(Loads(:, 3:4)));

% Exibir os resultados
fprintf('E = %.2f W = %.2f Gbps\n', E, W);

%% ex11.c.

clear all
close all
clc

load('InputData2.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

% Computing up to k=inf shortest paths for all flows from 1 to nFlows:
k= inf;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end
% sP{f}{i} is the i-th path of flow f
% nSP(f) is the number of paths of flow f

t = tic;
timeLimit = 30;
bestEnergy = inf;
contador = 0;
alfa = 1;
while toc(t) < timeLimit
    % greedy randomzied start
    % first greedy randomized solution
    [sol, load, Loads, energy] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP, L, alfa);

    while energy == inf
        [sol, load, Loads, energy] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP, L, alfa);
    end

    [sol, load, Loads, energy] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load, Loads, energy, L, alfa);

    if energy < bestEnergy
        bestSol= sol;
        bestLoad= load;
        bestLoads = Loads;
        bestEnergy = energy;
        bestLoadTime = toc(t);
    end
    contador = contador + 1;
end

sleepingNodes = '';
for i = 1 : length(bestLoads)
    if max(bestLoads(i, 3:4)) == 0
        sleepingNodes = append(sleepingNodes, ' {', num2str(bestLoads(i,1)), ',', num2str(bestLoads(i,2)), '}');
    end
end

fprintf('E = %.2f\tW = %.2f Gbps\tNo. sols = %d\ttime = %.2f\n', bestEnergy, bestLoad, contador, bestLoadTime);
fprintf('List of links in sleeping mode:%s\n', sleepingNodes);

%% ex11.d.

clear
clc

load('InputData2.mat')
nNodes = size(Nodes, 1);
nLinks = size(Links, 1);
nFlows = size(T, 1);

% Calcular os 6 caminhos mais curtos para todos os fluxos
sP = cell(1, nFlows);
nSP = zeros(1, nFlows);
for f = 1:nFlows
    k = 6;
    [shortestPath, totalCost] = kShortestPath(L, T(f, 1), T(f, 2), k);
    sP{f} = shortestPath;
    nSP(f) = length(totalCost);
end

t = tic;
timeLimit = 30;
bestEnergy = inf;
contador = 0;
alfa = 1;
while toc(t) < timeLimit
    % greedy randomzied start
    % first greedy randomized solution
    [sol, load, Loads, energy] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP, L, alfa);

    while energy == inf
        [sol, load, Loads, energy] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP, L, alfa);
    end

    [sol, load, Loads, energy] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load, Loads, energy, L, alfa);

    if energy < bestEnergy
        bestSol= sol;
        bestLoad= load;
        bestLoads = Loads;
        bestEnergy = energy;
        bestLoadTime = toc(t);
    end
    contador = contador + 1;
end

sleepingNodes = '';
for i = 1 : length(bestLoads)
    if max(bestLoads(i, 3:4)) == 0
        sleepingNodes = append(sleepingNodes, ' {', num2str(bestLoads(i,1)), ',', num2str(bestLoads(i,2)), '}');
    end
end

fprintf('E = %.2f\tW = %.2f Gbps\tNo. sols = %d\ttime = %.2f\n', bestEnergy, bestLoad, contador, bestLoadTime);
fprintf('List of links in sleeping mode:%s\n', sleepingNodes);

%% ex11.e.

clear
clc

load('InputData2.mat')
nNodes = size(Nodes, 1);
nLinks = size(Links, 1);
nFlows = size(T, 1);

% Calcular todos os caminhos mais curtos para todos os fluxos
sP = cell(1, nFlows);
nSP = zeros(1, nFlows);
for f = 1:nFlows
    k = inf;
    [shortestPath, totalCost] = kShortestPath(L, T(f, 1), T(f, 2), k);
    sP{f} = shortestPath;
    nSP(f) = length(totalCost);
end

t = tic;
timeLimit = 30;
bestEnergy = inf;
contador = 0;
alfa = 0.7;
while toc(t) < timeLimit
    % greedy randomzied start
    % first greedy randomized solution
    [sol, load, Loads, energy] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP, L, alfa);

    while energy == inf
        [sol, load, Loads, energy] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP, L, alfa);
    end

    [sol, load, Loads, energy] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load, Loads, energy, L, alfa);

    if energy < bestEnergy
        bestSol= sol;
        bestLoad= load;
        bestLoads = Loads;
        bestEnergy = energy;
        bestLoadTime = toc(t);
    end
    contador = contador + 1;
end

sleepingNodes = '';
for i = 1 : length(bestLoads)
    if max(bestLoads(i, 3:4)) == 0
        sleepingNodes = append(sleepingNodes, ' {', num2str(bestLoads(i,1)), ',', num2str(bestLoads(i,2)), '}');
    end
end

fprintf('E = %.2f\tW = %.2f Gbps\tNo. sols = %d\ttime = %.2f\n', bestEnergy, bestLoad, contador, bestLoadTime);
fprintf('List of links in sleeping mode:%s\n', sleepingNodes);

%% ex11.f.

clear
clc

load('InputData2.mat')
nNodes = size(Nodes, 1);
nLinks = size(Links, 1);
nFlows = size(T, 1);

% Calcular os 6 caminhos mais curtos para todos os fluxos
sP = cell(1, nFlows);
nSP = zeros(1, nFlows);
for f = 1:nFlows
    k = 6;
    [shortestPath, totalCost] = kShortestPath(L, T(f, 1), T(f, 2), k);
    sP{f} = shortestPath;
    nSP(f) = length(totalCost);
end

t = tic;
timeLimit = 30;
bestEnergy = inf;
contador = 0;
alfa = 0.7;
while toc(t) < timeLimit
    % greedy randomzied start
    % first greedy randomized solution
    [sol, load, Loads, energy] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP, L, alfa);

    while energy == inf
        [sol, load, Loads, energy] = greedyRandomizedStrategy(nNodes, Links, T, sP, nSP, L, alfa);
    end

    [sol, load, Loads, energy] = HillClimbingStrategy(nNodes, Links, T, sP, nSP, sol, load, Loads, energy, L, alfa);

    if energy < bestEnergy
        bestSol= sol;
        bestLoad= load;
        bestLoads = Loads;
        bestEnergy = energy;
        bestLoadTime = toc(t);
    end
    contador = contador + 1;
end

sleepingNodes = '';
for i = 1 : length(bestLoads)
    if max(bestLoads(i, 3:4)) == 0
        sleepingNodes = append(sleepingNodes, ' {', num2str(bestLoads(i,1)), ',', num2str(bestLoads(i,2)), '}');
    end
end

fprintf('E = %.2f\tW = %.2f Gbps\tNo. sols = %d\ttime = %.2f\n', bestEnergy, bestLoad, contador, bestLoadTime);
fprintf('List of links in sleeping mode:%s\n', sleepingNodes);
