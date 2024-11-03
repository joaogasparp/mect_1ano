%% 8.a.

clear all
close all
clc

load('InputData.mat')
nNodes = size(Nodes, 1);
nLinks = size(Links, 1);
nFlows= size(T,1);

k = 1;
sP = cell(1,nFlows);
nSP = zeros(1,nFlows);
for f = 1 : nFlows
    [shortestPath, totalCost] = kShortestPath(L, T(f, 1), T(f, 2), k);
    sP{f} = shortestPath;
    nSP(f) = totalCost;
    fprintf('Flow %d (%d -> %d): length = %d, Path = %s\n', f, T(f, 1), T(f, 2), totalCost, num2str(shortestPath{1}));
end

%% 8.b.

sol = ones(1, nFlows);
Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);

maxLoad = max(max(Loads(:,3:4)));
fprintf('Worst link load = %.2f\n', maxLoad);

for i = 1 : length(Loads)
    fprintf('{%d-%d}:\t%.2f\t%.2f\n', Loads(i, 1), Loads(i, 2), Loads(i, 3), Loads(i, 4));
end

%% 8.c.

k = 4;
f = 1;
[shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);

for i = 1 : length(totalCost)
    fprintf('Path %d = %s (length= %d)\n\n',i,num2str(shortestPath{i}),totalCost(i));
end

%% 8.d.

clear all
close all
clc

load('InputData.mat')
nNodes= size(Nodes,1);
nLinks= size(Links,1);
nFlows= size(T,1);

k= inf;
sP= cell(1,nFlows);
nSP= zeros(1,nFlows);
for f=1:nFlows
    [shortestPath, totalCost] = kShortestPath(L,T(f,1),T(f,2),k);
    sP{f}= shortestPath;
    nSP(f)= length(totalCost);
end

sol = ones(1, nFlows);
Loads = calculateLinkLoads(nNodes, Links, T, sP, sol);

maxLoad = max(max(Loads(:,3:4)));

